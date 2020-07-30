#include "jerry_extapi.h"
#include "user_camera_live.h"

static uint8_t *image_buffer_start;
static uint8_t *image_buffer;
static uint32_t image_size = 0;
static uint32_t buf_size = ARDUCAM_BUFF_SIZE;

static netconn_t conn;

bool send_picture ()
{
  // Get FIFO size and allocate buffer.
  image_size = read_fifo_length ();
  image_buffer_start = (uint8_t*) malloc (buf_size * sizeof (uint8_t)); // Extra byte for package type info
  if (image_buffer_start == NULL)
  {
    printf ("could not allocate image buffer\n");
    return false;
  }
  image_buffer = image_buffer_start + 1;

  uint32_t image_pos;
  uint8_t temp, temp_last;
  image_pos = temp = 0;

  spi_cs_low (CAMERA_CS);

  bool ret = true;
  while ((temp != 0xd9) | (temp_last != 0xff))
  {
    temp_last = temp;
    temp = read_reg (CAMERA_CS, REG_FIFO_SINGLE_READ);
    image_buffer[(image_pos++) % buf_size] = temp;

    if (image_pos % buf_size == 0)
    {
      printf ("send_picture: image_pos: %d\n", image_pos);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      if (!send_image_buffer (image_buffer_start, buf_size, conn))
      {
        printf ("could not send image fragment (main loop)\n");
        ret = false;
        break;
      }
    }
  }
  spi_cs_high (CAMERA_CS);

  if (ret && image_pos % buf_size != 0) // If we already have to return false, we can skip this.
  {
    if (!send_image_buffer (image_buffer_start, image_pos % buf_size, conn))
    {
      printf ("could not send image fragment (second loop)\n");
      ret = false;
    }
  }

  free (image_buffer_start);
  vTaskDelay (1000 / portTICK_PERIOD_MS);
  if (!send_close_connection (conn, MSG_TYPE_CLOSE_CONN))
  {
    printf ("could not send close connection message\n");
  }
  close_connection (conn);

  return ret;
}

bool user_arducam_live(uint32_t timeout)
{
  bool ret = true;
  if (initialize_connection (&conn))
  {
    if (_arducam_init () == ARDUCAM_INIT_OK)
    {
      int start = (int)(sdk_system_get_time() / 1000000);
      while (true)
      {
        if (_arducam_capture () == ARDUCAM_CAPTURE_OK)
        {
          if (!send_picture ())
          {
            printf ("sending picture failed\n");
            ret = false;
            break;
          }
        }
        else
        {
          printf ("ArduCAM capture failed\n");
          ret = false;
          break;
        }

        int end = (int)(sdk_system_get_time() / 1000000);
        if (end - start > timeout)
        {
          printf ("ArduCAM live timeout\n");
          break;
        }
      }
    }
    else
    {
      printf ("ArduCAM init failed\n");
      ret = false;
    }
  }
  else
  {
    printf ("Initializating connection failed\n");
    ret = false;
  }

  close_connection (conn);
  return ret;
}
