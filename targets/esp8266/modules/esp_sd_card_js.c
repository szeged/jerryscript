#include "esp_sd_card_js.h"
#include "jerryscript-port.h"

FATFS fatfs;
char vol[3];
static const char *results[] = {
        [FR_OK]                  = "Succeeded",
        [FR_DISK_ERR]            = "A hard error occurred in the low level disk I/O layer",
        [FR_INT_ERR]             = "Assertion failed",
        [FR_NOT_READY]           = "The physical drive cannot work",
        [FR_NO_FILE]             = "Could not find the file",
        [FR_NO_PATH]             = "Could not find the path",
        [FR_INVALID_NAME]        = "The path name format is invalid",
        [FR_DENIED]              = "Access denied due to prohibited access or directory full",
        [FR_EXIST]               = "Access denied due to prohibited access",
        [FR_INVALID_OBJECT]      = "The file/directory object is invalid",
        [FR_WRITE_PROTECTED]     = "The physical drive is write protected",
        [FR_INVALID_DRIVE]       = "The logical drive number is invalid",
        [FR_NOT_ENABLED]         = "The volume has no work area",
        [FR_NO_FILESYSTEM]       = "There is no valid FAT volume",
        [FR_MKFS_ABORTED]        = "The f_mkfs() aborted due to any problem",
        [FR_TIMEOUT]             = "Could not get a grant to access the volume within defined period",
        [FR_LOCKED]              = "The operation is rejected according to the file sharing policy",
        [FR_NOT_ENOUGH_CORE]     = "LFN working buffer could not be allocated",
        [FR_TOO_MANY_OPEN_FILES] = "Number of open files > _FS_LOCK",
        [FR_INVALID_PARAMETER]   = "Given parameter is invalid"
};

static inline bool failed (FRESULT res)
{
  bool fail = res != FR_OK;
  if (fail)
  {
    jerry_port_log (JERRY_LOG_LEVEL_ERROR, "%s\n", (const jerry_char_t * ) results[res]);
  }
  return fail;
}

uint32_t seek_file_size (FIL *file_p)
{
  work_buffer = malloc (WORK_BUFFER_SIZE);

  if (work_buffer == NULL)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to allocate work buffer.");
  }
  uint32_t file_size = 0;
  uint32_t read = 0;

  do
  {
    f_read (file_p, work_buffer, WORK_BUFFER_SIZE, &read);
    file_size += read;
  } while (read != 0);

  free (work_buffer);

  f_lseek (file_p, f_tell (file_p) - file_size);
  return file_size;
}

static void
native_freecb (void *native_p)
{
  free (native_p);
}

static const
jerry_object_native_info_t native_file_obj_type_info =
{
  .free_cb = native_freecb
};

const jerry_object_native_info_t* get_native_file_obj_type_info (void)
{
  return &native_file_obj_type_info;
}

static jerry_value_t
terminte_if_fail (FRESULT res, jerry_value_t *resource, char *operation)
{
  if (failed (res))
  {
    if (resource != NULL)
    {
      jerry_release_value (*resource);
    }

    free (work_buffer);
    char buffer[64];
    snprintf (buffer, sizeof (buffer), "%s%s%s", "Failed to process ", operation, " operation");
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) buffer);
  }

  return jerry_create_undefined ();
}

/* The string to be written can be bigger than the work_buffer capacity,
so we delete the file if necessarry and use append instead of create */
static bool
reopen_file_with_flag (FIL *file_p, jerry_char_t *path_buf_p, int8_t flag)
{
  int8_t param_flag = flag;
  if (flag == SD_CARD_USE_APPEND)
  {
    if ((flag & FA_CREATE_ALWAYS) != 0)
    {
      flag &= ~FA_CREATE_ALWAYS;
      flag |= FA_OPEN_APPEND;

      f_unlink ((TCHAR *) path_buf_p);
    }
  }

  uint8_t attempts = 0;


  while (failed (f_close (file_p)) || attempts++ < SD_CARD_MAX_F_IO_ATTEMPTS)
  {

    vTaskDelay (20 / portTICK_PERIOD_MS);
  }

  attempts = 0;
  while (attempts++ < SD_CARD_MAX_F_IO_ATTEMPTS && failed (f_open (file_p, (const TCHAR *) path_buf_p, flag)))
  {
    vTaskDelay (20 / portTICK_PERIOD_MS);
  }

  if (attempts == SD_CARD_MAX_F_IO_ATTEMPTS)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to open file.");
  }

  return param_flag == flag;
}

