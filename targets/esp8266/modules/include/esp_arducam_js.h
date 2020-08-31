#ifndef ESP_ARDUCAM_JS_H
#define ESP_ARDUCAM_JS_H

#include "jerry_extapi.h"
#include <esp/spi.h>
#include <esp/gpio.h>
#include <fatfs/ff.h>
#include <i2c/i2c.h>
#include <espressif/esp8266/gpio_register.h>
#include <espressif/esp8266/pin_mux_register.h>

#include "ov5642_regs.h"

// Property names of the ArduCAM js object
#define ARDUCAM_OBJECT_NAME     "ArduCAM"
#define ARDUCAM_INIT            "init"
#define ARDUCAM_CAPTURE         "capture"
#define ARDUCAM_STORE           "store"
#define ARDUCAM_PRINT           "_print"
#define ARDUCAM_SD_CS           "SD_CS"
#define ARDUCAM_SET_IMAGE_SIZE  "setImageSize"

#define ARDUCAM_EMPTY_SIZE_PROP_NAME "IMG_SIZE_EMPTY"
#define ARDUCAM_320x240_PROP_NAME "IMG_SIZE_320x240"
#define ARDUCAM_640x480_PROP_NAME "IMG_SIZE_640x480"
#define ARDUCAM_1024x768_PROP_NAME "IMG_SIZE_1024x768"
#define ARDUCAM_1280x960_PROP_NAME "IMG_SIZE_1280x960"
#define ARDUCAM_1600x1200_PROP_NAME "IMG_SIZE_1600x1200"
#define ARDUCAM_1920x1080_PROP_NAME "IMG_SIZE_1920x1080"
#define ARDUCAM_2048x1536_PROP_NAME "IMG_SIZE_2048x1536"
#define ARDUCAM_2592x1944_PROP_NAME "IMG_SIZE_2592x1944"

// Type definitions and helpers
typedef enum arducam_image_size {
  EMPTY = 0,
  OV5642_320x240,
  OV5642_640x480,
  OV5642_1024x768,
  OV5642_1280x960,
  OV5642_1600x1200,
  OV5642_2048x1536,
  OV5642_2592x1944,
} arducam_image_size;

typedef enum arducam_init_error_t {
  ARDUCAM_INIT_OK,
  ARDUCAM_INIT_SPI_INIT_FAILED,
  ARDUCAM_INIT_I2C_INIT_FAILED,
  ARDUCAM_INIT_SPI_INTERFACE_ERROR,
  ARDUCAM_INIT_I2C_INTERFACE_ERROR,
} arducam_init_error_t;


typedef enum arducam_capture_error_t {
  ARDUCAM_CAPTURE_OK,
  ARDUCAM_CAPTURE_OVER_SIZE,
  ARDUCAM_CAPTURE_SIZE_ZERO
} arducam_capture_error_t;

arducam_init_error_t _arducam_init();
arducam_capture_error_t _arducam_capture();

#define ARDUCAM_BUFF_SIZE              128

// Registers and masks
#define REG_TEST                       0x00

#define REG_CAPTURE_CONTROL            0x01
#define MASK_PIC_COUNT_1               0x01

#define REG_MODE                       0x02
#define MASK_MCU_MODE                  0x00

#define REG_TIMING_CONTROL             0x03
#define MASK_HSYNC_LEVEL               0x01  // 0 = active high, 1 = active low
#define MASK_VSYNC_LEVEL               0x02  // 0 = active high, 1 = active low
#define MASK_SENSOR_DATA_DELAY         0x04  // 0 = no delay, 1 = delay 1 PCLK
#define MASK_FIFO_MODE                 0x08  // 0 = disable, 1 = enable
#define MASK_LOW_POWER_MODE            0x10  // 0 = normal mode, 1 = low power mode

#define REG_FIFO_CONTROL               0x04
#define MASK_CLEAR_CAPTURE_DONE_FLAG   0x01
#define MASK_START_CAPTURE             0x02

#define REG_GPIO_WRITE                 0x06
#define MASK_GPIO_RESET                0x01
#define MASK_GPIO_PWDN                 0x02

#define REG_FIFO_BURST_READ            0x3c

#define REG_FIFO_SINGLE_READ           0x3d

#define REG_TRIG                       0x41
#define MASK_CAMERA_WRITE_FIFO_DONE    0x08
#define MASK_CAPTURE_DONE              0x08

#define REG_FIFO_SIZE1                 0x42
#define REG_FIFO_SIZE2                 0x43
#define REG_FIFO_SIZE3                 0x44

// SPI control
#define SPI_BUS                         1
#define CAMERA_CS                       2
#define SD_CS                           0

#define spi_cs_low(PIN)                   do { gpio_write (PIN, false); } while (0)
#define spi_cs_high(PIN)                  do { gpio_write (PIN, true); } while (0)
#define spi_read_byte()                (spi_transfer_8 (SPI_BUS, 0x00))

// I2C control
#define I2C_BUS                        0
#define I2C_SDA_PIN                    5
#define I2C_SCL_PIN                    4
#define I2C_SLAVE_ADDR_WRITE           0x78
#define I2C_SLAVE_ADDR_READ            0x79

// I2C and SPI low level functions
void wait (uint32_t timeout);
uint8_t read_reg (uint8_t pin, uint8_t address);
void write_reg (uint8_t pin, uint8_t address, uint8_t value);
uint8_t get_bit (uint8_t pin, uint8_t address, uint8_t bit);
void set_bit (uint8_t pin, uint8_t address, uint8_t bit);
void clear_bit (uint8_t pin, uint8_t address, uint8_t bit);
uint32_t read_fifo_length ();
void wr_sensor_reg_16_8 (uint16_t regID, uint8_t regDat);
void wr_sensor_regs_16_8 (const struct sensor_reg reglist[]);
uint8_t rd_sensor_reg_16_8 (uint16_t regID);
void init_cam ();
void set_image_size (enum arducam_image_size size);

void register_arducam_object (jerry_value_t global_object);

#endif
