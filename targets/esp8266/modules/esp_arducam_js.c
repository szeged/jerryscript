#include "esp_arducam_js.h"

#define BUS     1
#define CS_PIN 15

#define spi_cs_low()          do { gpio_write (CS_PIN, false); } while (0)
#define spi_cs_high()         do { gpio_write (CS_PIN, true); } while (0)
#define spi_read_byte()       (spi_transfer_8 (BUS, 0x00))

// Misc helper
const char *error_spi_interface = "SPI interface error";

uint8_t read_reg (uint8_t address)
{
  spi_cs_low ();
  // address is masked with 0b01111111 to make the first bit 0.
  spi_transfer_8 (BUS, address & 0x7f);
  uint8_t ret = spi_read_byte ();
  // correction for bit rotation from readback
  ret = (uint8_t)(ret >> 1) | (ret << 7);
  spi_cs_high ();
  return ret;
}

void write_reg (uint8_t address, uint8_t value)
{
  spi_cs_low ();

  spi_transfer_8 (BUS, address | 0x80);
  spi_transfer_8 (BUS, value);

  spi_cs_high ();
}

DELCARE_HANDLER (arducam_init)
{
  const spi_settings_t settings = {
    .mode = SPI_MODE1,
    .freq_divider = SPI_FREQ_DIV_4M,
    .msb = true, // ???
    .endianness = SPI_LITTLE_ENDIAN, // ???
    .minimal_pins = true
  };

  gpio_enable (CS_PIN, GPIO_OUTPUT);
  bool ret = spi_set_settings (BUS, &settings);

  // Don't ask why this is needed, it works.
  spi_cs_low ();
  spi_cs_high ();
  for (uint8_t i = 0; i < 10; i++)
  {
    spi_read_byte ();
  }

  write_reg (REG_TEST, 0x55);
  uint8_t value = read_reg (REG_TEST);
  printf ("%#x\n", value);
  if (value != 0x55)
  {
    printf ("SPI interface error\n");
    // TODO ez szar
    // return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t *) error_spi_interface);
  }

  return jerry_create_boolean (ret);
}

DELCARE_HANDLER (arducam_test)
{
  write_reg (REG_TEST, 0x55);
  return jerry_create_number (read_reg (REG_TEST));
}

DELCARE_HANDLER (arducam_capture)
{
  // Clear fifo flag.
  write_reg (REG_FIFO_CONTROL, MASK_CLEAR_FIFO);
  // Set the picture number to 1.
  write_reg (REG_CAPTURE_CONTROL, 0x01);
  // Start capture.
  write_reg (REG_FIFO_CONTROL, MASK_START_CAPTURE);
  // Wait until capture is complete.
  return jerry_create_undefined ();
}

void register_arducam_object (jerry_value_t global_object)
{
  jerry_value_t arducam_object = jerry_create_object ();
  register_js_value_to_object (ARDUCAM_OBJECT_NAME, arducam_object, global_object);

  register_native_function (ARDUCAM_INIT, arducam_init_handler, arducam_object);
  register_native_function (ARDUCAM_CAPTURE, arducam_capture_handler, arducam_object);
  register_native_function ("test", arducam_test_handler, arducam_object);

  jerry_release_value (arducam_object);
}