/**
 * Mount file system
 * params: args_p[0] - Chip select GPIO pin number
 */
DELCARE_HANDLER (sd_mount)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_MOUNT, "1");
  }

  if (!jerry_value_is_number (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_NUMBER);
  }

  int cs_pin = (int) jerry_get_number_value (args_p[0]);

  if (cs_pin >= 0 && cs_pin <= 16)
  {
    const char* cs_volume = f_gpio_to_volume (cs_pin);
    if (cs_volume == NULL)
    {
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "The 1st argument must be 0, 2, 4, 5, 15, 16");
    }
    strcpy (vol, cs_volume);
  }
  else
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Invalid GPIO number");
  }

  if (failed (f_mount (&fatfs, vol, 1)))
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to mount the SD card!");
  }

  if (failed (f_chdrive (vol)))
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to set default drive!");
  }

  return jerry_create_boolean (true);
}

/**
 * Unmount file system
 * params: none
 */
DELCARE_HANDLER (sd_unmount)
{
  if (args_cnt != 0)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_MOUNT, "0");
  }

  return jerry_create_boolean (!failed (f_mount (0, vol, 0)));
}

/**
 * Open file with the given name and flag
 * args_p[0] - path
 * args_p[1] - flag
 */
DELCARE_HANDLER (sd_open)
{
  if (args_cnt != 2)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_OPEN, "2");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  if (!jerry_value_is_string (args_p[1]))
  {
    return raise_argument_type_error ("2", TYPE_STRING);
  }

  jerry_size_t mode_req_sz = jerry_get_string_length (args_p[1]);
  if (mode_req_sz > 3)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Invalid mode flag!");
  }

  jerry_char_t mode_buf_p[mode_req_sz + 1];
  jerry_string_to_char_buffer (args_p[1],  mode_buf_p,  mode_req_sz);
  mode_buf_p[mode_req_sz] = 0;

  uint8_t flag;

  if (strcmp ((const char *) mode_buf_p, SD_CARD_READONLY) == 0)
  {
    flag = FA_READ;
  }
  else if (strcmp ((const char *) mode_buf_p, SD_CARD_READWRITE) == 0)
  {
    flag = FA_READ | FA_WRITE;
  }
  else if (strcmp ((const char *) mode_buf_p, SD_CARD_CREATE_EMPTY_FOR_WRITE) == 0)
  {
    flag = FA_CREATE_ALWAYS | FA_WRITE;
  }
  else if (strcmp ((const char *) mode_buf_p, SD_CARD_CREATE_EMPTY_FOR_READWRITE) == 0)
  {
    flag = FA_CREATE_ALWAYS | FA_WRITE | FA_READ;
  }
  else if (strcmp ((const char *) mode_buf_p, SD_CARD_APPEND_WRITE) == 0)
  {
    flag = FA_OPEN_APPEND | FA_WRITE;
  }
  else if (strcmp ((const char *) mode_buf_p, SD_CARD_APPEND_READWRITE) == 0)
  {
    flag = FA_OPEN_APPEND | FA_WRITE | FA_READ;
  }
  else if (strcmp ((const char *) mode_buf_p, SD_CARD_CREATE_EMPTY_FOR_WRITE_IF_NOT_EXISTS) == 0)
  {
    flag = FA_CREATE_NEW | FA_WRITE;
  }
  else if (strcmp ((const char *) mode_buf_p, SD_CARD_CREATE_EMPTY_FOR_READWRITE_IF_NOT_EXISTS) == 0)
  {
    flag = FA_CREATE_NEW | FA_WRITE | FA_READ;
  }
  else
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Invalid mode flag!");
  }

  jerry_size_t path_req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t path_buf_p[path_req_sz + 1];

  jerry_string_to_char_buffer (args_p[0], path_buf_p, path_req_sz);
  path_buf_p[path_req_sz] = 0;

  FIL *f = (FIL *) malloc (sizeof (FIL));
  if (f == NULL)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to allocate memory for file descriptor.");
  }

  uint8_t attempts = 0;
  while (attempts < SD_CARD_MAX_F_IO_ATTEMPTS && failed (f_open (f, (const TCHAR *)path_buf_p, flag)) )
  {
    vTaskDelay (20 / portTICK_PERIOD_MS);
    attempts++;
  }

  if (attempts == SD_CARD_MAX_F_IO_ATTEMPTS)
  {
    free (f);
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to open file.");
  }

  jerry_value_t file_descriptor = jerry_create_object ();
  jerry_set_object_native_pointer (file_descriptor, f, &native_file_obj_type_info);
  register_number_to_object (SD_CARD_OPEN_FLAG, flag, file_descriptor);
  register_string_to_object (SD_CARD_OPEN_PATH, (char *) path_buf_p, file_descriptor);

  return file_descriptor;
}

