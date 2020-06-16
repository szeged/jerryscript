#include "esp_arducam_js.h"

#define MAX_FIFO_SIZE 0x7ffff // 8M

static netconn_t conn;
static char err_msg[255];

DELCARE_HANDLER(arducam_init)
{
  /* --------------------- */
  /* INIT */
  wait (5000);
  const spi_settings_t settings = {
    .mode = SPI_MODE0,
    .freq_divider = SPI_FREQ_DIV_8M,
    .msb = true, // ???
    .endianness = SPI_BIG_ENDIAN, // ???
    .minimal_pins = true
  };

  gpio_enable (CAMERA_CS, GPIO_OUTPUT);
  spi_cs_high (CAMERA_CS);
  gpio_enable (SD_CS, GPIO_OUTPUT);
  spi_cs_high (SD_CS);

  int spi_succ = spi_set_settings (SPI_BUS, &settings);
  if (!spi_succ)
  {
    sprintf (err_msg, "SPI init failed, code: %d", spi_succ);
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) err_msg);
  }

  i2c_set_clock_stretch (I2C_BUS, 10);
  int i2c_succ = i2c_init (I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ_100K);
  if (i2c_succ)
  {
    sprintf (err_msg, "I2C init failed, code: %d", i2c_succ);
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) err_msg);
  }

  write_reg (CAMERA_CS, 0x07, 0x80);
  wait (100);
  write_reg (CAMERA_CS, 0x07, 0x00);
  wait (100);

  // Check that SPI and I2C interface works correctly.
  write_reg (CAMERA_CS, REG_TEST, 0x55);
  uint8_t test = read_reg (CAMERA_CS, REG_TEST);
  printf ("%#x\n", test);
  if (test != 0x55)
  {
    sprintf (err_msg, "SPI interface error, expected: 0x55, received: %#x", test);
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) err_msg);
  }

  wr_sensor_reg_16_8 (0xff, 0x01);
  uint8_t vid = rd_sensor_reg_16_8 (OV5642_CHIPID_HIGH);
  uint8_t pid = rd_sensor_reg_16_8 (OV5642_CHIPID_LOW);
  printf ("vid: %#x\n", vid);
  printf ("pid: %#x\n", pid);
  if (!(vid == 0x56 && pid == 0x42))
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "I2C interface error");
  }

  init_cam ();

  if (!initialize_connection_wrapper (&conn)) {
    printf ("Server connection has failed\n");
  }

  return jerry_create_undefined ();
}

DELCARE_HANDLER(arducam_capture)
{
  write_reg (CAMERA_CS, REG_TIMING_CONTROL, MASK_VSYNC_LEVEL);
  set_image_size (OV5642_640x480);

  write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  // Start capture.
  write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_START_CAPTURE);
  // Wait until capture is complete.
  while (!get_bit (CAMERA_CS, REG_TRIG, MASK_CAPTURE_DONE));

  printf ("Capture done!\n");

  uint32_t image_size = read_fifo_length ();
  printf ("image_size: %d\n", image_size);

  if (image_size >= MAX_FIFO_SIZE) //8M
  {
    sprintf (err_msg, "Over size.\n");
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) err_msg);
  }
  if (image_size == 0) //0 kb
  {
    sprintf (err_msg, "Size is 0.\n");
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) err_msg);
  }

  return jerry_create_undefined ();
}

DELCARE_HANDLER(arducam_send)
{
  send_picture (conn);

  return jerry_create_undefined ();
}

DELCARE_HANDLER(arducam_print)
{
  uint8_t temp, temp_last;
  temp = temp_last = 0;

  spi_cs_low (CAMERA_CS);
  spi_transfer_8 (SPI_BUS, REG_FIFO_BURST_READ);

  while ( (temp != 0xd9) | (temp_last != 0xff) )
  {
      temp_last = temp;
      temp = spi_transfer_8 (SPI_BUS, 0x00);

      printf("%02x", temp);

      if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
      {
          spi_cs_high (CAMERA_CS);
          break;
      }
  }
  spi_cs_high (CAMERA_CS);

  return jerry_create_undefined ();
}

