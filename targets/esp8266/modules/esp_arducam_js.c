#include "esp_arducam_js.h"

#define MAX_FIFO_SIZE 0x7ffff // 8M

static netconn_t conn;
static char err_msg[255];

extern const struct sensor_reg ov5642_RAW[];
extern const struct sensor_reg OV5642_1280x960_RAW[];
extern const struct sensor_reg OV5642_1920x1080_RAW[];
extern const struct sensor_reg OV5642_640x480_RAW[];
extern const struct sensor_reg ov5642_320x240[];
extern const struct sensor_reg ov5642_640x480[];
extern const struct sensor_reg ov5642_1280x960[];
extern const struct sensor_reg ov5642_1600x1200[];
extern const struct sensor_reg ov5642_1024x768[];
extern const struct sensor_reg ov5642_2048x1536[];
extern const struct sensor_reg ov5642_2592x1944[];
extern const struct sensor_reg ov5642_dvp_zoom8[];
extern const struct sensor_reg OV5642_QVGA_Preview[];
extern const struct sensor_reg OV5642_JPEG_Capture_QSXGA[];
extern const struct sensor_reg OV5642_1080P_Video_setting[];
extern const struct sensor_reg OV5642_720P_Video_setting[];

#define pgm_read_byte(x)        (*((char *)x))
#define pgm_read_word(x)        ( ((*((unsigned char *)x + 1)) << 8) + (*((unsigned char *)x)))

void wait (uint32_t timeout)
{
  vTaskDelay(timeout / portTICK_PERIOD_MS);
}

/* SPI FUNCTIONS */

uint8_t read_reg (uint8_t pin, uint8_t address)
{
  spi_cs_low (pin);
  // address is masked with 0b01111111 to make the first bit 0.
  spi_transfer_8 (SPI_BUS, address & 0x7f);
  uint8_t ret = spi_read_byte ();
  // correction for bit rotation from readback
  // ret = (uint8_t)(ret >> 1) | (ret << 7);
  spi_cs_high (pin);
  return ret;
}

void write_reg (uint8_t pin, uint8_t address, uint8_t value)
{
  spi_cs_low (pin);

  spi_transfer_8 (SPI_BUS, address | 0x80);
  spi_transfer_8 (SPI_BUS, value);

  spi_cs_high (pin);
}

uint8_t get_bit (uint8_t pin, uint8_t address, uint8_t bit)
{
  return read_reg (pin, address) & bit;
}

void set_bit (uint8_t pin, uint8_t address, uint8_t bit)
{
  write_reg (pin, address, read_reg (pin, address) | bit);
}

void clear_bit (uint8_t pin, uint8_t address, uint8_t bit)
{
  write_reg (pin, address, read_reg (pin, address) & (~bit));
}

uint32_t read_fifo_length ()
{
  uint8_t len1, len2, len3;
  uint32_t length = 0;
  len1 = read_reg (CAMERA_CS, REG_FIFO_SIZE1);
  len2 = read_reg (CAMERA_CS, REG_FIFO_SIZE2);
  len3 = read_reg (CAMERA_CS, REG_FIFO_SIZE3) & 0x7f;
  length = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
  return length;
}

/* I2C FUNCTIONS */

void write_buff_i2c (uint8_t *buff, uint8_t len, bool stop)
{
  i2c_start (I2C_BUS);
  for (uint8_t i = 0; i < len; i++)
  {
    i2c_write (I2C_BUS, buff[i]);
  }
  if (stop)
  {
    i2c_stop (I2C_BUS);
  }
}

void wr_sensor_reg_16_8 (uint16_t reg_id, uint8_t reg_dat)
{
  // Write 8 bit value to 16 bit register.
  uint8_t buff[4] = {
    I2C_SLAVE_ADDR_WRITE,
    reg_id >> 8,
    reg_id & 0x00ff,
    reg_dat & 0x00ff
  };
  write_buff_i2c (buff, 4, true);
  wait(10);
}

void wr_sensor_regs_16_8 (const struct sensor_reg reglist[])
{
  uint16_t reg_addr;
  uint8_t reg_val;

  const struct sensor_reg *next = reglist;

  printf ("wr_sensor_regs_16_8\n");

  while ((reg_addr != 0xffff) | (reg_val != 0xff))
  {
    reg_addr = pgm_read_word (&(next->reg_addr));
    reg_val = pgm_read_word (&(next->reg_val));
    wr_sensor_reg_16_8 (reg_addr, reg_val);
    next++;
  }
}

uint8_t rd_sensor_reg_16_8 (uint16_t regID)
{
  // Read 8 bit value from 16 bit register.
  uint8_t buff[4] = {
    I2C_SLAVE_ADDR_WRITE,
    regID >> 8,
    regID & 0x00ff,
    I2C_SLAVE_ADDR_READ
  };
  write_buff_i2c (buff, 3, true);
  write_buff_i2c (buff + 3, 1, false);
  uint8_t ret = i2c_read (I2C_BUS, true);
  i2c_stop (I2C_BUS);
  return ret;
}

