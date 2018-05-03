#include "esp_wifi_js.h"
#include "esp_sd_card_js.h"
#include "esp_sntp.h"
#include "jerryscript-port.h"

static bool wifi_been_connected = false;

DELCARE_HANDLER (wifi_disconnect)
{
  if (args_cnt != 0)
  {
    return raise_argument_count_error (WIFI_OBJECT_NAME, WIFI_CONNECT, "0");
  }

  sdk_wifi_station_disconnect ();
  wifi_been_connected = false;

  return jerry_create_boolean (true);
}

DELCARE_HANDLER (wifi_available)
{
  if (args_cnt != 0)
  {
    return raise_argument_count_error (WIFI_OBJECT_NAME, WIFI_AVAILABLE, "0");
  }

  return jerry_create_boolean (sdk_wifi_station_get_connect_status () == STATION_GOT_IP);
}

DELCARE_HANDLER (wifi_connect)
{
  if (args_cnt != 2)
  {
    return raise_argument_count_error (WIFI_OBJECT_NAME, WIFI_CONNECT, "2");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  if (!jerry_value_is_string (args_p[1]))
  {
    return raise_argument_type_error ("2", TYPE_STRING);
  }

  if (wifi_been_connected)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Wifi is already connected!");
  }

  struct sdk_station_config config;
  jerry_size_t ssid_req_sz = jerry_get_string_length (args_p[0]);
  jerry_size_t password_req_sz = jerry_get_string_length (args_p[1]);
  uint32_t buffer_size = ssid_req_sz > password_req_sz ? ssid_req_sz + 1 : password_req_sz + 1;
  jerry_char_t *str_buf_p;
  str_buf_p = (jerry_char_t *) malloc (sizeof (jerry_char_t) * buffer_size);

  jerry_string_to_char_buffer (args_p[0], str_buf_p, ssid_req_sz);
  str_buf_p[ssid_req_sz] = 0;
  strcpy((char *) config.ssid, (char *) str_buf_p);
  jerry_string_to_char_buffer (args_p[1], str_buf_p, password_req_sz);
  str_buf_p[password_req_sz] = 0;
  strcpy((char *) config.password, (char *) str_buf_p);
  free (str_buf_p);
  sdk_wifi_set_opmode (STATION_MODE);
  sdk_wifi_station_set_config (&config);
  sdk_wifi_station_connect ();
  sdk_wifi_station_set_auto_connect (0);

  uint8_t attempts = 0;
  while (sdk_wifi_station_get_connect_status () != STATION_GOT_IP && attempts < WIFI_MAX_CONNECT_ATTEMPTS)
  {
    vTaskDelay (100 / portTICK_PERIOD_MS);
    taskYIELD();
    attempts++;
  }
  if (attempts == WIFI_MAX_CONNECT_ATTEMPTS)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Cannot connect to the given AP!");
  }

  if (!sntp_been_init())
  {
    init_esp_sntp ();
  }

  wifi_been_connected = true;
  return jerry_create_boolean (true);
}

struct netconn *conn;
err_t err;

static bool send_package_size (jerry_size_t data_length)
{
  char *package_size_buff;
  package_size_buff = (char *) malloc (sizeof (char) * 8);
  if (package_size_buff == NULL)
  {
    return false;
  }

  package_size_buff[0] = '#';
  itoa ((int) data_length, package_size_buff + 1, 16);
  uint8_t num_of_digits = strlen (package_size_buff);
  package_size_buff[num_of_digits++] = '#';
  err_t result = netconn_write (conn, package_size_buff, num_of_digits, NETCONN_COPY);
  free (package_size_buff);

  return result == ERR_OK;
}

static bool send_package_data (jerry_size_t buffer_size, jerry_char_t *data_buffer)
{
  uint32_t buffer_length = buffer_size + 2;
  char *buffer;
  buffer = (char *) malloc (sizeof(char) * buffer_length);

  if (buffer == NULL)
  {
    return false;
  }

  buffer[0] = '&';
  memcpy (buffer + 1, (char *) data_buffer, buffer_size);
  buffer[buffer_size + 1] = '&';
  err_t result = netconn_write (conn, buffer, buffer_length, NETCONN_COPY);
  free (buffer);
  return result == ERR_OK;
}

UINT out_stream (const BYTE *p, UINT btf)
{
    if (btf == 0)
    {
      return 1;
    }

    if (!send_package_size (btf))
    {
      return 0;
    }

    uint32_t buffer_length = btf + 2;
    char *buffer;
    buffer = (char *) malloc (sizeof(char) * buffer_length);

    if (buffer == NULL)
    {
      return false;
    }
    buffer[0] = '&';
    memcpy (buffer + 1, p, btf);
    buffer[btf + 1] = '&';
    err_t result = netconn_write (conn, buffer, buffer_length, NETCONN_COPY);
    free (buffer);
    return result == ERR_OK ? btf : 0;
}

