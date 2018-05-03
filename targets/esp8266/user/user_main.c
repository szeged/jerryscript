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
#include "jerry_run.h"

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

  check_deep_sleep_status();

#ifdef JERRY_DEBUGGER
  struct sdk_station_config config =
  {
    .ssid = "ESP8266",
    .password = "Barackospite"
  };

  sdk_wifi_set_opmode (STATION_MODE);
  sdk_wifi_station_set_config (&config);
  //sdk_wifi_station_set_auto_connect (0);
  sdk_wifi_station_connect ();
#endif

  BaseType_t xReturned;
  TaskHandle_t xHandle = NULL;
  xReturned = xTaskCreate (jerry_task, "jerry", JERRY_STACK_SIZE, NULL, 2, &xHandle);
  if (xReturned != pdPASS)
  {
    printf("Cannot allocate enough memory to task.\n");
    while (true) {};
  }
} /* user_init */
