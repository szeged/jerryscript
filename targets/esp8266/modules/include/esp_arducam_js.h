#ifndef ESP_ARDUCAM_JS_H
#define ESP_ARDUCAM_JS_H

#include "jerry_extapi.h"
#include <esp/spi.h>
#include <esp/gpio.h>
#include <i2c/i2c.h>
#include <espressif/esp8266/gpio_register.h>
#include <espressif/esp8266/pin_mux_register.h>

#include "ov5642_regs.h"

// Property names of the ArduCAM js object
#define ARDUCAM_OBJECT_NAME     "ArduCAM"
#define ARDUCAM_INIT            "init"
#define ARDUCAM_CAPTURE         "capture"
#define ARDUCAM_STORE           "store"

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
#define CS_PIN                         15

#define spi_cs_low()                   do { gpio_write (CS_PIN, false); } while (0)
#define spi_cs_high()                  do { gpio_write (CS_PIN, true); } while (0)
#define spi_read_byte()                (spi_transfer_8 (SPI_BUS, 0x00))

// I2C control
#define I2C_BUS                        0
#define I2C_SDA_PIN                    5
#define I2C_SCL_PIN                    4
#define I2C_SLAVE_ADDR_WRITE           0x78
#define I2C_SLAVE_ADDR_READ            0x79

// Misc
#define MS 1000
#define timeout_expired(start, len) ((uint32_t)(sdk_system_get_time() - (start)) >= (len))

enum image_size {
  OV5642_320x240,
  OV5642_640x480,
  OV5642_1024x768,
  OV5642_1280x960,
  OV5642_1600x1200,
  OV5642_2048x1536,
  OV5642_2592x1944,
  OV5642_1920x1080,
};

bool wait (uint32_t timeout);
uint8_t read_reg (uint8_t address);
void write_reg (uint8_t address, uint8_t value);
uint8_t get_bit (uint8_t address, uint8_t bit);
void set_bit (uint8_t address, uint8_t bit);
void clear_bit (uint8_t address, uint8_t bit);
void wr_sensor_reg_16_8 (uint16_t regID, uint8_t regDat);
void wr_sensor_regs_16_8 (const struct sensor_reg reglist[]);
uint8_t rd_sensor_reg_16_8 (uint16_t regID);
void init_cam ();
void set_image_size (enum image_size size);
void set_raw_size (enum image_size size);

void register_arducam_object (jerry_value_t global_object);

#endif