/**
 * Close the given file
 * args_p[0] - file file_descriptor
 */
DELCARE_HANDLER (sd_close)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_CLOSE, "1");
  }

  if (!jerry_value_is_object (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_OBJECT);
  }

  void *native_p;
  const jerry_object_native_info_t *type_p;
  bool has_p = jerry_get_object_native_pointer (args_p[0], &native_p, &type_p);

  if (has_p && type_p == &native_file_obj_type_info)
  {
    FIL *f = native_p;
    if (failed (f_close (f)))
    {
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to close file.");
    }
  }
  return jerry_create_boolean (true);
}

/**
 * args_p[0] - file object
 * args_p[1] - Data ot be written
 * args_p[2] - Binary or text mode
 * args_p[3] - Number of bytes
 */
DELCARE_HANDLER (sd_write)
{
  if (args_cnt != 3 && args_cnt != 4)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_WRITE, "3 or 4");
  }

  if (!jerry_value_is_object (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_OBJECT);
  }

  if (!jerry_value_is_boolean (args_p[2]))
  {
    return raise_argument_type_error ("3", TYPE_BOOLEAN);
  }

  if (args_cnt == 4 && !jerry_value_is_number (args_p[3]))
  {
    return raise_argument_type_error ("4", TYPE_NUMBER);
  }

  FIL *file_p = NULL;
  void *native_p;
  const jerry_object_native_info_t *type_p;
  bool has_p = jerry_get_object_native_pointer (args_p[0], &native_p, &type_p);

  if (has_p && type_p == &native_file_obj_type_info)
  {
    file_p = native_p;
  }
  else
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Argument 1 must be a file object.");
  }

  jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) SD_CARD_OPEN_FLAG);
  jerry_value_t prop_value = jerry_get_property (args_p[0], prop_name);
  jerry_release_value (prop_name);

  uint8_t original_flag = jerry_get_number_value (prop_value);
  jerry_release_value (prop_value);

  prop_name = jerry_create_string ((const jerry_char_t *) SD_CARD_OPEN_PATH);
  prop_value = jerry_get_property (args_p[0], prop_name);
  jerry_release_value (prop_name);

  jerry_size_t path_req_sz = jerry_get_string_length (prop_value);
  jerry_char_t path_buf_p[path_req_sz + 1];
  jerry_string_to_char_buffer (prop_value, path_buf_p, path_req_sz);
  path_buf_p[path_req_sz] = 0;
  jerry_release_value (prop_value);

  bool write_as_text = (args_cnt == 3) ? true : jerry_get_boolean_value (args_p[2]);
  bool flag_must_be_restrored = false;
  work_buffer = malloc (WORK_BUFFER_SIZE);

  if (work_buffer == NULL)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to allocate work buffer.");
  }

  jerry_value_t ret_val = jerry_create_undefined ();

  if (jerry_value_is_number (args_p[1]))
  {
    double number = jerry_get_number_value (args_p[1]);
    int32_t integer_number = (int32_t) number;
    if (number == integer_number)
    {
      if (write_as_text)
      {
        ret_val = terminte_if_fail (f_printf (file_p, "%d", number), NULL, SD_CARD_WRITE);
      }
      else
      {
        uint32_t written;
        ret_val = terminte_if_fail (f_write (file_p, (char *) &integer_number, sizeof (integer_number), &written),
                                    NULL, SD_CARD_WRITE);
      }
    }
    else
    {
      if (write_as_text)
      {
        char *string_buffer = work_buffer;
        snprintf (string_buffer, sizeof (work_buffer), "%.10lf", number);
        f_printf (file_p, "%s", string_buffer);
      }
      else
      {
        uint32_t written;
        ret_val = terminte_if_fail (f_write (file_p, (char *) &number, sizeof (number), &written),
                                    NULL, SD_CARD_WRITE);
      }
    }
  }
  else if (jerry_value_is_typedarray (args_p[1]) && !write_as_text)
  {
    int32_t bytes_to_write;

    jerry_length_t byteLength = 0;
    jerry_length_t byteOffset = 0;
    jerry_value_t buffer = jerry_get_typedarray_buffer (args_p[1], &byteOffset, &byteLength);

    if (jerry_value_is_number (args_p[3]))
    {
      int32_t number = (int32_t) jerry_get_number_value (args_p[3]);
      if (number > byteLength)
      {
        char buff[128];
        snprintf (buff, sizeof (buff), "%s%d%s%d", "ArrayBuffer overflow! ArrayBuffer byteLength: ", byteLength,
                                                    " desired bytes to write: ", number);
        free (work_buffer);
        return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) buff);
      }
      bytes_to_write = (number < WORK_BUFFER_SIZE) ? number : WORK_BUFFER_SIZE;
    }
    else
    {
      bytes_to_write = (byteLength < WORK_BUFFER_SIZE) ? byteLength : WORK_BUFFER_SIZE;
    }

    if (bytes_to_write > WORK_BUFFER_SIZE)
    {
      flag_must_be_restrored = reopen_file_with_flag (file_p, path_buf_p, SD_CARD_USE_APPEND);
    }

    while (bytes_to_write > 0)
    {
      jerry_size_t offset = (bytes_to_write < WORK_BUFFER_SIZE) ? bytes_to_write : WORK_BUFFER_SIZE;
      jerry_arraybuffer_read (buffer, byteOffset, work_buffer, offset);

      uint32_t written;
      ret_val = terminte_if_fail (f_write (file_p, work_buffer, (UINT) offset, &written),
                                  &buffer, SD_CARD_WRITE);

      byteOffset += offset;
      bytes_to_write -= offset;
    }
  }
  else
  {
    jerry_char_t *string_buffer = work_buffer;
    jerry_size_t str_start = 0;
    jerry_value_t string_value = jerry_value_is_string (args_p[1]) ? args_p[1] : jerry_value_to_string (args_p[1]);
    jerry_size_t string_req_sz = jerry_get_string_length (string_value);

    if (string_req_sz > WORK_BUFFER_SIZE)
    {
      flag_must_be_restrored = reopen_file_with_flag (file_p, path_buf_p, original_flag);
    }

    while (string_req_sz > 0)
    {
      jerry_size_t offset = (string_req_sz < WORK_BUFFER_SIZE) ? string_req_sz : WORK_BUFFER_SIZE;

      jerry_substring_to_char_buffer (string_value, str_start, str_start + offset, string_buffer, offset);
      string_buffer[offset] = 0;
      str_start += offset;

      if (write_as_text)
      {
        f_printf (file_p, "%*s", offset, string_buffer);
      }
      else
      {
        uint32_t written;
        ret_val = terminte_if_fail (f_write (file_p, string_buffer, offset, &written), &string_value, SD_CARD_WRITE);
      }

      string_req_sz -= offset;
    }

    if (!jerry_value_is_string (args_p[1]))
    {
      jerry_release_value (string_value);
    }
  }

  if (flag_must_be_restrored)
  {
    reopen_file_with_flag (file_p, path_buf_p, original_flag);
  }

  free (work_buffer);
  return jerry_value_is_error (ret_val) ? ret_val : jerry_create_boolean (true);
}