static bool forward_data_on_tcp (FIL *fil, uint32_t to_send)
{
  FRESULT rc = FR_OK;
  uint32_t sent = 0;

  while (rc == FR_OK && sent < to_send)
  {
    UINT dmy;
    rc = f_forward(fil, out_stream, WIFI_PACKAGE_SIZE, &dmy);
    sent += dmy;
  }

  return rc == FR_OK;
}

static jerry_value_t
send_data_on_tcp (jerry_value_t source, uint32_t bytes_to_send, const char *server, uint32_t port,
                  jerry_char_t *file_name, jerry_size_t file_name_length, FIL *forward)
{
  conn = netconn_new (NETCONN_TCP);

  if (conn == NULL)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to allocate socket!");
  }

  ip_addr_t addr;
  err = netconn_gethostbyname(server, &addr);
  if (err)
  {
    netconn_delete(conn);
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed find given host!");
  }

  netconn_connect(conn, &addr, port);

  if (file_name != NULL)
  {
    if (!send_package_size (file_name_length))
    {
      netconn_close (conn);
      netconn_delete (conn);
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t * ) "Failed to send header!");
    }

    if (!send_package_data (file_name_length, file_name))
    {
      netconn_close (conn);
      netconn_delete (conn);
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t * ) "Failed to send data!");
    }
  }

  message_buffer = malloc (sizeof (char) * WIFI_PACKAGE_SIZE);

  if (message_buffer == NULL)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t * ) "Failed to allocate memory for message buffer!");
  }

  if (forward != NULL)
  {
    if (!forward_data_on_tcp (forward, bytes_to_send))
    {
      netconn_close (conn);
      netconn_delete (conn);
      free (message_buffer);
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t * ) "Failed to send header!");
    }
  }
  else
  {
    uint32_t byteOffset = 0;
    while (bytes_to_send > 0)
    {
      uint32_t offset = bytes_to_send < WIFI_PACKAGE_SIZE ? bytes_to_send : WIFI_PACKAGE_SIZE;
      jerry_substring_to_char_buffer (source, byteOffset, byteOffset + offset, message_buffer, offset);

      if (!send_package_size (offset))
      {
        netconn_close (conn);
        netconn_delete (conn);
        free (message_buffer);
        return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t * ) "Failed to send header!");
      }

      if (!send_package_data (offset, message_buffer))
      {
        netconn_close (conn);
        netconn_delete (conn);
        free (message_buffer);
        return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t * ) "Failed to send data!");
      }

      bytes_to_send -= offset;
      byteOffset += offset;
    }
  }

  free (message_buffer);

  netconn_close (conn);
  netconn_delete (conn);

  return jerry_create_boolean (true);
}

