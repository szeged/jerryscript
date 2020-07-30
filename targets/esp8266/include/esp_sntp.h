#ifndef ESP_SNTP_H
#define ESP_SNTP_H

#include "FreeRTOS.h"
#include "task.h"
#include <sntp.h>
#include <stdlib.h>
#include <stdio.h>
#include <espressif/esp_common.h>

static const struct timezone tz =
{
  1 * 60, 1
};

void init_esp_sntp (void);
bool sntp_been_init(void);

#endif
