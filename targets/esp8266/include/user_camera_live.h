#ifndef USER_CAMERA_LIVE_H
#define USER_CAMERA_LIVE_H

#include <esp/uart.h>

#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/netbuf.h"

#ifndef UART_NUM
#define UART_NUM 0
#endif

#define UART_BAUD_RATE 921600
#define CAMERA_RESET_PIN 5

typedef struct netconn* netconn_t;

#define MAX_CONNECT_ATTEMPTS 100

bool user_camera_live (uint32_t timeout);

#define LIVE_WIFI_SSID "ESP8266"
#define LIVE_WIFI_PWD "Barackospite"
#define LIVE_SERVER_ADDR "10.109.183.100"
#define LIVE_SERVER_PORT 5010

#endif
