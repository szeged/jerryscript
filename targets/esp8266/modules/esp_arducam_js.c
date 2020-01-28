#include "esp_arducam_js.h"

DELCARE_HANDLER (arducam_init)
{
  wait (5000 * MS);
  const spi_settings_t settings = {
    .mode = SPI_MODE0,
    .freq_divider = SPI_FREQ_DIV_8M,
    .msb = true, // ???
    .endianness = SPI_LITTLE_ENDIAN, // ???
    .minimal_pins = false
  };

  int spi_succ = spi_set_settings (SPI_BUS, &settings);
  if (!spi_succ)
  {
    char err_msg[30];
    sprintf (err_msg, "SPI init failed, code: %d", spi_succ);
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) err_msg);
  }
  gpio_enable (CS_PIN, GPIO_OUTPUT);

  printf ("ameno\n");

  i2c_set_clock_stretch (I2C_BUS, 10);
  int i2c_succ = i2c_init (I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ_80K);
  if (i2c_succ)
  {
    char err_msg[30];
    sprintf (err_msg, "I2C init failed, code: %d", i2c_succ);
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) err_msg);
  }
  printf ("dorime\n");

  // Check that SPI and I2C interface works correctly.
  write_reg (REG_TEST, 0x55);
  uint8_t test = read_reg (REG_TEST);
  printf ("%#x\n", test);
  if (test != 0x55)
  {
    char err_msg[50];
    sprintf (err_msg, "SPI interface error, expected: 0x55, received: %#x", test);
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) err_msg);
  }

  write_reg (REG_MODE, MASK_MCU_MODE);
  uint8_t vid = rd_sensor_reg_16_8 (OV5642_CHIPID_HIGH);
  uint8_t pid = rd_sensor_reg_16_8 (OV5642_CHIPID_LOW);
  printf ("vid: %#x\n", vid);
  printf ("pid: %#x\n", pid);
  if (!(vid == 0x56 && pid == 0x42))
  {
    return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "I2C interface error");
  }

  init_cam ();
  set_image_size (OV5642_320x240);

  printf ("latire\n");

  return jerry_create_boolean (true);
}

DELCARE_HANDLER (arducam_capture)
{
  write_reg (REG_TIMING_CONTROL, MASK_VSYNC_LEVEL);
  // Clear fifo flag.
  write_reg (REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  write_reg (REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  // Set the picture number to 1.
  write_reg (REG_CAPTURE_CONTROL, 1);
  // Start capture.
  write_reg (REG_FIFO_CONTROL, MASK_START_CAPTURE);
  // Wait until capture is complete.
  while (!get_bit (REG_TRIG, MASK_CAPTURE_DONE)){}

  printf ("Capture done!\n");
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
  jerry_value_t number_256 = jerry_create_number (256);

  uint8_t temp, temp_last;
  temp = temp_last = 0;
  uint8_t i = 0;
  jerry_value_t buf = jerry_create_array (256);

  spi_cs_low ();
  // spi_transfer_8 (SPI_BUS, REG_BURST_FIFO_READ);

  while ((temp != 0xd9) | (temp_last != 0xff)) // Is this condition correct?
  {
    temp_last = temp;
    // temp = spi_read_byte ();
    temp = read_reg (REG_FIFO_SINGLE_READ);
    temp = (uint8_t)(temp >> 1) | (temp << 7); // correction from bit rotation from readback

    jerry_value_t temp_as_number = jerry_create_number (temp);
    jerry_set_property_by_index (buf, i++, temp_as_number); // TODO handle the return of this
    jerry_release_value (temp_as_number);

    if (i == 256)
    {
      // spi_cs_high ();

      jerry_value_t write_args[4] = {
        fd,
        buf,
        as_binary,
        number_256
      };
      jerry_call_function (sd_write_fn, sd_object, write_args, 4); // TODO handle the result of this

      // spi_cs_low ();
      // spi_transfer_8 (SPI_BUS, REG_BURST_FIFO_READ);
      i = 0;
    }
  }

  spi_cs_high ();

  if (i > 0)
  {

    jerry_value_t number_i = jerry_create_number (i);
    jerry_value_t write_args[4] = {
      fd,
      buf,
      as_binary,
      number_i
    };
    jerry_value_t write_res = jerry_call_function (sd_write_fn, sd_object, write_args, 4); // TODO handle the return of this
    jerry_release_value (write_res);
    jerry_release_value (number_i);
  }
  jerry_release_value (buf);
  jerry_release_value (number_256);
  jerry_release_value (as_binary);
  jerry_release_value (sd_write_fn);

  jerry_release_value (sd_object);

  printf ("Save done\n");
  return jerry_create_boolean (true);
}

DELCARE_HANDLER(test_spi)
{
  write_reg (REG_TEST, 0x55);
  return jerry_create_number (read_reg (REG_TEST));
}

DELCARE_HANDLER(test_i2c)
{
  uint8_t high = rd_sensor_reg_16_8 (OV5642_CHIPID_HIGH);
  uint8_t low = rd_sensor_reg_16_8 (OV5642_CHIPID_LOW);
  return jerry_create_number (high << 8 | low);
}

void register_arducam_object (jerry_value_t global_object)
{
  jerry_value_t arducam_object = jerry_create_object ();
  register_js_value_to_object (ARDUCAM_OBJECT_NAME, arducam_object, global_object);

  register_native_function (ARDUCAM_INIT, arducam_init_handler, arducam_object);
  register_native_function (ARDUCAM_CAPTURE, arducam_capture_handler, arducam_object);
  register_native_function (ARDUCAM_STORE, arducam_store_handler, arducam_object);
  register_native_function ("test_spi", test_spi_handler, arducam_object);
  register_native_function ("test_i2c", test_i2c_handler, arducam_object);

  jerry_release_value (arducam_object);
}
