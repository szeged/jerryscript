#ifndef ESP_I2C_JS_H
#define ESP_I2C_JS_H

#include "jerry_extapi.h"
#include <i2c/i2c.h>

#define I2C_OBJECT_NAME "I2C"

void register_i2c_object (jerry_value_t global_object);

#endif /* ESP_I2C_JS_H */
