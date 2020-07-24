#include "jerry_extapi.h"
#include "user_camera_live.h"

static uint8_t *image_buffer_start;
static uint8_t *image_buffer;
static uint32_t image_size = 0;
static uint32_t buf_size = ARDUCAM_BUFF_SIZE;

// // Original
// bool send_picture (netconn_t conn)
// {
//   // Get FIFO size and allocate buffer.
//   image_size = read_fifo_length ();
//   send_image_size (image_size, conn);
//   image_buffer_start = (uint8_t*) malloc (buf_size * sizeof (uint8_t)); // Extra byte for package type info
//   if (image_buffer_start == NULL)
//   {
//     printf ("could not allocate image buffer\n");
//     return false;
//   }
//   image_buffer = image_buffer_start + 1;

//   printf ("send_picture: image_size: %d\n", image_size);
//   printf ("send_picture: image_buffer_start: %#x\n", (uint32_t) image_buffer_start);

//   uint32_t image_pos;
//   uint8_t temp, temp_last;
//   image_pos = temp = 0;

//   send_image_fragment_flag (conn); // Sending image data begins.

//   spi_cs_low (CAMERA_CS);
//   // while ((temp != 0xd9) | (temp_last != 0xff))
//   while (image_pos < image_size)
//   {
//     temp_last = temp;
//     temp = read_reg (CAMERA_CS, REG_FIFO_SINGLE_READ);
//     printf("%x", temp);
//     image_buffer[(image_pos++) % buf_size] = temp;

//     if (image_pos % buf_size == 0)
//     {
//       printf ("send_picture: image_pos: %d\n", image_pos);
//       vTaskDelay(100 / portTICK_PERIOD_MS);
//       if (!send_image_fragment (image_buffer_start, buf_size, conn))
//       {
//         printf ("could not send image fragment\n");
//         send_close_connection (conn, MSG_TYPE_CLOSE_CONN);
//         close_connection (conn);
//         return false;
//       }
//     }
//   }
//   spi_cs_high (CAMERA_CS);

//   printf("send_picture: image_pos: %d, buf_size: %d\n", image_pos, buf_size);
//   printf("send_picture: sending %d bytes\n", image_pos % buf_size);
//   if (image_pos % buf_size != 0)
//   {
//     if (!send_image_fragment (image_buffer_start, image_pos % buf_size, conn))
//     {
//       printf ("could not send image fragment\n");
//       send_close_connection (conn, MSG_TYPE_CLOSE_CONN);
//       close_connection (conn);
//       return false;
//     }
//   }

//   free (image_buffer_start);
//   vTaskDelay(1000 / portTICK_PERIOD_MS);
//   if (!send_close_connection (conn, MSG_TYPE_CLOSE_CONN))
//   {
//     printf ("could not send close connection message\n");
//   }
//   close_connection (conn);

//   return true;
// }

// Workaround
bool send_picture (netconn_t conn)
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

  printf ("send_picture: image_size: %d\n", image_size);
  printf ("send_picture: image_buffer_start: %#x\n", (uint32_t) image_buffer_start);

  uint32_t image_pos;
  uint8_t temp, temp_last;
  image_pos = temp = 0;

  spi_cs_low (CAMERA_CS);
  while ((temp != 0xd9) | (temp_last != 0xff))
  {
    temp_last = temp;
    temp = read_reg (CAMERA_CS, REG_FIFO_SINGLE_READ);
    printf("%x", temp);
    image_buffer[(image_pos++) % buf_size] = temp;

    if (image_pos % buf_size == 0)
    {
      // printf ("send_picture: image_pos: %d\n", image_pos);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      if (!send_image_buffer (image_buffer_start, buf_size, conn))
      {
        printf ("could not send image fragment\n");
        send_close_connection (conn, MSG_TYPE_CLOSE_CONN);
        close_connection (conn);
        return false;
      }
    }
  }
  spi_cs_high (CAMERA_CS);

  printf("send_picture: image_pos: %d, buf_size: %d\n", image_pos, buf_size);
  printf("send_picture: sending %d bytes\n", image_pos % buf_size);
  if (image_pos % buf_size != 0)
  {
    if (!send_image_buffer (image_buffer_start, image_pos % buf_size, conn))
    {
      printf ("could not send image fragment\n");
      send_close_connection (conn, MSG_TYPE_CLOSE_CONN);
      close_connection (conn);
      return false;
    }
  }

  free (image_buffer_start);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  if (!send_close_connection (conn, MSG_TYPE_CLOSE_CONN))
  {
    printf ("could not send close connection message\n");
  }
  close_connection (conn);

  return true;
}
