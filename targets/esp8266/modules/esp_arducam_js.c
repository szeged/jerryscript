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

DELCARE_HANDLER(arducam_main)
{
  /* --------------------- */
  /* CAPTURE */
  write_reg (CAMERA_CS, REG_TIMING_CONTROL, MASK_VSYNC_LEVEL);
  set_image_size (OV5642_640x480);
  // Clear fifo flag.
  write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  // Start capture.
  write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_START_CAPTURE);
  // Wait until capture is complete.
  while (!get_bit (CAMERA_CS, REG_TRIG, MASK_CAPTURE_DONE));

  // Discard the first image.
  // Clear fifo flag.
  write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  // Start capture.
  write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_START_CAPTURE);
  // Wait until capture is complete.
  while (!get_bit (CAMERA_CS, REG_TRIG, MASK_CAPTURE_DONE));

  printf ("Capture done!\n");

  /* --------------------- */
  /* SEND */
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

  // uint8_t temp, temp_last;
  // temp = temp_last = 0;

  // spi_cs_low (CAMERA_CS);
  // spi_transfer_8 (SPI_BUS, REG_FIFO_BURST_READ);

  // while ( image_size-- )
  // {
  //     temp_last = temp;
  //     temp = spi_transfer_8 (SPI_BUS, 0x00);

  //     printf("%02x", temp);

  //     if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
  //     {
  //         spi_cs_high (CAMERA_CS);
  //         break;
  //     }
  // }
  // spi_cs_high (CAMERA_CS);

  send_picture (conn);

  return jerry_create_undefined ();
}

void register_arducam_object (jerry_value_t global_object)
{
  jerry_value_t arducam_object = jerry_create_object ();
  register_js_value_to_object (ARDUCAM_OBJECT_NAME, arducam_object, global_object);

  register_native_function (ARDUCAM_INIT, arducam_init_handler, arducam_object);
  register_native_function (ARDUCAM_MAIN, arducam_main_handler, arducam_object);

  jerry_release_value (arducam_object);
}