void init_cam ()
{
  wr_sensor_reg_16_8 (0x3008, 0x80);
  wr_sensor_regs_16_8 (OV5642_QVGA_Preview);
  wait (200);
  wr_sensor_regs_16_8 (OV5642_JPEG_Capture_QSXGA);
  wr_sensor_regs_16_8 (ov5642_320x240);
  wait (100);
  wr_sensor_reg_16_8 (0x3818, 0xa8);
  wr_sensor_reg_16_8 (0x3621, 0x10);
  wr_sensor_reg_16_8 (0x3801, 0xb0);
  wr_sensor_reg_16_8 (0x4407, 0x04);
  wr_sensor_reg_16_8 (0x5888, 0x00);
  wr_sensor_reg_16_8 (0x5000, 0xFF);
}

void set_image_size (enum image_size size)
{
  switch (size)
  {
    case OV5642_320x240:
      wr_sensor_regs_16_8 (ov5642_320x240);
      break;
    case OV5642_640x480:
      wr_sensor_regs_16_8 (ov5642_640x480);
      break;
    case OV5642_1024x768:
      wr_sensor_regs_16_8 (ov5642_1024x768);
      break;
    case OV5642_1280x960:
      wr_sensor_regs_16_8 (ov5642_1280x960);
      break;
    case OV5642_1600x1200:
      wr_sensor_regs_16_8 (ov5642_1600x1200);
      break;
    case OV5642_2048x1536:
      wr_sensor_regs_16_8 (ov5642_2048x1536);
      break;
    case OV5642_2592x1944:
      wr_sensor_regs_16_8 (ov5642_2592x1944);
      break;
    default:
      wr_sensor_regs_16_8 (ov5642_320x240);
      break;
  }
}

DELCARE_HANDLER(arducam_init)
{
  /* --------------------- */
  /* INIT */
  wait (5000);
  const spi_settings_t settings = {
    .mode = SPI_MODE0,
    .freq_divider = SPI_FREQ_DIV_8M,
    .msb = true,
    .endianness = SPI_BIG_ENDIAN,
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

#define RELEASE_VALUES() do { \
    jerry_release_value (buf); \
    jerry_release_value (number_arducam_buff_size); \
    jerry_release_value (as_binary); \
    jerry_release_value (sd_write_fn); \
    jerry_release_value (sd_object); \
  } while (0)

#define DO_WRITE() do { \
    jerry_value_t sd_write_val = jerry_call_function (sd_write_fn, sd_object, write_args, 4); \
    if (jerry_value_is_error (sd_write_val)) \
    { \
      RELEASE_VALUES (); \
      return sd_write_val; \
    } \
    else if (jerry_value_is_boolean (sd_write_val)) \
    { \
      bool raw_val = jerry_get_boolean_value (sd_write_val); \
      if (!raw_val) \
      { \
        RELEASE_VALUES (); \
        return sd_write_val; \
      } \
    } \
    jerry_release_value (sd_write_val); \
  } while (0)

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

  while ((temp != 0xd9) | (temp_last != 0xff))
  {
    temp_last = temp;
    temp = spi_read_byte ();

    jerry_value_t temp_as_number = jerry_create_number (temp);
    jerry_value_t set_index_val = jerry_set_property_by_index (buf, image_pos++ % ARDUCAM_BUFF_SIZE, temp_as_number);
    if (jerry_value_is_error (set_index_val))
    {
      return set_index_val;
    }
    jerry_release_value (set_index_val);
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
      DO_WRITE ();
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
    DO_WRITE();
    spi_cs_high (SD_CS);

    jerry_release_value (buf_size);
  }
  RELEASE_VALUES();

  printf ("Save done\n");
  return jerry_create_boolean (true);
}

#undef DO_WRITE
#undef RELEASE_VALUES

void register_arducam_object (jerry_value_t global_object)
{
  jerry_value_t arducam_object = jerry_create_object ();
  register_js_value_to_object (ARDUCAM_OBJECT_NAME, arducam_object, global_object);

  register_native_function (ARDUCAM_INIT, arducam_init_handler, arducam_object);
  register_native_function (ARDUCAM_CAPTURE, arducam_capture_handler, arducam_object);
  register_native_function (ARDUCAM_STORE, arducam_store_handler, arducam_object);
  register_native_function (ARDUCAM_PRINT, arducam_print_handler, arducam_object);

  jerry_value_t sd_cs_val = jerry_create_number (SD_CS);
  register_js_value_to_object (ARDUCAM_SD_CS, sd_cs_val, arducam_object);
  jerry_release_value (sd_cs_val);

  jerry_release_value (arducam_object);
}
