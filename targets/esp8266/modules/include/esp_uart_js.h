#ifndef ESP_UART_JS_H
#define ESP_UART_JS_H

#include "jerry_extapi.h"
#include <esp/uart.h>

#define UART_OBJECT_NAME "Serial"
#define UART_INIT "init"
#define UART_READ "read"
#define UART_WRITE "write"
#define UART_FLUSH "flush"
#define UART_AVAILABLE "available"

#define UART_NUM 0

void register_uart_object (jerry_value_t global_object);

#endif
