#include "esp_arducam_js.h"

static netconn_t conn;

void initialize_conn()
{
 if (!initialize_connection_wrapper (&conn)) {
    printf("Server connection has failed\n");
  }
}

#define MAX_FIFO_SIZE 0x7ffff

DELCARE_HANDLER(arducam_main)
{
  /* --------------------- */
  /* INIT */
  wait(5000 * MS);
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
    char err_msg[30];
    sprintf (err_msg, "SPI init failed, code: %d", spi_succ);
    // return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) err_msg);
  }
  // gpio_enable (SD_CS, GPIO_OUTPUT);
  // spi_cs_high (SD_CS);

  i2c_set_clock_stretch (I2C_BUS, 10);
  int i2c_succ = i2c_init (I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ_100K);
  if (i2c_succ)
  {
    char err_msg[30];
    sprintf (err_msg, "I2C init failed, code: %d", i2c_succ);
    // return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) err_msg);
  }

  write_reg (CAMERA_CS, 0x07, 0x80);
  wait (100 * MS);
  write_reg (CAMERA_CS, 0x07, 0x00);
  wait (100 * MS);

  // Check that SPI and I2C interface works correctly.
  write_reg (CAMERA_CS, REG_TEST, 0x55);
  uint8_t test = read_reg (CAMERA_CS, REG_TEST);
  printf ("%#x\n", test);
  if (test != 0x55)
  {
    char err_msg[50];
    sprintf (err_msg, "SPI interface error, expected: 0x55, received: %#x", test);
    // return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) err_msg);
  }

  wr_sensor_reg_16_8 (0xff, 0x01);
  uint8_t vid = rd_sensor_reg_16_8 (OV5642_CHIPID_HIGH);
  uint8_t pid = rd_sensor_reg_16_8 (OV5642_CHIPID_LOW);
  printf ("vid: %#x\n", vid);
  printf ("pid: %#x\n", pid);
  if (!(vid == 0x56 && pid == 0x42))
  {
    // return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) "I2C interface error");
  }

  init_cam ();

  /* --------------------- */
  /* CAPTURE */
  // set_bit (CAMERA_CS, REG_TIMING_CONTROL, MASK_VSYNC_LEVEL);
  write_reg (CAMERA_CS, REG_TIMING_CONTROL, MASK_VSYNC_LEVEL);
  set_image_size (OV5642_640x480);
  // Clear fifo flag.
  write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
  // Set the picture number to 1.
  // write_reg (CAMERA_CS, REG_CAPTURE_CONTROL, 1);
  // Start capture.
  write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_START_CAPTURE);
  // Wait until capture is complete.
  while (!get_bit (CAMERA_CS, REG_TRIG, MASK_CAPTURE_DONE)){/*wait(1 * MS);*/}

  printf ("Capture done!\n");

  /* --------------------- */
  /* SEND */
  uint32_t imageSize = read_fifo_length ();
  printf("arducam_print %d\n", imageSize);
  uint8_t temp, temp_last;
  temp = temp_last = 0;

  if(imageSize >= 0x7FFFF) //8M
  {
      printf("Over size.\n");
  }
  if(imageSize == 0) //0 kb
  {
      printf("Size is 0.\n");
  }

  spi_cs_low (CAMERA_CS);
  spi_transfer_8 (SPI_BUS, REG_FIFO_BURST_READ);

  while ( imageSize-- )
  {
      temp_last = temp;
      temp = spi_transfer_8 (SPI_BUS, 0x00);

      printf("%02x", temp);
      // taskYIELD();

      if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
      {
          spi_cs_high (CAMERA_CS);
          break;
      }
  }

  spi_cs_high (CAMERA_CS);
  printf ("\n");

  while(1)
  {
    // start_capture();
    write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_CLEAR_CAPTURE_DONE_FLAG);
    write_reg (CAMERA_CS, REG_FIFO_CONTROL, MASK_START_CAPTURE);
    while (!get_bit(CAMERA_CS, REG_TRIG, MASK_CAPTURE_DONE));

    imageSize = read_fifo_length();
    printf("\nlen: %d\n", imageSize);

    if (imageSize >= 0x7FFFF) //8M
    {
        printf("Over size.\n");
        continue;
    }
    if (imageSize == 0 ) //0 kb
    {
        printf("Size is 0.\n");
        continue;
    }

    printf("BEGIN\n");

    spi_cs_low (CAMERA_CS);
    spi_transfer_8 (SPI_BUS, REG_FIFO_BURST_READ);
    while (imageSize--)
    {
        temp_last = temp;
        temp =  spi_transfer_8 (SPI_BUS, 0x00);
        if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
        {
            spi_cs_high (CAMERA_CS);
            printf("%02x", temp);
            printf("\nEND\n");
            break;
        }
        printf("%02x", temp);
    }
    spi_cs_high (CAMERA_CS);
  }

  // if (!initialize_connection_wrapper (&conn)) {
  //   printf("Server connection has failed\n");
  // }

  // sendPicture (conn);

  return jerry_create_undefined ();
}

void register_arducam_object (jerry_value_t global_object)
{
  jerry_value_t arducam_object = jerry_create_object ();
  register_js_value_to_object (ARDUCAM_OBJECT_NAME, arducam_object, global_object);

  // register_native_function (ARDUCAM_INIT, arducam_init_handler, arducam_object);
  // register_native_function (ARDUCAM_CAPTURE, arducam_capture_handler, arducam_object);
  // register_native_function (ARDUCAM_STORE, arducam_store_handler, arducam_object);
  // register_native_function (ARDUCAM_SEND, arducam_send_handler, arducam_object);
  // register_native_function ("test_spi", test_spi_handler, arducam_object);
  // register_native_function ("test_i2c", test_i2c_handler, arducam_object);
  register_native_function (ARDUCAM_MAIN, arducam_main_handler, arducam_object);

  jerry_release_value (arducam_object);
}
