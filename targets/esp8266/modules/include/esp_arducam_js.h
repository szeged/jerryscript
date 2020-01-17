#ifndef ESP_ARDUCAM_JS_H
#define ESP_ARDUCAM_JS_H

#include "jerry_extapi.h"
#include <esp/spi.h>
#include <esp/gpio.h>

// Property names of the ArduCAM js object
#define ARDUCAM_OBJECT_NAME     "ArduCAM"
#define ARDUCAM_INIT            "init"
#define ARDUCAM_CAPTURE         "capture"

// Registers
#define REG_TEST                0x00
#define REG_CAPTURE_CONTROL     0x01
#define REG_FIFO_CONTROL        0x04

// Masks
#define MASK_CLEAR_FIFO     0x01
#define MASK_START_CAPTURE  0x02

void register_arducam_object (jerry_value_t global_object);

#endif