/**
 * args_p[0] - file object
 */
DELCARE_HANDLER (sd_file_size)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_FILE_SIZE, "1");
  }

  if (!jerry_value_is_object (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_OBJECT);
  }

  FIL *file_p;
  void *native_p;
  const jerry_object_native_info_t *type_p;
  bool has_p = jerry_get_object_native_pointer (args_p[0], &native_p, &type_p);

  if (has_p && type_p == &native_file_obj_type_info)
  {
    file_p = (FIL *) native_p;
  }
  else
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Argument 1 must be a file object.");
  }

  return jerry_create_number (seek_file_size (file_p));
}

/**
 * args_p[0] - path
 */
DELCARE_HANDLER (sd_mkdir)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_MKDIR, "1");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  jerry_size_t req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t str_buf_p[req_sz + 1];

  jerry_string_to_char_buffer (args_p[0], str_buf_p, req_sz);
  str_buf_p[req_sz] = 0;

  if (failed (f_mkdir ((const TCHAR *) str_buf_p)))
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to create directory");
  }

  return jerry_create_boolean (true);
}

/**
 * args_p[0] - path
 */
DELCARE_HANDLER (sd_file_exists)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_FILE_EXISTS, "1");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  jerry_size_t req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t str_buf_p[req_sz + 1];

  jerry_string_to_char_buffer (args_p[0], str_buf_p, req_sz);
  str_buf_p[req_sz] = 0;

  FILINFO fno;

  return jerry_create_boolean (!failed(f_stat ((const TCHAR *)str_buf_p, &fno)));
}

