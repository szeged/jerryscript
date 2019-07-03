#include "jerry_extapi.h"
#include "user_camera_live.h"

typedef enum {
  IMAGE_FORMAT_8_BIT = 0x03,
  IMAGE_FORMAT_16_RAW = 0x08,
  IMAGE_FORMAT_16_RAW_RGB = 0x06,
  IMAGE_FORMAT_JPEG = 0x07,
} image_format_t;

typedef enum {
  RAW_RESOLUTION_80_60 = 0x01,
  RAW_RESOLUTION_160_120 = 0x05,
  RAW_RESOLUTION_128_128 = 0x09,
  RAW_RESOLUTION_128_96 = 0x0B,
} RAW_resolution_t;

typedef enum {
  JPEG_RESOLUTION_160_128 = 0x03,
  JPEG_RESOLUTION_320_240 = 0x05,
  JPEG_RESOLUTION_640_480 = 0x07,
} JPEG_resolution_t;

typedef enum {
  IMAGE_TYPE_SNAPSHOT = 0x01,
  IMAGE_TYPE_RAW = 0x02,
  IMAGE_TYPE_JPEG = 0x05,
} image_type_t;

typedef enum {
  CMD_FIST_BYTE = 0xAA,
  CMD_INITIAL = 0x01,
  CMD_GET_PICTURE = 0x04,
  CMD_SNAPSHOT = 0x05,
  CMD_SET_PACKAGE_SIZE = 0x06,
  CMD_SET_BAUD_RATE = 0x07,
  CMD_RESET = 0x08,
  CMD_DATA = 0x0A,
  CMD_SYNC = 0x0D,
  CMD_ACK = 0x0E,
  CMD_NAK = 0x0F,
  CMD_LIGHT = 0x13,
  CMD_CBE = 0x14,
  CMD_SLEEP = 0x15,
} command_type_t;

