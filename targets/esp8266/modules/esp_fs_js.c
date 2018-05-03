#include "esp_fs_js.h"

static bool local_init_spiffs ()
{
  esp_spiffs_init();
  return (esp_spiffs_mount () == SPIFFS_OK);
}

static void local_deinit_spiffs ()
{
  SPIFFS_unmount(&fs);
  esp_spiffs_deinit();
}

static bool open_file_success (const jerry_char_t *path, int flag, spiffs_file *fd)
{
  *fd = SPIFFS_open (&fs, (const char *) path, flag, 0);
  return !(*fd < 0);
}

/**
 * args_p[0] - path
 * args_p[1] - content
 */
DELCARE_HANDLER (fs_write)
{
  if (args_cnt != 2)
  {
    return raise_argument_count_error (FS_OBJECT_NAME, FS_WRITE, "2");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  jerry_size_t path_req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t path_str_buf_p[path_req_sz + 1];
  jerry_string_to_char_buffer (args_p[0], path_str_buf_p, path_req_sz);
  path_str_buf_p[path_req_sz] = 0;

  jerry_value_t to_string = jerry_value_to_string (args_p[1]);

  jerry_size_t content_req_sz = jerry_get_string_length (to_string);
  jerry_char_t content_str_buf_p[content_req_sz + 1];
  jerry_string_to_char_buffer (to_string, content_str_buf_p, content_req_sz);
  content_str_buf_p[content_req_sz] = 0;

  jerry_release_value (to_string);

  int written;
  if (local_init_spiffs())
  {
    spiffs_file fd;
    if (open_file_success (path_str_buf_p, SPIFFS_O_WRONLY | SPIFFS_O_CREAT | O_TRUNC, &fd))
    {
      written = SPIFFS_write (&fs, fd, content_str_buf_p, content_req_sz);
      SPIFFS_close(&fs, fd);
    }
    else
    {
      local_deinit_spiffs ();
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Cannot read file!");
    }
  }
  else
  {
    local_deinit_spiffs();
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Cannot init SPIFFS");
  }

  local_deinit_spiffs ();
  return jerry_create_boolean (written == content_req_sz);
}

/**
 * args_p[0] - path
 */
DELCARE_HANDLER (fs_read)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (FS_OBJECT_NAME, FS_READ, "1");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  jerry_size_t path_req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t path_str_buf_p[path_req_sz + 1];
  jerry_string_to_char_buffer (args_p[0], path_str_buf_p, path_req_sz);
  path_str_buf_p[path_req_sz] = 0;

  char *buffer;
  buffer = (char *) malloc (sizeof (char) * FS_BUFFER_SIZE);
  if (buffer == NULL)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Cannot allocate enough mermory!");
  }
  jerry_value_t result = jerry_create_undefined ();

  if (local_init_spiffs ())
  {
    spiffs_file fd;
    if (open_file_success (path_str_buf_p, SPIFFS_O_RDONLY, &fd))
    {
      uint32_t bytes_read = SPIFFS_read (&fs, fd, buffer, sizeof (char) * FS_BUFFER_SIZE);
      if (bytes_read >= 0){
        result = jerry_create_string_sz ((const jerry_char_t *) buffer, bytes_read);
      }
      SPIFFS_close (&fs, fd);
    }
    else
    {
      result = jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Cannot read file!");
    }
  }
  else
  {
    result = jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Cannot init SPIFFS");
  }

  free (buffer);
  local_deinit_spiffs ();

  return result;
}

/**
 * args_p[0] - path
 */
DELCARE_HANDLER (fs_exists)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (FS_OBJECT_NAME, FS_READ, "1");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  jerry_size_t path_req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t path_str_buf_p[path_req_sz + 1];
  jerry_string_to_char_buffer (args_p[0], path_str_buf_p, path_req_sz);
  path_str_buf_p[path_req_sz] = 0;
  bool success = false;
  if (local_init_spiffs ())
  {
    spiffs_file fd;
    success = open_file_success (path_str_buf_p, O_RDONLY, &fd);
    if (success)
    {
      SPIFFS_close (&fs, fd);
    }
  }
  else
  {
    local_deinit_spiffs ();
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Cannot init SPIFFS");
  }

  local_deinit_spiffs ();
  return jerry_create_boolean (success);
}

/**
 * args_p[0] - path
 */
DELCARE_HANDLER (fs_remove)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (FS_OBJECT_NAME, FS_READ, "1");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  jerry_size_t path_req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t path_str_buf_p[path_req_sz + 1];
  jerry_string_to_char_buffer (args_p[0], path_str_buf_p, path_req_sz);
  path_str_buf_p[path_req_sz] = 0;

  bool success = false;
  if (local_init_spiffs ())
  {
    success = (SPIFFS_remove(&fs, (const char *) path_str_buf_p) >= 0);
  }
  else
  {
    local_deinit_spiffs ();
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Cannot init SPIFFS");
  }

  local_deinit_spiffs ();
  return jerry_create_boolean (success);
}


void register_fs_object (jerry_value_t global_object)
{
  jerry_value_t fs_object = jerry_create_object ();
  register_js_value_to_object (FS_OBJECT_NAME, fs_object, global_object);

  register_native_function (FS_WRITE, fs_write_handler, fs_object);
  register_native_function (FS_READ, fs_read_handler, fs_object);
  register_native_function (FS_EXISTS, fs_exists_handler, fs_object);
  register_native_function (FS_REMOVE, fs_remove_handler, fs_object);

  jerry_release_value (fs_object);
}