/**
 * args_p[0] - file obj
 */
DELCARE_HANDLER (sd_file_eof)
{
  if (args_cnt != 0)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_FILE_EOF, "0");
  }

  if (!jerry_value_is_object (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_OBJECT);
  }

  FIL *file_p;
  void *native_p;
  const jerry_object_native_info_t *type_p;
  bool has_p = jerry_get_object_native_pointer (args_p[0], &native_p, &type_p);

  if (has_p && type_p == &native_file_obj_type_info)
  {
    file_p = (FIL *) native_p;
  }
  else
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Argument 1 must be a file object.");
  }

  return jerry_create_boolean (((int)(file_p->fptr == seek_file_size (file_p))) != 0);
}

/**
 * args_p[0] - file object
 * args_p[1] - Binary or text mode
 * args_p[2] - Number of bytes
 */
DELCARE_HANDLER (sd_read)
{
  if (args_cnt != 2 && args_cnt != 3)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_WRITE, "2 or 3");
  }

  if (!jerry_value_is_object (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_OBJECT);
  }

  if (!jerry_value_is_boolean (args_p[1]))
  {
    return raise_argument_type_error ("2", TYPE_BOOLEAN);
  }

  if (args_cnt == 3 && !jerry_value_is_number (args_p[2]))
  {
    return raise_argument_type_error ("3", TYPE_NUMBER);
  }

  FIL *file_p = NULL;
  void *native_p;
  const jerry_object_native_info_t *type_p;
  bool has_p = jerry_get_object_native_pointer (args_p[0], &native_p, &type_p);

  if (has_p && type_p == &native_file_obj_type_info)
  {
    file_p = native_p;
  }
  else
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Argument 1 must be a file object.");
  }

  bool read_as_text = (args_cnt == 2) ? true : jerry_get_boolean_value (args_p[1]);

  uint32_t bytes_to_read = 0;
  if (args_cnt == 3)
  {
    uint32_t desired_bytes_to_read = jerry_get_number_value (args_p[2]);
    bytes_to_read = desired_bytes_to_read < WORK_BUFFER_SIZE ? desired_bytes_to_read : WORK_BUFFER_SIZE;
  }

  work_buffer = malloc (WORK_BUFFER_SIZE);
  if (work_buffer == NULL)
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Failed to allocate work buffer.");
  }

  jerry_value_t ret_val = jerry_create_undefined ();
  uint32_t read = 0;
  if (read_as_text)
  {
    char *succ = f_gets (work_buffer, WORK_BUFFER_SIZE, file_p);

    if (succ != NULL)
    {
      free (work_buffer);
      return jerry_create_string_sz ((const jerry_char_t *) work_buffer, strlen (work_buffer));
    }
  }
  else
  {
    ret_val = terminte_if_fail (f_read (file_p, work_buffer, bytes_to_read, &read), NULL, SD_CARD_READ);
    if (read != 0 && jerry_value_is_undefined (ret_val))
    {
      free (work_buffer);
      return jerry_create_string_sz ((const jerry_char_t *) work_buffer, read);
    }
  }

  free (work_buffer);
  return ret_val;

}

DIR find_dj;
FILINFO find_fno;
/**
 *  args_p[0] - path
 *  args_p[1] - patttern
 */
DELCARE_HANDLER (sd_find)
{
  if (args_cnt != 2)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_FIND, "2");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  if (!jerry_value_is_string (args_p[1]))
  {
    return raise_argument_type_error ("2", TYPE_STRING);
  }

  jerry_size_t path_req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t path_str_buf_p[path_req_sz];

  jerry_string_to_char_buffer (args_p[0], path_str_buf_p, path_req_sz);
  path_str_buf_p[path_req_sz] = 0;

  jerry_size_t pattern_req_sz = jerry_get_string_length (args_p[1]);
  jerry_char_t pattern_str_buf_p[pattern_req_sz];

  jerry_string_to_char_buffer (args_p[1], pattern_str_buf_p, pattern_req_sz);
  pattern_str_buf_p[pattern_req_sz] = 0;

  FRESULT fr = f_findfirst (&find_dj, &find_fno, (const TCHAR *) path_str_buf_p, (const TCHAR *) pattern_str_buf_p);

  if (fr == FR_OK && find_fno.fname[0])
  {
    return jerry_create_string ((const jerry_char_t*) find_fno.fname);
  }

  return jerry_create_undefined ();
}

