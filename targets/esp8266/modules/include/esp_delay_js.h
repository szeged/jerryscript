#ifndef ESP_DELAY_JS_H
#define ESP_DELAY_JS_H

#include "jerry_extapi.h"
#include "fcntl.h"
#include "unistd.h"
#include "spiffs.h"
#include "esp_spiffs.h"
#include "esp_sntp.h"

#define DELAY_OBJECT_NAME "DELAY"
#define DELAY_MS "millis"
#define DELAY_US "micros"
#define DELAY_SYSTEM_TIME "systemTime"
#define DELAY_DEEP_SLEEP "deepSleep"

void register_delay_object (jerry_value_t global_object);

#endif
