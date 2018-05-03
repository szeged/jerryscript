#include <stdlib.h>  // If there are pointers, malloc() and free() required
#include "esp_i2c_js.h"


// external function for API functions or for getters / setters
jerry_value_t i2c_init_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 4)
  {
    char* msg = "Wrong argument count for i2c_init(), expected 4.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_init(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[1]))
  {
    char* msg = "Wrong argument type for i2c_init(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_1 = jerry_get_number_value (args_p[1]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[2]))
  {
    char* msg = "Wrong argument type for i2c_init(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_2 = jerry_get_number_value (args_p[2]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[3]))
  {
    char* msg = "Wrong argument type for i2c_init(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  i2c_freq_t arg_3 = jerry_get_number_value (args_p[3]);

  // native function call
  int result = i2c_init (arg_0, arg_1, arg_2, arg_3);


  jerry_value_t ret_val = jerry_create_number (result);

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_init_hz_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 4)
  {
    char* msg = "Wrong argument count for i2c_init_hz(), expected 4.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_init_hz(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[1]))
  {
    char* msg = "Wrong argument type for i2c_init_hz(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_1 = jerry_get_number_value (args_p[1]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[2]))
  {
    char* msg = "Wrong argument type for i2c_init_hz(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_2 = jerry_get_number_value (args_p[2]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[3]))
  {
    char* msg = "Wrong argument type for i2c_init_hz(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint32_t arg_3 = jerry_get_number_value (args_p[3]);

  // native function call
  int result = i2c_init_hz (arg_0, arg_1, arg_2, arg_3);


  jerry_value_t ret_val = jerry_create_number (result);

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_set_frequency_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 2)
  {
    char* msg = "Wrong argument count for i2c_set_frequency(), expected 2.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_set_frequency(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[1]))
  {
    char* msg = "Wrong argument type for i2c_set_frequency(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  i2c_freq_t arg_1 = jerry_get_number_value (args_p[1]);

  // native function call
  int result = i2c_set_frequency (arg_0, arg_1);


  jerry_value_t ret_val = jerry_create_number (result);

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_set_frequency_hz_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 2)
  {
    char* msg = "Wrong argument count for i2c_set_frequency_hz(), expected 2.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_set_frequency_hz(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[1]))
  {
    char* msg = "Wrong argument type for i2c_set_frequency_hz(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint32_t arg_1 = jerry_get_number_value (args_p[1]);

  // native function call
  int result = i2c_set_frequency_hz (arg_0, arg_1);


  jerry_value_t ret_val = jerry_create_number (result);

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_set_clock_stretch_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 2)
  {
    char* msg = "Wrong argument count for i2c_set_clock_stretch(), expected 2.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_set_clock_stretch(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[1]))
  {
    char* msg = "Wrong argument type for i2c_set_clock_stretch(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  TickType_t arg_1 = jerry_get_number_value (args_p[1]);

  // native function call
  i2c_set_clock_stretch (arg_0, arg_1);


  jerry_value_t ret_val = jerry_create_undefined ();

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_write_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 2)
  {
    char* msg = "Wrong argument count for i2c_write(), expected 2.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_write(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[1]))
  {
    char* msg = "Wrong argument type for i2c_write(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_1 = jerry_get_number_value (args_p[1]);

  // native function call
  _Bool result = i2c_write (arg_0, arg_1);


  jerry_value_t ret_val = jerry_create_boolean (result);

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_read_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 2)
  {
    char* msg = "Wrong argument count for i2c_read(), expected 2.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_read(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);


  // create a _Bool value from a jerry_value_t
  _Bool arg_1 = jerry_value_to_boolean (args_p[1]);

  // native function call
  unsigned char result = i2c_read (arg_0, arg_1);


  jerry_value_t ret_val = jerry_create_number (result);

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_start_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 1)
  {
    char* msg = "Wrong argument count for i2c_start(), expected 1.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_start(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);

  // native function call
  i2c_start (arg_0);


  jerry_value_t ret_val = jerry_create_undefined ();

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_stop_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 1)
  {
    char* msg = "Wrong argument count for i2c_stop(), expected 1.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_stop(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);

  // native function call
  _Bool result = i2c_stop (arg_0);


  jerry_value_t ret_val = jerry_create_boolean (result);

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_status_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 1)
  {
    char* msg = "Wrong argument count for i2c_status(), expected 1.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_status(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);

  // native function call
  _Bool result = i2c_status (arg_0);


  jerry_value_t ret_val = jerry_create_boolean (result);

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_force_bus_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 2)
  {
    char* msg = "Wrong argument count for i2c_force_bus(), expected 2.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_force_bus(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);


  // create a _Bool value from a jerry_value_t
  _Bool arg_1 = jerry_value_to_boolean (args_p[1]);

  // native function call
  i2c_force_bus (arg_0, arg_1);


  jerry_value_t ret_val = jerry_create_undefined ();

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_slave_write_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 5)
  {
    char* msg = "Wrong argument count for i2c_slave_write(), expected 5.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_slave_write(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[1]))
  {
    char* msg = "Wrong argument type for i2c_slave_write(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_1 = jerry_get_number_value (args_p[1]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_typedarray (args_p[2]) && !jerry_value_is_null (args_p[2]))
  {
    char* msg = "Wrong argument type for i2c_slave_write(), expected typedarray or null.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create a pointer to number from a jerry_value_t
  unsigned char * arg_2 = NULL;
  jerry_length_t arg_2_byteLength = 0;
  jerry_length_t arg_2_byteOffset = 0;
  jerry_value_t arg_2_buffer = jerry_create_undefined();
  if (jerry_value_is_typedarray (args_p[2]))
  {
    arg_2_buffer = jerry_get_typedarray_buffer (args_p[2], &arg_2_byteOffset, &arg_2_byteLength);
    arg_2 = (unsigned char*) malloc (arg_2_byteLength);
    if(arg_2 == NULL)
    {
      jerry_release_value (arg_2_buffer);
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t*)"Fail to allocate memory.");
    }
    jerry_arraybuffer_read (arg_2_buffer, arg_2_byteOffset, (uint8_t*)arg_2, arg_2_byteLength);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_typedarray (args_p[3]) && !jerry_value_is_null (args_p[3]))
  {
    char* msg = "Wrong argument type for i2c_slave_write(), expected typedarray or null.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create a pointer to number from a jerry_value_t
  unsigned char * arg_3 = NULL;
  jerry_length_t arg_3_byteLength = 0;
  jerry_length_t arg_3_byteOffset = 0;
  jerry_value_t arg_3_buffer = jerry_create_undefined();
  if (jerry_value_is_typedarray (args_p[3]))
  {
    arg_3_buffer = jerry_get_typedarray_buffer (args_p[3], &arg_3_byteOffset, &arg_3_byteLength);
    arg_3 = (unsigned char*) malloc (arg_3_byteLength);
    if(arg_3 == NULL)
    {
      jerry_release_value (arg_3_buffer);
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t*)"Fail to allocate memory.");
    }
    jerry_arraybuffer_read (arg_3_buffer, arg_3_byteOffset, (uint8_t*)arg_3, arg_3_byteLength);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[4]))
  {
    char* msg = "Wrong argument type for i2c_slave_write(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint32_t arg_4 = jerry_get_number_value (args_p[4]);

  // native function call
  int result = i2c_slave_write (arg_0, arg_1, arg_2, arg_3, arg_4);


  // write the values back into an arraybuffer from a pointer
  if (jerry_value_is_typedarray (args_p[2]))
  {
    jerry_arraybuffer_write (arg_2_buffer, arg_2_byteOffset, (uint8_t*)arg_2, arg_2_byteLength);
    jerry_release_value (arg_2_buffer);
    // TODO: if you won't use arg_2 pointer, uncomment the line below
    //free (arg_2);
  }


  // write the values back into an arraybuffer from a pointer
  if (jerry_value_is_typedarray (args_p[3]))
  {
    jerry_arraybuffer_write (arg_3_buffer, arg_3_byteOffset, (uint8_t*)arg_3, arg_3_byteLength);
    jerry_release_value (arg_3_buffer);
    // TODO: if you won't use arg_3 pointer, uncomment the line below
    //free (arg_3);
  }


  jerry_value_t ret_val = jerry_create_number (result);

  return ret_val;
}


// external function for API functions or for getters / setters
jerry_value_t i2c_slave_read_handler (const jerry_value_t function_obj,
                      const jerry_value_t this_val,
                      const jerry_value_t args_p[],
                      const jerry_length_t args_cnt)
{

  // check the count of the external function's arguments
  if (args_cnt != 5)
  {
    char* msg = "Wrong argument count for i2c_slave_read(), expected 5.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[0]))
  {
    char* msg = "Wrong argument type for i2c_slave_read(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_0 = jerry_get_number_value (args_p[0]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[1]))
  {
    char* msg = "Wrong argument type for i2c_slave_read(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint8_t arg_1 = jerry_get_number_value (args_p[1]);


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_typedarray (args_p[2]) && !jerry_value_is_null (args_p[2]))
  {
    char* msg = "Wrong argument type for i2c_slave_read(), expected typedarray or null.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create a pointer to number from a jerry_value_t
  unsigned char * arg_2 = NULL;
  jerry_length_t arg_2_byteLength = 0;
  jerry_length_t arg_2_byteOffset = 0;
  jerry_value_t arg_2_buffer = jerry_create_undefined();
  if (jerry_value_is_typedarray (args_p[2]))
  {
    arg_2_buffer = jerry_get_typedarray_buffer (args_p[2], &arg_2_byteOffset, &arg_2_byteLength);
    arg_2 = (unsigned char*) malloc (arg_2_byteLength);
    if(arg_2 == NULL)
    {
      jerry_release_value (arg_2_buffer);
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t*)"Fail to allocate memory.");
    }
    jerry_arraybuffer_read (arg_2_buffer, arg_2_byteOffset, (uint8_t*)arg_2, arg_2_byteLength);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_typedarray (args_p[3]) && !jerry_value_is_null (args_p[3]))
  {
    char* msg = "Wrong argument type for i2c_slave_read(), expected typedarray or null.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create a pointer to number from a jerry_value_t
  unsigned char * arg_3 = NULL;
  jerry_length_t arg_3_byteLength = 0;
  jerry_length_t arg_3_byteOffset = 0;
  jerry_value_t arg_3_buffer = jerry_create_undefined();
  if (jerry_value_is_typedarray (args_p[3]))
  {
    arg_3_buffer = jerry_get_typedarray_buffer (args_p[3], &arg_3_byteOffset, &arg_3_byteLength);
    arg_3 = (unsigned char*) malloc (arg_3_byteLength);
    if(arg_3 == NULL)
    {
      jerry_release_value (arg_3_buffer);
      return jerry_create_error (JERRY_ERROR_COMMON, (const jerry_char_t*)"Fail to allocate memory.");
    }
    jerry_arraybuffer_read (arg_3_buffer, arg_3_byteOffset, (uint8_t*)arg_3, arg_3_byteLength);
  }


  // check the type of a jerry_value_t variable
  if (!jerry_value_is_number (args_p[4]))
  {
    char* msg = "Wrong argument type for i2c_slave_read(), expected number.";
    return jerry_create_error (JERRY_ERROR_TYPE, (const jerry_char_t*)msg);
  }

  // create an integer / floating point number from a jerry_value_t
  uint32_t arg_4 = jerry_get_number_value (args_p[4]);

  // native function call
  int result = i2c_slave_read (arg_0, arg_1, arg_2, arg_3, arg_4);


  // write the values back into an arraybuffer from a pointer
  if (jerry_value_is_typedarray (args_p[2]))
  {
    jerry_arraybuffer_write (arg_2_buffer, arg_2_byteOffset, (uint8_t*)arg_2, arg_2_byteLength);
    jerry_release_value (arg_2_buffer);
    // TODO: if you won't use arg_2 pointer, uncomment the line below
    //free (arg_2);
  }


  // write the values back into an arraybuffer from a pointer
  if (jerry_value_is_typedarray (args_p[3]))
  {
    jerry_arraybuffer_write (arg_3_buffer, arg_3_byteOffset, (uint8_t*)arg_3, arg_3_byteLength);
    jerry_release_value (arg_3_buffer);
    // TODO: if you won't use arg_3 pointer, uncomment the line below
    //free (arg_3);
  }


  jerry_value_t ret_val = jerry_create_number (result);

  return ret_val;
}


void register_i2c_object (jerry_value_t global_object)
{
  jerry_value_t object = jerry_create_object();
  register_js_value_to_object (I2C_OBJECT_NAME, object, global_object);

  // set an external function as a property to the module object
  jerry_value_t i2c_init_name = jerry_create_string ((const jerry_char_t*)"i2c_init");
  jerry_value_t i2c_init_func = jerry_create_external_function (i2c_init_handler);
  jerry_value_t i2c_init_ret = jerry_set_property (object, i2c_init_name, i2c_init_func);
  jerry_release_value (i2c_init_name);
  jerry_release_value (i2c_init_func);
  jerry_release_value (i2c_init_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_init_hz_name = jerry_create_string ((const jerry_char_t*)"i2c_init_hz");
  jerry_value_t i2c_init_hz_func = jerry_create_external_function (i2c_init_hz_handler);
  jerry_value_t i2c_init_hz_ret = jerry_set_property (object, i2c_init_hz_name, i2c_init_hz_func);
  jerry_release_value (i2c_init_hz_name);
  jerry_release_value (i2c_init_hz_func);
  jerry_release_value (i2c_init_hz_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_set_frequency_name = jerry_create_string ((const jerry_char_t*)"i2c_set_frequency");
  jerry_value_t i2c_set_frequency_func = jerry_create_external_function (i2c_set_frequency_handler);
  jerry_value_t i2c_set_frequency_ret = jerry_set_property (object, i2c_set_frequency_name, i2c_set_frequency_func);
  jerry_release_value (i2c_set_frequency_name);
  jerry_release_value (i2c_set_frequency_func);
  jerry_release_value (i2c_set_frequency_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_set_frequency_hz_name = jerry_create_string ((const jerry_char_t*)"i2c_set_frequency_hz");
  jerry_value_t i2c_set_frequency_hz_func = jerry_create_external_function (i2c_set_frequency_hz_handler);
  jerry_value_t i2c_set_frequency_hz_ret = jerry_set_property (object, i2c_set_frequency_hz_name, i2c_set_frequency_hz_func);
  jerry_release_value (i2c_set_frequency_hz_name);
  jerry_release_value (i2c_set_frequency_hz_func);
  jerry_release_value (i2c_set_frequency_hz_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_set_clock_stretch_name = jerry_create_string ((const jerry_char_t*)"i2c_set_clock_stretch");
  jerry_value_t i2c_set_clock_stretch_func = jerry_create_external_function (i2c_set_clock_stretch_handler);
  jerry_value_t i2c_set_clock_stretch_ret = jerry_set_property (object, i2c_set_clock_stretch_name, i2c_set_clock_stretch_func);
  jerry_release_value (i2c_set_clock_stretch_name);
  jerry_release_value (i2c_set_clock_stretch_func);
  jerry_release_value (i2c_set_clock_stretch_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_write_name = jerry_create_string ((const jerry_char_t*)"i2c_write");
  jerry_value_t i2c_write_func = jerry_create_external_function (i2c_write_handler);
  jerry_value_t i2c_write_ret = jerry_set_property (object, i2c_write_name, i2c_write_func);
  jerry_release_value (i2c_write_name);
  jerry_release_value (i2c_write_func);
  jerry_release_value (i2c_write_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_read_name = jerry_create_string ((const jerry_char_t*)"i2c_read");
  jerry_value_t i2c_read_func = jerry_create_external_function (i2c_read_handler);
  jerry_value_t i2c_read_ret = jerry_set_property (object, i2c_read_name, i2c_read_func);
  jerry_release_value (i2c_read_name);
  jerry_release_value (i2c_read_func);
  jerry_release_value (i2c_read_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_start_name = jerry_create_string ((const jerry_char_t*)"i2c_start");
  jerry_value_t i2c_start_func = jerry_create_external_function (i2c_start_handler);
  jerry_value_t i2c_start_ret = jerry_set_property (object, i2c_start_name, i2c_start_func);
  jerry_release_value (i2c_start_name);
  jerry_release_value (i2c_start_func);
  jerry_release_value (i2c_start_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_stop_name = jerry_create_string ((const jerry_char_t*)"i2c_stop");
  jerry_value_t i2c_stop_func = jerry_create_external_function (i2c_stop_handler);
  jerry_value_t i2c_stop_ret = jerry_set_property (object, i2c_stop_name, i2c_stop_func);
  jerry_release_value (i2c_stop_name);
  jerry_release_value (i2c_stop_func);
  jerry_release_value (i2c_stop_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_status_name = jerry_create_string ((const jerry_char_t*)"i2c_status");
  jerry_value_t i2c_status_func = jerry_create_external_function (i2c_status_handler);
  jerry_value_t i2c_status_ret = jerry_set_property (object, i2c_status_name, i2c_status_func);
  jerry_release_value (i2c_status_name);
  jerry_release_value (i2c_status_func);
  jerry_release_value (i2c_status_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_force_bus_name = jerry_create_string ((const jerry_char_t*)"i2c_force_bus");
  jerry_value_t i2c_force_bus_func = jerry_create_external_function (i2c_force_bus_handler);
  jerry_value_t i2c_force_bus_ret = jerry_set_property (object, i2c_force_bus_name, i2c_force_bus_func);
  jerry_release_value (i2c_force_bus_name);
  jerry_release_value (i2c_force_bus_func);
  jerry_release_value (i2c_force_bus_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_slave_write_name = jerry_create_string ((const jerry_char_t*)"i2c_slave_write");
  jerry_value_t i2c_slave_write_func = jerry_create_external_function (i2c_slave_write_handler);
  jerry_value_t i2c_slave_write_ret = jerry_set_property (object, i2c_slave_write_name, i2c_slave_write_func);
  jerry_release_value (i2c_slave_write_name);
  jerry_release_value (i2c_slave_write_func);
  jerry_release_value (i2c_slave_write_ret);


  // set an external function as a property to the module object
  jerry_value_t i2c_slave_read_name = jerry_create_string ((const jerry_char_t*)"i2c_slave_read");
  jerry_value_t i2c_slave_read_func = jerry_create_external_function (i2c_slave_read_handler);
  jerry_value_t i2c_slave_read_ret = jerry_set_property (object, i2c_slave_read_name, i2c_slave_read_func);
  jerry_release_value (i2c_slave_read_name);
  jerry_release_value (i2c_slave_read_func);
  jerry_release_value (i2c_slave_read_ret);


  // set an enum constant as a property to the module object
  jerry_property_descriptor_t I2C_FREQ_80K_prop_desc;
  jerry_init_property_descriptor_fields (&I2C_FREQ_80K_prop_desc);
  I2C_FREQ_80K_prop_desc.is_value_defined = true;
  I2C_FREQ_80K_prop_desc.value = jerry_create_number (I2C_FREQ_80K);
  jerry_value_t I2C_FREQ_80K_name = jerry_create_string ((const jerry_char_t *)"I2C_FREQ_80K");
  jerry_value_t I2C_FREQ_80K_ret = jerry_define_own_property (object, I2C_FREQ_80K_name, &I2C_FREQ_80K_prop_desc);
  jerry_release_value (I2C_FREQ_80K_ret);
  jerry_release_value (I2C_FREQ_80K_name);
  jerry_free_property_descriptor_fields (&I2C_FREQ_80K_prop_desc);


  // set an enum constant as a property to the module object
  jerry_property_descriptor_t I2C_FREQ_100K_prop_desc;
  jerry_init_property_descriptor_fields (&I2C_FREQ_100K_prop_desc);
  I2C_FREQ_100K_prop_desc.is_value_defined = true;
  I2C_FREQ_100K_prop_desc.value = jerry_create_number (I2C_FREQ_100K);
  jerry_value_t I2C_FREQ_100K_name = jerry_create_string ((const jerry_char_t *)"I2C_FREQ_100K");
  jerry_value_t I2C_FREQ_100K_ret = jerry_define_own_property (object, I2C_FREQ_100K_name, &I2C_FREQ_100K_prop_desc);
  jerry_release_value (I2C_FREQ_100K_ret);
  jerry_release_value (I2C_FREQ_100K_name);
  jerry_free_property_descriptor_fields (&I2C_FREQ_100K_prop_desc);


  // set an enum constant as a property to the module object
  jerry_property_descriptor_t I2C_FREQ_400K_prop_desc;
  jerry_init_property_descriptor_fields (&I2C_FREQ_400K_prop_desc);
  I2C_FREQ_400K_prop_desc.is_value_defined = true;
  I2C_FREQ_400K_prop_desc.value = jerry_create_number (I2C_FREQ_400K);
  jerry_value_t I2C_FREQ_400K_name = jerry_create_string ((const jerry_char_t *)"I2C_FREQ_400K");
  jerry_value_t I2C_FREQ_400K_ret = jerry_define_own_property (object, I2C_FREQ_400K_name, &I2C_FREQ_400K_prop_desc);
  jerry_release_value (I2C_FREQ_400K_ret);
  jerry_release_value (I2C_FREQ_400K_name);
  jerry_free_property_descriptor_fields (&I2C_FREQ_400K_prop_desc);


  // set an enum constant as a property to the module object
  jerry_property_descriptor_t I2C_FREQ_500K_prop_desc;
  jerry_init_property_descriptor_fields (&I2C_FREQ_500K_prop_desc);
  I2C_FREQ_500K_prop_desc.is_value_defined = true;
  I2C_FREQ_500K_prop_desc.value = jerry_create_number (I2C_FREQ_500K);
  jerry_value_t I2C_FREQ_500K_name = jerry_create_string ((const jerry_char_t *)"I2C_FREQ_500K");
  jerry_value_t I2C_FREQ_500K_ret = jerry_define_own_property (object, I2C_FREQ_500K_name, &I2C_FREQ_500K_prop_desc);
  jerry_release_value (I2C_FREQ_500K_ret);
  jerry_release_value (I2C_FREQ_500K_name);
  jerry_free_property_descriptor_fields (&I2C_FREQ_500K_prop_desc);


  // set an enum constant as a property to the module object
  jerry_property_descriptor_t I2C_FREQ_600K_prop_desc;
  jerry_init_property_descriptor_fields (&I2C_FREQ_600K_prop_desc);
  I2C_FREQ_600K_prop_desc.is_value_defined = true;
  I2C_FREQ_600K_prop_desc.value = jerry_create_number (I2C_FREQ_600K);
  jerry_value_t I2C_FREQ_600K_name = jerry_create_string ((const jerry_char_t *)"I2C_FREQ_600K");
  jerry_value_t I2C_FREQ_600K_ret = jerry_define_own_property (object, I2C_FREQ_600K_name, &I2C_FREQ_600K_prop_desc);
  jerry_release_value (I2C_FREQ_600K_ret);
  jerry_release_value (I2C_FREQ_600K_name);
  jerry_free_property_descriptor_fields (&I2C_FREQ_600K_prop_desc);


  // set an enum constant as a property to the module object
  jerry_property_descriptor_t I2C_FREQ_800K_prop_desc;
  jerry_init_property_descriptor_fields (&I2C_FREQ_800K_prop_desc);
  I2C_FREQ_800K_prop_desc.is_value_defined = true;
  I2C_FREQ_800K_prop_desc.value = jerry_create_number (I2C_FREQ_800K);
  jerry_value_t I2C_FREQ_800K_name = jerry_create_string ((const jerry_char_t *)"I2C_FREQ_800K");
  jerry_value_t I2C_FREQ_800K_ret = jerry_define_own_property (object, I2C_FREQ_800K_name, &I2C_FREQ_800K_prop_desc);
  jerry_release_value (I2C_FREQ_800K_ret);
  jerry_release_value (I2C_FREQ_800K_name);
  jerry_free_property_descriptor_fields (&I2C_FREQ_800K_prop_desc);


  // set an enum constant as a property to the module object
  jerry_property_descriptor_t I2C_FREQ_1000K_prop_desc;
  jerry_init_property_descriptor_fields (&I2C_FREQ_1000K_prop_desc);
  I2C_FREQ_1000K_prop_desc.is_value_defined = true;
  I2C_FREQ_1000K_prop_desc.value = jerry_create_number (I2C_FREQ_1000K);
  jerry_value_t I2C_FREQ_1000K_name = jerry_create_string ((const jerry_char_t *)"I2C_FREQ_1000K");
  jerry_value_t I2C_FREQ_1000K_ret = jerry_define_own_property (object, I2C_FREQ_1000K_name, &I2C_FREQ_1000K_prop_desc);
  jerry_release_value (I2C_FREQ_1000K_ret);
  jerry_release_value (I2C_FREQ_1000K_name);
  jerry_free_property_descriptor_fields (&I2C_FREQ_1000K_prop_desc);


  // set an enum constant as a property to the module object
  jerry_property_descriptor_t I2C_FREQ_1300K_prop_desc;
  jerry_init_property_descriptor_fields (&I2C_FREQ_1300K_prop_desc);
  I2C_FREQ_1300K_prop_desc.is_value_defined = true;
  I2C_FREQ_1300K_prop_desc.value = jerry_create_number (I2C_FREQ_1300K);
  jerry_value_t I2C_FREQ_1300K_name = jerry_create_string ((const jerry_char_t *)"I2C_FREQ_1300K");
  jerry_value_t I2C_FREQ_1300K_ret = jerry_define_own_property (object, I2C_FREQ_1300K_name, &I2C_FREQ_1300K_prop_desc);
  jerry_release_value (I2C_FREQ_1300K_ret);
  jerry_release_value (I2C_FREQ_1300K_name);
  jerry_free_property_descriptor_fields (&I2C_FREQ_1300K_prop_desc);


  jerry_value_t I2C_MAX_BUS_js = jerry_create_number (I2C_MAX_BUS);


  // set a global constant or a macro as a property to the module object
  jerry_property_descriptor_t I2C_MAX_BUS_prop_desc;
  jerry_init_property_descriptor_fields (&I2C_MAX_BUS_prop_desc);
  I2C_MAX_BUS_prop_desc.is_value_defined = true;
  I2C_MAX_BUS_prop_desc.value = I2C_MAX_BUS_js;
  jerry_value_t I2C_MAX_BUS_prop_name = jerry_create_string ((const jerry_char_t *)"I2C_MAX_BUS");
  jerry_value_t I2C_MAX_BUS_return_value = jerry_define_own_property (object, I2C_MAX_BUS_prop_name, &I2C_MAX_BUS_prop_desc);
  jerry_release_value (I2C_MAX_BUS_return_value);
  jerry_release_value (I2C_MAX_BUS_prop_name);
  jerry_free_property_descriptor_fields (&I2C_MAX_BUS_prop_desc);


  jerry_value_t I2C_USE_GPIO16_js = jerry_create_number (I2C_USE_GPIO16);


  // set a global constant or a macro as a property to the module object
  jerry_property_descriptor_t I2C_USE_GPIO16_prop_desc;
  jerry_init_property_descriptor_fields (&I2C_USE_GPIO16_prop_desc);
  I2C_USE_GPIO16_prop_desc.is_value_defined = true;
  I2C_USE_GPIO16_prop_desc.value = I2C_USE_GPIO16_js;
  jerry_value_t I2C_USE_GPIO16_prop_name = jerry_create_string ((const jerry_char_t *)"I2C_USE_GPIO16");
  jerry_value_t I2C_USE_GPIO16_return_value = jerry_define_own_property (object, I2C_USE_GPIO16_prop_name, &I2C_USE_GPIO16_prop_desc);
  jerry_release_value (I2C_USE_GPIO16_return_value);
  jerry_release_value (I2C_USE_GPIO16_prop_name);
  jerry_free_property_descriptor_fields (&I2C_USE_GPIO16_prop_desc);

  jerry_release_value (object);
}