/**
 */
DELCARE_HANDLER (sd_find_next)
{
  if (args_cnt != 0)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_FIND_NEXT, "0");
  }

  FRESULT fr = f_findnext (&find_dj, &find_fno);

  if (fr == FR_OK && find_fno.fname[0])
  {
    return jerry_create_string ((const jerry_char_t*) find_fno.fname);
  }

  return jerry_create_undefined ();

}

/**
 *  args_p[0] - old path
 *  args_p[1] - new path
 */
DELCARE_HANDLER (sd_rename)
{
  if (args_cnt != 2)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_READ, "2");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  if (!jerry_value_is_string (args_p[1]))
  {
    return raise_argument_type_error ("2", TYPE_STRING);
  }

  jerry_size_t old_path_req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t old_path_str_buf_p[old_path_req_sz + 1];

  jerry_string_to_char_buffer (args_p[0], old_path_str_buf_p, old_path_req_sz);
  old_path_str_buf_p[old_path_req_sz] = 0;

  jerry_size_t new_pattern_req_sz = jerry_get_string_length (args_p[1]);
  jerry_char_t new_pattern_str_buf_p[new_pattern_req_sz + 1];

  jerry_string_to_char_buffer (args_p[1], new_pattern_str_buf_p, new_pattern_req_sz);
  new_pattern_str_buf_p[new_pattern_req_sz] = 0;

  return jerry_create_boolean (!failed (f_rename ((const TCHAR *) old_path_str_buf_p, (const TCHAR *) new_pattern_str_buf_p)));
}

/**
 * args_p[0] - path
 */
DELCARE_HANDLER (sd_delete)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_DELETE, "1");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  jerry_size_t path_req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t path_str_buf_p[path_req_sz + 1];

  jerry_string_to_char_buffer (args_p[0], path_str_buf_p, path_req_sz);
  path_str_buf_p[path_req_sz] = 0;

  return jerry_create_boolean (!failed (f_unlink((const TCHAR *) path_str_buf_p)));
}

DIR search_dir;

/**
 * args_p[0] - path
 */
DELCARE_HANDLER (sd_open_directory)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_OPEN_DIRECTORY, "1");
  }

  if (!jerry_value_is_string (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_STRING);
  }

  jerry_size_t path_req_sz = jerry_get_string_length (args_p[0]);
  jerry_char_t path_str_buf_p[path_req_sz + 1];

  jerry_string_to_char_buffer (args_p[0], path_str_buf_p, path_req_sz);
  path_str_buf_p[path_req_sz] = 0;

  if (failed (f_opendir (&search_dir, (const TCHAR *) path_str_buf_p)))
  {
    return jerry_create_undefined ();
  }

  jerry_value_t directory_descriptor = jerry_create_object ();
  jerry_set_object_native_pointer (directory_descriptor, &search_dir, NULL);
  register_string_to_object (SD_CARD_DIRECTORY_PATH, (char *) path_str_buf_p, directory_descriptor);
  return directory_descriptor;
}

FILINFO dir_fno;

/**
 * args_p[0] - directory descriptor
 */
DELCARE_HANDLER (sd_read_directory_structure)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_READ_DIRECTORY_STRUCTURE, "1");
  }

  if (!jerry_value_is_object (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_OBJECT);
  }

  DIR *dir;
  void *native_p;
  const jerry_object_native_info_t *type_p;
  bool has_p = jerry_get_object_native_pointer (args_p[0], &native_p, &type_p);

  if (has_p)
  {
    dir = native_p;
  }
  else
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Argument 1 must be a directory object.");
  }

  if (!failed (f_readdir (dir, &dir_fno)) && dir_fno.fname[0] != 0)
  {
    jerry_value_t directory_struture_descriptor = jerry_create_object ();
    jerry_set_object_native_pointer (directory_struture_descriptor, &dir_fno, NULL);
    register_string_to_object (SD_CARD_DIRECTORY_STRUCTURE_ELEMENT, dir_fno.fname, directory_struture_descriptor);
    return directory_struture_descriptor;
  }

  return jerry_create_undefined();
}

