/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/12/1, v1.0 create this file.
*******************************************************************************/
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>
#if REDIRECT_STDOUT == 1 || REDIRECT_STDOUT == 2
#include <sys/reent.h>
#include <semphr.h>
#include <stdout_redirect.h>
#include <espressif/esp8266/gpio_register.h>
#include <espressif/esp8266/pin_mux_register.h>
#endif
#include <math.h>
#include "fcntl.h"
#include "unistd.h"
#include "spiffs.h"
#include "esp_spiffs.h"

#include "user_config.h"
#include "user_camera_live.h"
#include "jerry_run.h"
#include "jerry_extapi.h"

static uint32_t camera_live_timeout = 0;

/**
 * Start Camera Live in a task
 */
static void camera_task (void *pvParameters)
{
  if (!user_camera_live (camera_live_timeout)) {
    printf("Camera Task failed\n");
  }
  sdk_system_restart ();
} /* camera_task */

/**
 * Start JerryScript in a task
 */
static void jerry_task (void *pvParameters)
{
  if (jerry_task_init ())
  {
    uint32_t ticknow = 0;

    while (true)
    {
      vTaskDelay (100 / portTICK_PERIOD_MS);

      if (!js_loop (ticknow++))
      {
        break;
      }
    }
  }

  jerry_task_exit ();
  printf("Jerry Task failed\n");
  while (true)
  {
  };
} /* jerry_task */

#if REDIRECT_STDOUT == 1 || REDIRECT_STDOUT == 2
ssize_t _write_stdout_r (struct _reent *r, int fd, const void *ptr, size_t len)
{
#if REDIRECT_STDOUT == 1
    for (int i = 0; i < len; i++)
    {
        if (((char *)ptr)[i] == '\r')
        {
          continue;
        }

        if (((char *)ptr)[i] == '\n')
        {
          uart_putc (1, '\r');
        }
        uart_putc (1, ((char *)ptr)[i]);
    }

    return len;
#endif
    return 0;
}
#endif

static bool check_camera_live ()
{
  esp_spiffs_init();
  int err = esp_spiffs_mount ();
  if (err != SPIFFS_OK)
  {
    if (err == SPIFFS_ERR_NOT_A_FS)
    {
      SPIFFS_unmount (&fs);  // FS must be unmounted before formating
      if (SPIFFS_format (&fs) == SPIFFS_OK)
      {
        printf("Format complete\n");
      }
      else
      {
        printf("Format failed\n");
      }
      esp_spiffs_mount();
    }
    else
    {
      printf("Error mount SPIFFS\n");
      while (true) {};
    }
  }

  spiffs_file fd = SPIFFS_open (&fs, "live.txt", SPIFFS_O_RDONLY | SPIFFS_O_WRONLY, 0);

  bool ret_value = false;
  if (!(fd < 0))
  {
    const int buf_size = 0xFF;
    uint8_t buf[buf_size];
    spiffs_file read_bytes = SPIFFS_read (&fs, fd, buf, buf_size);
    printf("Read %d bytes\n", read_bytes);
    if (!(read_bytes < 0))
    {
      buf[read_bytes] = '\0';    // zero terminate string
      camera_live_timeout = atoi ((const char *)buf);
      printf("Data: %s time: %d\n", buf, camera_live_timeout);
      ret_value = true;

      close(fd);
      SPIFFS_remove(&fs, "live.txt");
    }
  }
  else {
    printf("NO file\n");
  }
  SPIFFS_unmount(&fs);
  esp_spiffs_deinit();

  return ret_value;
}

static void check_deep_sleep_status ()
{
  esp_spiffs_init();
  int err = esp_spiffs_mount ();
  if (err != SPIFFS_OK)
  {
    if (err == SPIFFS_ERR_NOT_A_FS)
    {
      SPIFFS_unmount (&fs);  // FS must be unmounted before formating
      if (SPIFFS_format (&fs) == SPIFFS_OK)
      {
        printf("Format complete\n");
      }
      else
      {
        printf("Format failed\n");
      }
      esp_spiffs_mount();
    }
    else
    {
      printf("Error mount SPIFFS\n");
      while (true) {};
    }
  }

  spiffs_file fd = SPIFFS_open (&fs, "deep_sleep.txt", O_RDONLY | O_WRONLY, 0);

  if (!(fd < 0))
  {
    const int buf_size = 0xFF;
    double stored_sleep_time = -1;
    double new_sleep_time = -1;
    uint8_t buf[buf_size];
    spiffs_file read_bytes = SPIFFS_read (&fs, fd, buf, buf_size);
    printf("Read %d bytes\n", read_bytes);
    if (!(read_bytes < 0)){
      buf[read_bytes] = '\0';    // zero terminate string
      printf("Data: %s\n", buf);
      stored_sleep_time = atof ((const char *)buf);
      if (stored_sleep_time < (double) UINT32_MAX)
      {
        modf(stored_sleep_time, &new_sleep_time);

        char *buffer;
        buffer = (char *) malloc (sizeof (char) * 256);
        snprintf (buffer, sizeof (char) * 256, "%.0lf", new_sleep_time - UINT32_MAX);
        int written = write(fd, buffer, strlen (buffer));
        if (written != strlen(buffer))
        {
          printf("fatal error\n");
          while (true){};
        }
        free (buffer);
        close(fd);
      }
      else {
        close(fd);
        SPIFFS_remove(&fs, "deep_sleep.txt");
      }
    }

    SPIFFS_unmount(&fs);
    esp_spiffs_deinit();

    sdk_system_deep_sleep (new_sleep_time == -1 ? stored_sleep_time : new_sleep_time, 0);
  }
  else {
    printf("NO file\n");
  }
  SPIFFS_unmount(&fs);
  esp_spiffs_deinit();
}

