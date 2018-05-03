#include "esp_uart_js.h"

DELCARE_HANDLER (uart_init)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (UART_OBJECT_NAME, UART_INIT, "1");
  }

  if (!jerry_value_is_number (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_NUMBER);
  }

  uint32_t baud = jerry_get_number_value (args_p[0]);

  uart_set_baud(UART_NUM, baud);
  return jerry_create_boolean (true);
}

DELCARE_HANDLER (uart_available)
{
  if (args_cnt != 0 && args_cnt != 1)
  {
    return raise_argument_count_error (UART_OBJECT_NAME, UART_AVAILABLE, "0 or 1");
  }

  if (args_cnt == 1 && !jerry_value_is_number (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_NUMBER);
  }

  if (args_cnt == 1)
  {
    int32_t bytes = jerry_get_number_value (args_p[0]);
    return jerry_create_number (uart_rxfifo_wait (UART_NUM, bytes));
  }

  return jerry_create_number (uart_rxfifo_wait (UART_NUM, 0));
}

DELCARE_HANDLER (uart_read)
{
  if (args_cnt != 0)
  {
    return raise_argument_count_error (UART_OBJECT_NAME, UART_READ, "0");
  }

  return jerry_create_number (uart_getc (UART_NUM));
}

DELCARE_HANDLER (uart_flush)
{
  if (args_cnt != 0)
  {
    return raise_argument_count_error (UART_OBJECT_NAME, UART_FLUSH, "0");
  }

  while(uart_rxfifo_wait (UART_NUM, 0))
  {
    uart_getc (UART_NUM);
  }

  return jerry_create_boolean (true);
}

DELCARE_HANDLER (uart_write){
  if (args_cnt != 1)
  {
    return raise_argument_count_error (UART_OBJECT_NAME, UART_WRITE, "0");
  }

  if (jerry_value_is_number (args_p[0]))
  {
    int value = (int) jerry_get_number_value (args_p[0]);
    uart_putc (UART_NUM, value);
  }
  else if (jerry_value_is_array(args_p[0]))
  {
    uint32_t len = jerry_get_array_length (args_p[0]);
    uint8_t element = 0;

    for (int i = 0; i < len; i++)
    {
      jerry_value_t idx = jerry_get_property_by_index (args_p[0], i);
      element = (int) jerry_get_number_value (idx);
      uart_putc (UART_NUM, element);
      jerry_release_value (idx);
    }
  }
  else
  {
    return raise_argument_type_error ("1", TYPE_NUMBER "or" TYPE_ARRAY);
  }

  return jerry_create_boolean (true);
}

void
register_uart_object (jerry_value_t global_object)
{
  jerry_value_t uart_object = jerry_create_object ();
  register_js_value_to_object (UART_OBJECT_NAME, uart_object, global_object);

  register_native_function (UART_INIT, uart_init_handler, uart_object);
  register_native_function (UART_READ, uart_read_handler, uart_object);
  register_native_function (UART_WRITE, uart_write_handler, uart_object);
  register_native_function (UART_AVAILABLE, uart_available_handler, uart_object);
  register_native_function (UART_FLUSH, uart_flush_handler, uart_object);

  jerry_release_value (uart_object);
}