DELCARE_HANDLER (arducam_store)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (ARDUCAM_OBJECT_NAME, ARDUCAM_STORE, "1");
  }
  if (!jerry_value_is_object (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_OBJECT);
  }
  jerry_value_t fd = args_p[0];

  // Get the SD object and SD.write.
  jerry_value_t global_object = jerry_get_global_object ();
  jerry_value_t sd_name = jerry_create_string ((const jerry_char_t *) SD_CARD_OBJECT_NAME);
  jerry_value_t sd_object = jerry_get_property (global_object, sd_name);
  jerry_release_value (global_object);
  jerry_release_value (sd_name);

  jerry_value_t write_name = jerry_create_string ((const jerry_char_t *) SD_CARD_WRITE);
  jerry_value_t sd_write_fn = jerry_get_property (sd_object, write_name);
  jerry_release_value (write_name);

  // Get a few properties for writing.
  jerry_value_t as_binary_name = jerry_create_string ((const jerry_char_t *) SD_CARD_AS_BINARY);
  jerry_value_t as_binary = jerry_get_property (sd_object, as_binary_name);
  jerry_release_value (as_binary_name);

  jerry_value_t number_arducam_buff_size = jerry_create_number (ARDUCAM_BUFF_SIZE);

  uint8_t temp, temp_last;
  temp = temp_last = 0;
  uint8_t image_pos = 0;
  jerry_value_t buf = jerry_create_typedarray (JERRY_TYPEDARRAY_UINT8, ARDUCAM_BUFF_SIZE);

  spi_cs_low (CAMERA_CS);
  spi_transfer_8 (SPI_BUS, REG_FIFO_BURST_READ);

  while ((temp != 0xd9) | (temp_last != 0xff)) // Is this condition correct?
  {
    temp_last = temp;
    temp = spi_read_byte ();
    // temp = read_reg (CAMERA_CS, REG_FIFO_SINGLE_READ);
    // printf ("%#x\n", temp);

    jerry_value_t temp_as_number = jerry_create_number (temp);
    jerry_set_property_by_index (buf, image_pos++ % ARDUCAM_BUFF_SIZE, temp_as_number); // TODO handle the return of this
    jerry_release_value (temp_as_number);

    if (image_pos % ARDUCAM_BUFF_SIZE == 0)
    {
      spi_cs_high (CAMERA_CS);

      jerry_value_t write_args[4] = {
        fd,
        buf,
        as_binary,
        number_arducam_buff_size
      };

      printf("writing %d bytes to SD card\n", ARDUCAM_BUFF_SIZE);

      spi_cs_low (SD_CS);
      jerry_call_function (sd_write_fn, sd_object, write_args, 4); // TODO handle the result of this
      spi_cs_high (SD_CS);

      spi_cs_low (CAMERA_CS);
      spi_transfer_8 (SPI_BUS, REG_FIFO_BURST_READ);
    }
  }

  spi_cs_high (CAMERA_CS);

  if (image_pos % ARDUCAM_BUFF_SIZE != 0)
  {
    jerry_value_t buf_size = jerry_create_number (image_pos % ARDUCAM_BUFF_SIZE);
    jerry_value_t write_args[4] = {
      fd,
      buf,
      as_binary,
      buf_size
    };

    printf("writing %d bytes to SD card\n", ARDUCAM_BUFF_SIZE);

    spi_cs_low (SD_CS);
    jerry_value_t write_res = jerry_call_function (sd_write_fn, sd_object, write_args, 4); // TODO handle the return of this
    spi_cs_high (SD_CS);

    jerry_release_value (write_res);
    jerry_release_value (buf_size);
  }
  jerry_release_value (buf);
  jerry_release_value (number_arducam_buff_size);
  jerry_release_value (as_binary);
  jerry_release_value (sd_write_fn);

  jerry_release_value (sd_object);

  printf ("Save done\n");
  return jerry_create_boolean (true);
}

void register_arducam_object (jerry_value_t global_object)
{
  jerry_value_t arducam_object = jerry_create_object ();
  register_js_value_to_object (ARDUCAM_OBJECT_NAME, arducam_object, global_object);

  register_native_function (ARDUCAM_INIT, arducam_init_handler, arducam_object);
  register_native_function (ARDUCAM_CAPTURE, arducam_capture_handler, arducam_object);
  register_native_function (ARDUCAM_STORE, arducam_store_handler, arducam_object);
  register_native_function (ARDUCAM_SEND, arducam_send_handler, arducam_object);
  register_native_function (ARDUCAM_PRINT, arducam_print_handler, arducam_object);

  jerry_value_t sd_cs_val = jerry_create_number (SD_CS);
  register_js_value_to_object (ARDUCAM_SD_CS, sd_cs_val, arducam_object);
  jerry_release_value (sd_cs_val);

  jerry_release_value (arducam_object);
}