/*
  args_p[0] - Host
  args_p[1] - port
  args_p[2] - data to be send
  args_p[3] - name
  args_p[4] - data length
*/
DELCARE_HANDLER (wifi_send)
{
  if (args_cnt != 5)
  {
    return raise_argument_count_error (WIFI_OBJECT_NAME, WIFI_SEND, "5");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  if (!jerry_value_is_number (args_p[1]))
  {
    return raise_argument_type_error ("1", TYPE_NUMBER);
  }

  if (!jerry_value_is_string (args_p[3]))
  {
    return raise_argument_type_error ("3", TYPE_STRING);
  }

  if (!jerry_value_is_number (args_p[4]))
  {
    return raise_argument_type_error ("4", TYPE_NUMBER);
  }

  jerry_size_t req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t str_buf_p[req_sz + 1];
  jerry_string_to_char_buffer (args_p[0], str_buf_p, req_sz);
  str_buf_p[req_sz] = 0;

  uint32_t port = jerry_get_number_value (args_p[1]);

  jerry_size_t file_name_req_sz = jerry_get_string_length (args_p[3]);
  jerry_char_t file_name_buf_p[file_name_req_sz + 1];
  jerry_string_to_char_buffer (args_p[3], file_name_buf_p, file_name_req_sz);
  file_name_buf_p[file_name_req_sz] = 0;

  uint32_t data_length = jerry_get_number_value (args_p[4]);
  jerry_value_t source = jerry_create_undefined();

  if (jerry_value_is_object (args_p[2]))
  {
    void *native_p;
    const jerry_object_native_info_t *type_p;
    bool has_p = jerry_get_object_native_pointer (args_p[2], &native_p, &type_p);
    if (has_p && type_p == get_native_file_obj_type_info())
    {
      FIL *f = native_p;
      send_data_on_tcp (0, data_length, (const char *) str_buf_p, port, file_name_buf_p, file_name_req_sz, f);
      return jerry_create_boolean (true);
    }
  }

  source = jerry_value_to_string (args_p [2]);
  return send_data_on_tcp (source, data_length, (const char *) str_buf_p, port, file_name_buf_p, 0, NULL);
}

/*
  args[0] address
  args[1] port
  args[2] path
*/
DELCARE_HANDLER (wifi_receive)
{
  if (args_cnt != 3)
  {
    return raise_argument_count_error (WIFI_OBJECT_NAME, WIFI_RECEIVE, "3");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  if (!jerry_value_is_number (args_p[1]))
  {
    return raise_argument_type_error ("2", TYPE_NUMBER);
  }

  if (!jerry_value_is_string (args_p[2]))
  {
    return raise_argument_type_error ("3", TYPE_STRING);
  }

  jerry_size_t address_req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t address_str_buf_p[address_req_sz + 1];

  jerry_string_to_char_buffer (args_p[0], address_str_buf_p, address_req_sz);
  address_str_buf_p[address_req_sz] = 0;

  jerry_value_t port_string = jerry_value_to_string (args_p[1]);
  jerry_size_t port_req_sz = jerry_get_string_length (port_string);
  jerry_char_t port_str_buf_p[port_req_sz + 1];

  jerry_string_to_char_buffer (port_string, port_str_buf_p, port_req_sz);
  port_str_buf_p[port_req_sz] = 0;

  jerry_release_value (port_string);

  jerry_size_t path_req_sz = jerry_get_string_length (args_p[2]);
  jerry_char_t path_str_buf_p[path_req_sz + 1];

  jerry_string_to_char_buffer (args_p[2], path_str_buf_p, path_req_sz);
  path_str_buf_p[path_req_sz] = 0;

  const struct addrinfo hints =
  {
      .ai_family = AF_UNSPEC,
      .ai_socktype = SOCK_STREAM,
  };

  struct addrinfo *res;
  int err = getaddrinfo ((const char *) address_str_buf_p, (const char *) port_str_buf_p, &hints, &res);

  if (err != 0 || res == NULL)
  {
    if (res)
    {
      freeaddrinfo(res);
    }
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t * ) "DNS lookup failed!");
  }

  struct sockaddr *sa = res->ai_addr;

  if (sa->sa_family == AF_INET)
  {
    inet_ntoa (((struct sockaddr_in *) sa)->sin_addr);
  }

  int s = socket (res->ai_family, res->ai_socktype, 0);
  if (s < 0)
  {
    freeaddrinfo (res);
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t * ) "Failed to allocate socket!");
  }

  if (connect (s, res->ai_addr, res->ai_addrlen) != 0)
  {
    close (s);
    freeaddrinfo (res);
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t * ) "Failed to connect!");
  }

  freeaddrinfo (res);

  message_buffer = malloc (WIFI_PACKAGE_SIZE * 2);

  if (message_buffer == NULL)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to allocate work buffer.");
  }

  snprintf ((char *) message_buffer, WIFI_PACKAGE_SIZE, "GET %s HTTP/1.0\r\nHost: %s:%s\r\nConnection: close\r\n\r\n",
           (const char *) path_str_buf_p, (const char *) address_str_buf_p, port_str_buf_p);

  uint32_t written = write (s, message_buffer, strlen ((char *) message_buffer));

  if (written < 0)
  {
    free (message_buffer);
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t * ) "Socket send failed!");
  }

  jerry_value_t result = jerry_create_undefined ();
  int r = 0;
  do
  {
    r = read (s, message_buffer, WIFI_PACKAGE_SIZE);
    if (r != 0)
    {
      char *data = strstr ((char *) message_buffer, "\r\n\r\n");
      if (data != NULL)
      {
        result = jerry_create_string_sz ((const jerry_char_t *) data + 4, (jerry_size_t) (r - (data + 4 - (char *) message_buffer)));
      }
    }
  } while (r == 0);

  free (message_buffer);
  close (s);
  return result;
}

void
register_wifi_object (jerry_value_t global_object)
{
  jerry_value_t wifi_object = jerry_create_object ();
  register_js_value_to_object (WIFI_OBJECT_NAME, wifi_object, global_object);

  register_native_function (WIFI_CONNECT, wifi_connect_handler, wifi_object);
  register_native_function (WIFI_DISCONNECT, wifi_disconnect_handler, wifi_object);
  register_native_function (WIFI_AVAILABLE, wifi_available_handler, wifi_object);
  register_native_function (WIFI_SEND, wifi_send_handler, wifi_object);
  register_native_function (WIFI_RECEIVE, wifi_receive_handler, wifi_object);

  jerry_release_value (wifi_object);
}