static const uint8_t initial_command[] = { CMD_FIST_BYTE, CMD_INITIAL, 0x00, IMAGE_FORMAT_JPEG, RAW_RESOLUTION_80_60, JPEG_RESOLUTION_320_240 };
static const uint8_t sync_command[] = { CMD_FIST_BYTE, CMD_SYNC, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t sync_final_command[] = { CMD_FIST_BYTE, CMD_ACK, CMD_SYNC, 0x00, 0x00, 0x00 };
static const uint8_t generic_ack_reply[] = { CMD_FIST_BYTE, CMD_ACK, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t sync_ack_reply_ext[] = { CMD_FIST_BYTE, CMD_SYNC, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t pack_size[] = { CMD_FIST_BYTE, CMD_SET_PACKAGE_SIZE, 0x08, 0x80, 0x00, 0x00 }; // 128B
static const uint8_t get_picture[] = {CMD_FIST_BYTE, CMD_GET_PICTURE, IMAGE_TYPE_JPEG, 0x00, 0x00, 0x00 };


static const uint8_t command_length = 6;
static const uint8_t sync_attempts_max = 5;
static uint8_t *imageBufferStart;
static uint8_t *imageBuffer;
static int image_size = 0;

static bool send_image_size (uint32_t size, netconn_t conn);
static bool send_image (uint8_t *data_p, uint32_t size, netconn_t conn);

static void serial_flush ()
{
  while (uart_rxfifo_wait (UART_NUM, 0))
  {
    uart_getc (UART_NUM);
  }
}

static void delay_millies (int ms)
{
  vTaskDelay (ms / portTICK_PERIOD_MS);
}

static void reset_device (int port)
{
  gpio_write (port, 0);
  delay_millies(900);
  gpio_write (port, 1);
}

static void serial_write_buffer (const uint8_t* buff, int len)
{
  for (size_t i = 0; i < len; i++) {
    uart_putc (UART_NUM, buff[i]);
  }
}

static bool serial_available()
{
  return uart_rxfifo_wait (UART_NUM, 0);
}

static int serial_read()
{
  return uart_getc (UART_NUM);
}

static int serial_read_no_wait()
{
  return uart_getc_nowait (UART_NUM);
}

static uint8_t _commandID (const uint8_t* cmd)
{
  return cmd[1];
}

static bool _waitForBytes(const uint8_t* sentCommand, const uint8_t* ackReply)
{
  uint8_t reply[6];

  memcpy (reply, ackReply, command_length);

  if (ackReply != sync_ack_reply_ext) {
    reply[2] = _commandID(sentCommand);
  }

  int found_bytes = 0;
  int i = 0;

  while (serial_available() && i < command_length) {
    if (serial_read() == reply[i] || reply[i] == 0) {
      found_bytes++;
    }
    i++;
  }

  return found_bytes == command_length;
}

static bool attempt_sync ()
{
  int attempts = 0;


  while (attempts < 60)
  {
    serial_flush();
    serial_write_buffer(sync_command, command_length);

    delay_millies(50 + attempts);

    if (_waitForBytes (sync_command, generic_ack_reply))
    {
      if (_waitForBytes (sync_command, sync_ack_reply_ext))
      {
        delay_millies(50);
        serial_write_buffer (sync_final_command, command_length);
        return true;
      }
    }
    attempts++;
  }

  return false;
}

static bool _sendCommand (const uint8_t* cmd)
{
  delay_millies(75);
  serial_write_buffer(cmd, command_length);
  delay_millies(75);

  return _waitForBytes(cmd, generic_ack_reply);
}

static bool _sendInitial ()
{
  return _sendCommand(initial_command);
}

static bool _setPackageSize()
{

  return _sendCommand(pack_size);
}

static bool _getPicture()
{
  int ack[6];
  if (_sendCommand (get_picture)) {
    for (int i = 0; i < 6; i++) {
      while (!serial_available());
      ack[i] = serial_read();
    }

    image_size = ack[5];
    image_size = (image_size << 8) | ack[4];
    image_size = (image_size << 8) | ack[3];

    if (image_size > 0)
    {
      /* 1 extra byte for package type info */
      imageBufferStart = malloc ((image_size + 1) * sizeof (uint8_t));
      imageBuffer = imageBufferStart + 1;

      if (imageBufferStart == NULL)
      {
        return false;
      }
    }

    return image_size > 0;
  }
  return false;
}

static bool init ()
{
  uart_set_baud (UART_NUM, UART_BAUD_RATE);
  gpio_enable (CAMERA_RESET_PIN, 1);
  reset_device (CAMERA_RESET_PIN);

  int attempts = 0;

  while (attempts < sync_attempts_max)
  {
    printf ("Attempt %d\n", attempts);

    if (attempt_sync ())
    {
      printf ("CONNECTED\n");
      delay_millies (2000);
      return true;
    }

    reset_device(CAMERA_RESET_PIN);
    attempts++;
  }

  return false;
}

static bool storePicture (netconn_t conn)
{
  uint8_t ack[] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};
  int bytes;
  uint32_t image_pos = image_size;
  const uint32_t imageBuffer_size = 122;

  send_image_size (image_size, conn);

  int counter = 0;
  uint8_t* buffer_iter_p = imageBuffer;
  while (image_pos > 0) {
    if (image_pos < imageBuffer_size) {
      bytes = image_pos + command_length;
    }
    else {
      bytes = imageBuffer_size + command_length;
    }
    ack[4] = counter++;
    serial_write_buffer(ack, command_length);
    int attempts = 0;
    while (serial_available() != bytes && attempts++ < sync_attempts_max) {
      delay_millies(10);
    }

    if (attempts == sync_attempts_max){
      printf ("Failed to store data!");
      return false;
    }

    /* data id */
    for (int i = 0; i < 4; i++) {
      serial_read_no_wait();
    }

    /* data */
    int end = bytes - 6;
    for (int i = 0; i < end; i++) {
      buffer_iter_p[i] = serial_read_no_wait();
    }

    /* crc */
    for (int i = 0; i < 2; i++) {
      serial_read_no_wait();
    }

    buffer_iter_p += end;
    image_pos -= end;
  }

  send_image (imageBufferStart, image_size, conn);

  ack[4] = 0xF0;
  ack[5] = 0xF0;
  serial_write_buffer(ack, command_length);
  free (imageBufferStart);
  return true;
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

static bool initialize_connection (netconn_t *conn_p)
{
  if (!wifi_connect (LIVE_WIFI_SSID, LIVE_WIFI_PWD))
  {
    return false;
  }

  printf("WIFI connected\n");

  netconn_t conn = netconn_new (NETCONN_TCP);

  if (conn == NULL)
  {
    return false;
  }

  ip_addr_t addr;
  err_t err = netconn_gethostbyname (LIVE_SERVER_ADDR, &addr);

  if (err != ERR_OK)
  {
    netconn_delete (conn);
    return false;
  }

  err = netconn_connect (conn, &addr, LIVE_SERVER_PORT);

  if (err != ERR_OK)
  {
    netconn_delete (conn);
    return false;
  }

  *conn_p = conn;
  printf("Socket created\n");
  return true;
}

static void close_connection (netconn_t conn)
{
  netconn_close (conn);
  netconn_delete (conn);
  printf("Socket closed\n");
}

typedef enum {
  MSG_TYPE_IMAGE_SIZE = 0,
  MSG_TYPE_IMAGE_DATA = 1,
  MSG_TYPE_ABORT_CONN = 2,
  MSG_TYPE_CLOSE_CONN = 3,
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

static bool send_image(uint8_t *data_p, uint32_t size, netconn_t conn)
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

static bool send_close_connection(netconn_t conn, message_type_t reason)
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

bool user_camera_live(uint32_t timeout)
{
  if (init ())
  {
    if(_sendInitial())
    {
      if(_setPackageSize())
      {
        printf("uCam configuration succesful\n");

        netconn_t conn;

        if (!initialize_connection (&conn)) {
          printf("Server connection has failed\n");
          return false;
        }

        int start = (int)(sdk_system_get_time() / 1000000);
        int counter = 0;
        while (true)
        {
          if (_getPicture ())
          {
            if (storePicture(conn))
            {
              printf("Picture %03d has been taken\n", ++counter);
            }
          }
          else
          {
            printf ("Taking picture has failed\n");
          }

          int end = (int)(sdk_system_get_time() / 1000000);
          if (end - start > timeout)
          {
            break;
          }
        }
        send_close_connection (conn, MSG_TYPE_CLOSE_CONN);
        close_connection (conn);
        return true;
      }
    }
  }
  else
  {
    printf ("Sync with uCam has failed\n");
  }

  return false;
}
