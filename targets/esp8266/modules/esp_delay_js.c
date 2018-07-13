#include "esp_delay_js.h"
#include "math.h"

DELCARE_HANDLER (delay_ms)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (DELAY_OBJECT_NAME, DELAY_MS, "1");
  }

  if (!jerry_value_is_number (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_NUMBER);
  }

  int value = (int) jerry_get_number_value (args_p[0]);
  if (value > 0)
  {
    vTaskDelay (value / portTICK_PERIOD_MS);
  }

  return jerry_create_boolean (true);
} /* delay_ms */

DELCARE_HANDLER (delay_us)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (DELAY_OBJECT_NAME, DELAY_US, "1");
  }

  if (!jerry_value_is_number (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_NUMBER);
  }

  int value = (int) jerry_get_number_value (args_p[0]);

  if (value > 0)
  {
    sdk_os_delay_us (value);
  }

  return jerry_create_boolean (true);
} /* delay_us */

DELCARE_HANDLER (delay_system_time)
{
  if (args_cnt != 0)
  {
    return raise_argument_count_error (DELAY_OBJECT_NAME, DELAY_US, "0");
  }

  return jerry_create_number (sdk_system_get_time() / 1000);
} /* delay_system_time */

DELCARE_HANDLER (delay_deep_sleep)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (DELAY_OBJECT_NAME, DELAY_US, "1");
  }

  if (!jerry_value_is_number (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_NUMBER);
  }

  double value = jerry_get_number_value (args_p[0]) * 1000 ;

  if ((uint32_t) value <= 0)
  {
    return jerry_create_boolean (true);
  }

  if (value < (double) UINT32_MAX)
  {
    sdk_system_deep_sleep ((uint32_t) value, 0);
  }
  else
  {
    esp_spiffs_init();

    if (esp_spiffs_mount () != SPIFFS_OK)
    {
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Cannot mount SPIFFS!");
    }

    int fd = open ("deep_sleep.txt", O_WRONLY | O_CREAT);
    if (fd < 0)
    {
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Cannot open file!");
    }
    double integer;

    modf(value, &integer);

    char *buffer;
    buffer = (char *) malloc (sizeof (char) * 256);
    snprintf (buffer, sizeof (char) * 256, "%.0lf", integer - UINT32_MAX);
    write(fd, buffer, strlen (buffer));
    free (buffer);

    SPIFFS_unmount(&fs);
    esp_spiffs_deinit();

    sdk_system_deep_sleep (UINT32_MAX, 0);
  }

  return jerry_create_boolean (true);
} /* delay_deep_sleep */

void register_delay_object (jerry_value_t global_object)
{
  jerry_value_t delay_object = jerry_create_object ();
  register_js_value_to_object (DELAY_OBJECT_NAME, delay_object, global_object);

  register_native_function (DELAY_MS, delay_ms_handler, delay_object);
  register_native_function (DELAY_US, delay_us_handler, delay_object);
  register_native_function (DELAY_DEEP_SLEEP, delay_deep_sleep_handler, delay_object);
  register_native_function (DELAY_SYSTEM_TIME, delay_system_time_handler, delay_object);
  jerry_release_value (delay_object);
}