/**
 * args_p[0] - directory structure descriptor
 */
DELCARE_HANDLER (sd_is_directory)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (SD_CARD_OBJECT_NAME, SD_CARD_IS_DIRECTORY, "1");
  }

  if (!jerry_value_is_object (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_OBJECT);
  }

  FILINFO *fno;
  void *native_p;
  const jerry_object_native_info_t *type_p;
  bool has_p = jerry_get_object_native_pointer (args_p[0], &native_p, &type_p);

  if (has_p)
  {
    fno = native_p;
  }
  else
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "Argument 1 must be a directory object.");
  }

  return jerry_create_boolean (fno->fattrib & AM_DIR);
}

void register_sd_card_object (jerry_value_t global_object)
{
  jerry_value_t sd_card_object = jerry_create_object ();
  register_js_value_to_object (SD_CARD_OBJECT_NAME, sd_card_object, global_object);

  register_native_function (SD_CARD_MOUNT, sd_mount_handler, sd_card_object);
  register_native_function (SD_CARD_UNMOUNT, sd_unmount_handler, sd_card_object);
  register_native_function (SD_CARD_OPEN, sd_open_handler, sd_card_object);
  register_native_function (SD_CARD_CLOSE, sd_close_handler, sd_card_object);
  register_native_function (SD_CARD_WRITE, sd_write_handler, sd_card_object);
  register_native_function (SD_CARD_READ, sd_read_handler, sd_card_object);
  register_native_function (SD_CARD_FILE_SIZE, sd_file_size_handler, sd_card_object);
  register_native_function (SD_CARD_MKDIR, sd_mkdir_handler, sd_card_object);
  register_native_function (SD_CARD_FILE_EXISTS, sd_file_exists_handler, sd_card_object);
  register_native_function (SD_CARD_FILE_EOF, sd_file_eof_handler, sd_card_object);
  register_native_function (SD_CARD_FIND, sd_find_handler, sd_card_object);
  register_native_function (SD_CARD_FIND_NEXT, sd_find_next_handler, sd_card_object);
  register_native_function (SD_CARD_RENAME, sd_rename_handler, sd_card_object);
  register_native_function (SD_CARD_DELETE, sd_delete_handler, sd_card_object);
  register_native_function (SD_CARD_OPEN_DIRECTORY, sd_open_directory_handler, sd_card_object);
  register_native_function (SD_CARD_READ_DIRECTORY_STRUCTURE, sd_read_directory_structure_handler, sd_card_object);
  register_native_function (SD_CARD_IS_DIRECTORY, sd_is_directory_handler, sd_card_object);

  register_boolean_to_object (SD_CARD_AS_TEXT, SD_CARD_WRITE_TEXT, sd_card_object);
  register_boolean_to_object (SD_CARD_AS_BINARY, SD_CARD_WRITE_BINARY, sd_card_object);

  register_string_to_object (SD_CARD_READONLY_PROP_NAME, SD_CARD_READONLY, sd_card_object);
  register_string_to_object (SD_CARD_READWRITE_PROP_NAME, SD_CARD_READWRITE, sd_card_object);
  register_string_to_object (SD_CARD_CREATE_EMPTY_FOR_WRITE_PROP_NAME, SD_CARD_CREATE_EMPTY_FOR_WRITE, sd_card_object);
  register_string_to_object (SD_CARD_CREATE_EMPTY_FOR_READWRITE_PROP_NAME, SD_CARD_CREATE_EMPTY_FOR_READWRITE, sd_card_object);
  register_string_to_object (SD_CARD_APPEND_WRITE_PROP_NAME, SD_CARD_APPEND_WRITE, sd_card_object);
  register_string_to_object (SD_CARD_APPEND_READWRITE_PROP_NAME, SD_CARD_APPEND_READWRITE, sd_card_object);
  register_string_to_object (SD_CARD_CREATE_EMPTY_FOR_WRITE_IF_NOT_EXISTS_PROP_NAME, SD_CARD_CREATE_EMPTY_FOR_WRITE_IF_NOT_EXISTS, sd_card_object);
  register_string_to_object (SD_CARD_CREATE_EMPTY_FOR_READWRITE_IF_NOT_EXISTS_PROP_NAME, SD_CARD_CREATE_EMPTY_FOR_READWRITE_IF_NOT_EXISTS, sd_card_object);

  jerry_release_value (sd_card_object);
}
