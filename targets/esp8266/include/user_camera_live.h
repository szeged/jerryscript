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

typedef enum {
  MSG_TYPE_IMAGE_SIZE = 0,
  MSG_TYPE_IMAGE_DATA = 1,
  MSG_TYPE_ABORT_CONN = 2,
  MSG_TYPE_CLOSE_CONN = 3,
  MSG_TYPE_IMAGE_FRAG = 4
} message_type_t;

#define MAX_CONNECT_ATTEMPTS 100

bool user_ucam_live (uint32_t timeout);
bool user_arducam_live (uint32_t timeout);

void delay_millies (int ms);
bool initialize_connection (netconn_t *conn_p);
void close_connection (netconn_t conn);

bool send_image_size (uint32_t size, netconn_t conn);
bool send_image (uint8_t *data_p, uint32_t size, netconn_t conn);
bool send_close_connection(netconn_t conn, message_type_t reason);
bool send_image_fragment_flag (netconn_t conn);
bool send_image_fragment (uint8_t *data_p, uint32_t size, netconn_t conn);
bool send_image_buffer(uint8_t *data_p, uint32_t size, netconn_t conn);

#define LIVE_WIFI_SSID "ESP8266"
#define LIVE_WIFI_PWD "Barackospite"
#define LIVE_SERVER_ADDR "10.109.169.1"
#define LIVE_SERVER_PORT 5011

#endif
