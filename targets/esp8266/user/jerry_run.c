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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include <espressif/esp_common.h>

#ifdef JERRY_DEBUGGER
#include "esp_sntp.h"
#endif

#include "jerry_extapi.h"
#include "jerry_run.h"
#include "jerry-targetjs.h"

#include "jerryscript.h"
#include "jerryscript-port.h"

static const char* fn_sys_loop_name = "sysloop";

/**
 * Js2C creates e.g. `main` from `main.js` so we convert it back
 */
static inline void revert_resource_name (const char* name, jerry_char_t* resource_name, jerry_size_t resource_name_length)
{
  strncpy ((char*) resource_name, name, resource_name_length);
  resource_name[resource_name_length + 2] = 's';
  resource_name[resource_name_length + 1] = 'j';
  resource_name[resource_name_length] = '.';
} /* revert_resource_name */

/**
 * Print the failing script name
 */
static void jerry_parse_failed (const char* name)
{
  jerry_size_t resource_name_length = strlen (name);
  jerry_char_t resource_name[resource_name_length + 3];

  revert_resource_name (name, resource_name, resource_name_length);

  jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Script parsing failed in %s\n", resource_name);
} /* jerry_parse_failed */

/**
 * Initialize the engine and parse the given sources
 * Return true - if all script parsing was succesful
 *        fasle - otherwise
 */
bool jerry_task_init (void)
{
  srand ((unsigned) jerry_port_get_current_time ());
  DECLARE_JS_CODES;
  jerry_init (JERRY_INIT_EMPTY);

  register_js_entries ();

#ifdef JERRY_DEBUGGER
  while (sdk_wifi_station_get_connect_status () != STATION_GOT_IP)
  {
    vTaskDelay (1000 / portTICK_PERIOD_MS);
  }

  if (!sntp_been_init())
  {
    init_esp_sntp ();
  }

  jerry_debugger_init (5001);
#endif

  /* run rest of the js files first */
  for (int idx = 1; js_codes[idx].source; idx++)
  {
    if (!parse_resource (js_codes[idx].name, js_codes[idx].source, js_codes[idx].length))
    {
      jerry_parse_failed (js_codes[idx].name);
      return false;
    }
  }

  /* run main.js */
  if (!parse_resource (js_codes[0].name, js_codes[0].source, js_codes[0].length))
  {
    jerry_parse_failed (js_codes[0].name);
    return false;
  }

  return true;
} /* jerry_task_init */

/**
 * Parse and run the given sources
 * Return true - if parsing and run was succesful
 *        fasle - otherwise
 */
bool parse_resource (const char *name, const char *source, const int length)
{
  jerry_size_t resource_name_length = strlen (name);
  jerry_char_t resource_name[resource_name_length + 3];

  revert_resource_name (name, resource_name, resource_name_length);

  jerry_value_t res = jerry_parse (resource_name,
                                  resource_name_length + 3,
                                  (jerry_char_t *) source,
                                  length,
                                  false);

  if (!jerry_value_is_error (res))
  {
    jerry_value_t func_val = res;
    res = jerry_run (func_val);
    jerry_release_value (func_val);
  }

  jerry_release_value (res);

  return true;
} /* parse_resource */

/**
 * Call sysloop function
 * Return true - if the function call was succesfull
 *        fasle - otherwise
 */
bool js_loop (uint32_t ticknow)
{
  jerry_value_t global_obj_val = jerry_get_global_object ();
  jerry_value_t prop_name_val = jerry_create_string ((const jerry_char_t *) fn_sys_loop_name);
  jerry_value_t sysloop_func = jerry_get_property (global_obj_val, prop_name_val);
  jerry_release_value (prop_name_val);

  if (jerry_value_is_error (sysloop_func)) {
    printf ("Error: '%s' not defined!!!\r\n", fn_sys_loop_name);
    jerry_release_value (sysloop_func);
    jerry_release_value (global_obj_val);
    return false;
  }

  if (!jerry_value_is_function (sysloop_func))
  {
    printf ("Error: '%s' is not a function!!!\r\n", fn_sys_loop_name);
    jerry_release_value (sysloop_func);
    jerry_release_value (global_obj_val);
    return false;
  }

  jerry_value_t val_args[] =
  {
    jerry_create_number (ticknow)
  };

  uint16_t val_argv = sizeof (val_args) / sizeof (jerry_value_t);

  jerry_value_t res = jerry_call_function (sysloop_func,
                                           global_obj_val,
                                           val_args,
                                           val_argv);

  for (uint16_t i = 0; i < val_argv; i++)
  {
    jerry_release_value (val_args[i]);
  }

  jerry_release_value (sysloop_func);
  jerry_release_value (global_obj_val);

  if (jerry_value_is_error (res)) {
    jerry_release_value (res);
    return false;
  }

  jerry_release_value (res);

  return true;
} /* js_loop */

/**
 * Terminate the engine
 */
void jerry_task_exit (void)
{
  jerry_cleanup ();
} /* jerry_task_exit */
