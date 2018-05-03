#ifndef ESP_FS_JS_H
#define ESP_FS_JS_H

#include "jerry_extapi.h"
#include <fatfs/ff.h>

#define FS_OBJECT_NAME "FS"
#define FS_READ "read"
#define FS_WRITE "write"
#define FS_EXISTS "exists"
#define FS_REMOVE "remove"
#define FS_BUFFER_SIZE 1024

void register_fs_object (jerry_value_t global_object);

#endif
