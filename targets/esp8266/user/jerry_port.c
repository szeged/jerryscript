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


#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <espressif/esp_common.h>
#include "jerryscript-port.h"
#ifdef JERRY_DEBUGGER
#include "jerryscript-core.h"
#include "jerryscript-debugger.h"
#endif /* JERRY_DEBUGGER */
#include "FreeRTOS.h"
#include "task.h"
#include "esp_sntp.h"

/**
 * Actual log level
 */
static jerry_log_level_t jerry_port_default_log_level = JERRY_LOG_LEVEL_DEBUG;

#define JERRY_PORT_DEFAULT_LOG_LEVEL jerry_port_default_log_level

/**
 * Provide log message implementation for the engine.
 */
void
jerry_port_log (jerry_log_level_t level, /**< log level */
                const char *format, /**< format string */
                ...)  /**< parameters */
{
  if (level <= JERRY_PORT_DEFAULT_LOG_LEVEL)
  {
    va_list args;
    va_start (args, format);
#ifdef JERRY_DEBUGGER
    char buffer[256];
    int length = 0;
    length = vsnprintf (buffer, 255, format, args);
    buffer[length] = '\0';
    printf ("%*s", length, buffer);
    jerry_char_t *jbuffer = (jerry_char_t *) buffer;
    jerry_debugger_send_output (jbuffer, (jerry_size_t) length, (uint8_t) (level + 2));
#else
    vprintf (format, args);
#endif /* JERRY_DEBUGGER */
    va_end (args);
  }
} /* jerry_port_log */

/**
 * Provide fatal message implementation for the engine.
 */
void
jerry_port_fatal (jerry_fatal_code_t code)
{
  jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Jerry fatal error, error code: %d!\n", code);
  while (true);
} /* jerry_port_fatal */

/**
 * Implementation of jerry_port_get_current_time.
 *
 * @return current timer's counter value in milliseconds
 */
double
jerry_port_get_current_time (void)
{
  int32_t microseconds;
  time_t seconds = sntp_get_rtc_time (&microseconds);
  if (seconds > 15e8)
  {
    return ((double) (seconds * 1000 + microseconds / 1000));
  }

  /** If system_get_rtc_time returns `x` (it means `x` RTC cycles), and
   *  system_rtc_clock_cali_proc returns `y` (it means `y` microseconds per RTC clock cycle),
   *  then the actual time is `x` * `y` / 1000 milliseconds.
   */
  uint32_t rtc_time = sdk_system_get_time () / 1000;
  return (double) rtc_time;
} /* jerry_port_get_current_time */

/**
 * Dummy function to get the time zone adjustment.
 *
 * @return 0
 */
double
jerry_port_get_local_time_zone_adjustment (double unix_ms, bool is_utc)
{
  return 0;
} /* jerry_port_get_time_zone */

#ifdef JERRY_DEBUGGER
void jerry_port_sleep (uint32_t sleep_time)
{
  vTaskDelay (sleep_time / portTICK_PERIOD_MS);
}
#endif

/**
 * Opens file with the given path and reads its source.
 * @return the source of the file
 */
uint8_t *
jerry_port_read_source (const char *file_name_p, /**< file name */
                        size_t *out_size_p) /**< [out] read bytes */
{
  return NULL;
} /* jerry_port_read_source */

/**
 * Release the previously opened file's content.
 */
void
jerry_port_release_source (uint8_t *buffer_p) /**< buffer to free */
{
} /* jerry_port_release_source */
