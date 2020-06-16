#include "jerry_extapi.h"

static uint8_t *image_buffer_start;
static uint8_t *image_buffer;
static uint32_t image_size = 0;
static uint32_t buf_size = ARDUCAM_BUFF_SIZE;

static void delay_millies (int ms)
{
  vTaskDelay (ms / portTICK_PERIOD_MS);
}

static bool wifi_connect (char* ssid, char *pwd)
{
  struct sdk_station_config config;
  strcpy ((char *) config.ssid, ssid);
  strcpy ((char *) config.password, pwd);

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

// delay_millies(25);
static bool initialize_connection (netconn_t *conn_p)
{
  if (!wifi_connect (ARDUCAM_LIVE_WIFI_SSID, ARDUCAM_LIVE_WIFI_PWD))
  {
    return false;
  }

  printf("WIFI connected\n");

  netconn_t conn = netconn_new (NETCONN_TCP);

  if (conn == NULL)
  {
    printf ("conn == NULL\n");
    return false;
  }

  ip_addr_t addr;
  err_t err = netconn_gethostbyname (ARDUCAM_LIVE_SERVER_ADDR, &addr);

  if (err != ERR_OK)
  {
    printf ("err != ERR_OK\n");
    netconn_delete (conn);
    return false;
  }

  err = netconn_connect (conn, &addr, ARDUCAM_LIVE_SERVER_PORT);

  if (err != ERR_OK)
  {
    printf ("err != ERR_OK 2\n");
    netconn_delete (conn);
    return false;
  }

  *conn_p = conn;
  printf("Socket created\n");
  return true;
}

bool initialize_connection_wrapper (netconn_t *conn_p)
{
  return initialize_connection (conn_p);
}

static void close_connection (netconn_t conn)
{
  netconn_close (conn);
  netconn_delete (conn);
  printf ("Socket closed\n");
}

typedef enum {
  MSG_TYPE_IMAGE_SIZE = 0,
  MSG_TYPE_IMAGE_DATA = 1,
  MSG_TYPE_ABORT_CONN = 2,
  MSG_TYPE_CLOSE_CONN = 3,
  MSG_TYPE_IMAGE_FRAG = 4
} message_type_t;

#define SOCKET_SEND_ATTEMPT_MAX 5

static bool send_image_size (uint32_t size, netconn_t conn)
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

static bool send_image_fragment_flag (netconn_t conn)
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

static bool send_image_fragment (uint8_t *data_p, uint32_t size, netconn_t conn)
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

static bool send_close_connection (netconn_t conn, message_type_t reason)
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

static bool send_image_buffer(uint8_t *data_p, uint32_t size, netconn_t conn)
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

// // Original
// bool send_picture (netconn_t conn)
// {
//   // Get FIFO size and allocate buffer.
//   image_size = read_fifo_length ();
//   send_image_size (image_size, conn);
//   image_buffer_start = (uint8_t*) malloc (buf_size * sizeof (uint8_t)); // Extra byte for package type info
//   if (image_buffer_start == NULL)
//   {
//     printf ("could not allocate image buffer\n");
//     return false;
//   }
//   image_buffer = image_buffer_start + 1;

//   printf ("send_picture: image_size: %d\n", image_size);
//   printf ("send_picture: image_buffer_start: %#x\n", (uint32_t) image_buffer_start);

//   uint32_t image_pos;
//   uint8_t temp, temp_last;
//   image_pos = temp = 0;

//   send_image_fragment_flag (conn); // Sending image data begins.

//   spi_cs_low (CAMERA_CS);
//   // while ((temp != 0xd9) | (temp_last != 0xff))
//   while (image_pos < image_size)
//   {
//     temp_last = temp;
//     temp = read_reg (CAMERA_CS, REG_FIFO_SINGLE_READ);
//     printf("%x", temp);
//     image_buffer[(image_pos++) % buf_size] = temp;

//     if (image_pos % buf_size == 0)
//     {
//       printf ("send_picture: image_pos: %d\n", image_pos);
//       vTaskDelay(100 / portTICK_PERIOD_MS);
//       if (!send_image_fragment (image_buffer_start, buf_size, conn))
//       {
//         printf ("could not send image fragment\n");
//         send_close_connection (conn, MSG_TYPE_CLOSE_CONN);
//         close_connection (conn);
//         return false;
//       }
//     }
//   }
//   spi_cs_high (CAMERA_CS);

//   printf("send_picture: image_pos: %d, buf_size: %d\n", image_pos, buf_size);
//   printf("send_picture: sending %d bytes\n", image_pos % buf_size);
//   if (image_pos % buf_size != 0)
//   {
//     if (!send_image_fragment (image_buffer_start, image_pos % buf_size, conn))
//     {
//       printf ("could not send image fragment\n");
//       send_close_connection (conn, MSG_TYPE_CLOSE_CONN);
//       close_connection (conn);
//       return false;
//     }
//   }

//   free (image_buffer_start);
//   vTaskDelay(1000 / portTICK_PERIOD_MS);
//   if (!send_close_connection (conn, MSG_TYPE_CLOSE_CONN))
//   {
//     printf ("could not send close connection message\n");
//   }
//   close_connection (conn);

//   return true;
// }

// Workaround
bool send_picture (netconn_t conn)
{
  // Get FIFO size and allocate buffer.
  image_size = read_fifo_length ();
  image_buffer_start = (uint8_t*) malloc (buf_size * sizeof (uint8_t)); // Extra byte for package type info
  if (image_buffer_start == NULL)
  {
    printf ("could not allocate image buffer\n");
    return false;
  }
  image_buffer = image_buffer_start + 1;

  printf ("send_picture: image_size: %d\n", image_size);
  printf ("send_picture: image_buffer_start: %#x\n", (uint32_t) image_buffer_start);

  uint32_t image_pos;
  uint8_t temp, temp_last;
  image_pos = temp = 0;

  spi_cs_low (CAMERA_CS);
  while ((temp != 0xd9) | (temp_last != 0xff))
  {
    temp_last = temp;
    temp = read_reg (CAMERA_CS, REG_FIFO_SINGLE_READ);
    printf("%x", temp);
    image_buffer[(image_pos++) % buf_size] = temp;

    if (image_pos % buf_size == 0)
    {
      // printf ("send_picture: image_pos: %d\n", image_pos);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      if (!send_image_buffer (image_buffer_start, buf_size, conn))
      {
        printf ("could not send image fragment\n");
        send_close_connection (conn, MSG_TYPE_CLOSE_CONN);
        close_connection (conn);
        return false;
      }
    }
  }
  spi_cs_high (CAMERA_CS);

  printf("send_picture: image_pos: %d, buf_size: %d\n", image_pos, buf_size);
  printf("send_picture: sending %d bytes\n", image_pos % buf_size);
  if (image_pos % buf_size != 0)
  {
    if (!send_image_buffer (image_buffer_start, image_pos % buf_size, conn))
    {
      printf ("could not send image fragment\n");
      send_close_connection (conn, MSG_TYPE_CLOSE_CONN);
      close_connection (conn);
      return false;
    }
  }

  free (image_buffer_start);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  if (!send_close_connection (conn, MSG_TYPE_CLOSE_CONN))
  {
    printf ("could not send close connection message\n");
  }
  close_connection (conn);

  return true;
}
