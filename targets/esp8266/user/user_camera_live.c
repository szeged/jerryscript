#include "jerry_extapi.h"
#include "user_camera_live.h"

void delay_millies (int ms)
{
  vTaskDelay (ms / portTICK_PERIOD_MS);
}

static bool wifi_connect (char* ssid, char *pwd)
{
  struct sdk_station_config config;
  strcpy((char *) config.ssid, ssid);
  strcpy((char *) config.password, pwd);

  sdk_wifi_set_opmode (STATION_MODE);
  sdk_wifi_station_set_config (&config);
  sdk_wifi_station_connect ();
  sdk_wifi_station_set_auto_connect (0);

  uint8_t attempts = 0;
  while (sdk_wifi_station_get_connect_status () != STATION_GOT_IP
         && attempts < MAX_CONNECT_ATTEMPTS)
  {
    vTaskDelay (100 / portTICK_PERIOD_MS);
    taskYIELD();
    attempts++;
  }

  return attempts != MAX_CONNECT_ATTEMPTS;
}

bool initialize_connection (netconn_t *conn_p)
{
  if (!wifi_connect (LIVE_WIFI_SSID, LIVE_WIFI_PWD))
  {
    return false;
  }

  printf("WIFI connected\n");

  netconn_t conn = netconn_new (NETCONN_TCP);

  if (conn == NULL)
  {
    printf ("netconn_new failed\n");
    return false;
  }

  ip_addr_t addr;
  err_t err = netconn_gethostbyname (LIVE_SERVER_ADDR, &addr);

  if (err != ERR_OK)
  {
    printf("Could not find host %s\n", LIVE_SERVER_ADDR);
    netconn_delete (conn);
    return false;
  }

  err = netconn_connect (conn, &addr, LIVE_SERVER_PORT);

  if (err != ERR_OK)
  {
    printf("Could not connect to host %s on port %d\n", LIVE_SERVER_ADDR, LIVE_SERVER_PORT);
    netconn_delete (conn);
    return false;
  }

  *conn_p = conn;
  printf("Socket created\n");
  return true;
}

void close_connection (netconn_t conn)
{
  netconn_close (conn);
  netconn_delete (conn);
  printf("Socket closed\n");
}

#define SOCKET_SEND_ATTEMPT_MAX 5

bool send_image_size (uint32_t size, netconn_t conn)
{
  char buff[8] = {0};

  *buff = MSG_TYPE_IMAGE_SIZE;
  itoa ((int) size, buff + 1, 16);
  uint8_t num_of_digits = strlen (buff + 1);

  err_t result = -1;
  int attempts = 0;
  do {
    result = netconn_write (conn, buff, num_of_digits + 1, NETCONN_COPY);
    if (result == ERR_OK) {
      break;
    }
    delay_millies(25);
  } while (attempts++ < SOCKET_SEND_ATTEMPT_MAX);

  return result == ERR_OK;
}

bool send_image(uint8_t *data_p, uint32_t size, netconn_t conn)
{
  *data_p = MSG_TYPE_IMAGE_DATA;
  err_t result = -1;
  int attempts = 0;
  do {
    result = netconn_write (conn, data_p, size + 1, NETCONN_COPY);
    if (result == ERR_OK) {
      break;
    }
    delay_millies(25);
  } while (attempts++ < SOCKET_SEND_ATTEMPT_MAX);
  return result == ERR_OK;
}

bool send_close_connection(netconn_t conn, message_type_t reason)
{
  char buff[] = {reason, 0, 0, 0};
  err_t result = -1;
  int attempts = 0;
  do {
    result = netconn_write (conn, buff, sizeof(buff), NETCONN_COPY);
    if (result == ERR_OK) {
      break;
    }
    delay_millies(25);
  } while (attempts++ < SOCKET_SEND_ATTEMPT_MAX);
  return result == ERR_OK;
}

bool send_image_fragment_flag (netconn_t conn)
{
  uint8_t buff[1] = { MSG_TYPE_IMAGE_FRAG };
  err_t result = -1;
  int attempts = 0;
  do
  {
    result = netconn_write (conn, buff, 1, NETCONN_COPY);
    if (result == ERR_OK)
    {
      break;
    }
    // delay_millies(25);
    taskYIELD ();
  } while (attempts++ < SOCKET_SEND_ATTEMPT_MAX);
  return result == ERR_OK;
}

bool send_image_fragment (uint8_t *data_p, uint32_t size, netconn_t conn)
{
  err_t result = -1;
  int attempts = 0;
  do {
    result = netconn_write (conn, data_p, size, NETCONN_COPY);
    if (result == ERR_OK) {
      break;
    }
    // delay_millies(25);
    taskYIELD ();
  } while (attempts++ < SOCKET_SEND_ATTEMPT_MAX);
  return result == ERR_OK;
}

bool send_image_buffer(uint8_t *data_p, uint32_t size, netconn_t conn)
{
  *data_p = MSG_TYPE_IMAGE_DATA;
  err_t result = -1;
  int attempts = 0;
  do {
    result = netconn_write (conn, data_p, size + 1, NETCONN_COPY);
    if (result == ERR_OK) {
      break;
    }
    delay_millies(25);
  } while (attempts++ < SOCKET_SEND_ATTEMPT_MAX);
  return result == ERR_OK;
}