/**
 * This is entry point for user code
 */
void user_init (void)
{
#if REDIRECT_STDOUT == 1
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
  uart_set_baud (1, 115200);
#endif

#if REDIRECT_STDOUT == 1 || REDIRECT_STDOUT == 2
  set_write_stdout (_write_stdout_r);
#else
  uart_set_baud (0, 115200);
#endif
  sdk_system_update_cpu_freq (160);
  check_deep_sleep_status();

#ifdef JERRY_DEBUGGER
  struct sdk_station_config config =
  {
    .ssid = "ESP8266",
    .password = "Barackospite"
  };

  sdk_wifi_set_opmode (STATION_MODE);
  sdk_wifi_station_set_config (&config);
  sdk_wifi_station_connect ();
#endif

  BaseType_t xReturned;
  TaskHandle_t xHandle = NULL;

  if (check_camera_live ())
  {
    xReturned = xTaskCreate (camera_task, "camera", 1280, NULL, 2, &xHandle);
  }
  else
  {
    // initialize_conn();
    xReturned = xTaskCreate (jerry_task, "jerry", JERRY_STACK_SIZE, NULL, 2, &xHandle);
  }

  if (xReturned != pdPASS)
  {
    printf("Cannot allocate enough memory to task.\n");
    while (true) {};
  }
  // while (true) {
  //   /* --------------------- */
  //   /* INIT */
  //   wait(5000 * MS);
  //   const spi_settings_t settings = {
  //     .mode = SPI_MODE0,
  //     .freq_divider = SPI_FREQ_DIV_1M,
  //     .msb = true, // ???
  //     .endianness = SPI_LITTLE_ENDIAN, // ???
  //     .minimal_pins = false
  //   };

  //   gpio_enable (CAMERA_CS, GPIO_OUTPUT);
  //   spi_cs_high (CAMERA_CS);
  //   int spi_succ = spi_set_settings (SPI_BUS, &settings);
  //   if (!spi_succ)
  //   {
  //     printf ("SPI init failed, code: %d", spi_succ);
  //     while (true) {};
  //   }
  //   // gpio_enable (SD_CS, GPIO_OUTPUT);
  //   // spi_cs_high (SD_CS);

  //   i2c_set_clock_stretch (I2C_BUS, 10);
  //   int i2c_succ = i2c_init (I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ_80K);
  //   if (i2c_succ)
  //   {
  //     printf ("I2C init failed, code: %d", i2c_succ);
  //     while (true) {};
  //   }

  //   write_reg (CAMERA_CS, 0x07, 0x80);
  //   wait (100 * MS);
  //   write_reg (CAMERA_CS, 0x07, 0x00);
  //   wait (100 * MS);

  //   // Check that SPI and I2C interface works correctly.
  //   write_reg (CAMERA_CS, REG_TEST, 0x55);
  //   uint8_t test = read_reg (CAMERA_CS, REG_TEST);
  //   printf ("%#x\n", test);
  //   if (test != 0x55)
  //   {
  //     printf ("SPI interface error, expected: 0x55, received: %#x", test);
  //     while (true) {};
  //   }

  //   wr_sensor_reg_16_8 (0xff, 0x01);
  //   uint8_t vid = rd_sensor_reg_16_8 (OV5642_CHIPID_HIGH);
  //   uint8_t pid = rd_sensor_reg_16_8 (OV5642_CHIPID_LOW);
  //   printf ("vid: %#x\n", vid);
  //   printf ("pid: %#x\n", pid);
  //   if (!(vid == 0x56 && pid == 0x42))
  //   {
  //     printf ("OV5642 not found\n");
  //     while (true) {};
  //   }

  //   init_cam ();
  //   set_image_size (OV5642_640x480);

  //   /* --------------------- */
  //   /* CAPTURE */
  //   set_bit (CAMERA_CS, REG_TIMING_CONTROL, MASK_VSYNC_LEVEL);
  //   // Clear fifo flag.
  //   write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  //   write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  //   write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  //   write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  //   // Set the picture number to 1.
  //   write_reg (CAMERA_CS, REG_CAPTURE_CONTROL, 1);
  //   // Start capture.
  //   write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_START_CAPTURE);
  //   // Wait until capture is complete.
  //   while (!get_bit (CAMERA_CS, REG_TRIG, MASK_CAPTURE_DONE)){/*wait(1 * MS);*/}

  //   printf ("Capture done!\n");

  //   /* --------------------- */
  //   /* SEND */
  //   uint32_t imageSize = read_fifo_length ();
  //   printf("arducam_print %d\n", imageSize);
  //   uint8_t temp, temp_last;
  //   temp = temp_last = 0;

  //   spi_cs_low (CAMERA_CS);
  //   spi_transfer_8 (SPI_BUS, REG_FIFO_BURST_READ);

  //   while ( imageSize-- )
  //   {
  //       temp_last = temp;
  //       temp = spi_transfer_8 (SPI_BUS, 0x00);

  //       printf("%02x", temp);
  //       taskYIELD();

  //       if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
  //       {
  //           printf ("sajt\n");
  //           break;
  //       }
  //   }

  //   spi_cs_high (CAMERA_CS);
  //   printf ("\n");

  //   // if (!initialize_connection_wrapper (&conn)) {
  //   //   printf("Server connection has failed\n");
  //   // }

  //   // sendPicture (conn);
  // }
} /* user_init */
