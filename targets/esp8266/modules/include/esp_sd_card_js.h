#ifndef ESP_SD_CARD_JS_H
#define ESP_SD_CARD_JS_H

#include "jerry_extapi.h"
#include <fatfs/ff.h>

#define SD_CARD_OBJECT_NAME "SD"
#define SD_CARD_MOUNT "mount"
#define SD_CARD_UNMOUNT "unmount"
#define SD_CARD_OPEN_FLAG "openFlag"
#define SD_CARD_OPEN_PATH "openPath"
#define SD_CARD_OPEN "open"
#define SD_CARD_CLOSE "close"
#define SD_CARD_READ "read"
#define SD_CARD_WRITE "write"
#define SD_CARD_FILE_SIZE "fileSize"
#define SD_CARD_FILE_EXISTS "exists"
#define SD_CARD_FILE_EOF "endOfFile"
#define SD_CARD_MKDIR "mkdir"
#define SD_CARD_FIND "find"
#define SD_CARD_FIND_NEXT "findNext"
#define SD_CARD_RENAME "rename"
#define SD_CARD_DELETE "delete"
#define SD_CARD_OPEN_DIRECTORY "openDirectory"
#define SD_CARD_READ_DIRECTORY_STRUCTURE "readDirectoryStructure"
#define SD_CARD_IS_DIRECTORY "isDirectory"
#define SD_CARD_DIRECTORY_PATH "path"
#define SD_CARD_DIRECTORY_STRUCTURE_ELEMENT "elementName"
#define SD_CARD_AS_TEXT "asText"
#define SD_CARD_AS_BINARY "asBinary"
#define SD_CARD_WRITE_TEXT true
#define SD_CARD_WRITE_BINARY false

#define SD_CARD_READONLY "r"
#define SD_CARD_READWRITE "r+"
#define SD_CARD_CREATE_EMPTY_FOR_WRITE "w"
#define SD_CARD_CREATE_EMPTY_FOR_READWRITE "w+"
#define SD_CARD_APPEND_WRITE "a"
#define SD_CARD_APPEND_READWRITE "a+"
#define SD_CARD_CREATE_EMPTY_FOR_WRITE_IF_NOT_EXISTS "wx"
#define SD_CARD_CREATE_EMPTY_FOR_READWRITE_IF_NOT_EXISTS "w+x"

#define SD_CARD_READONLY_PROP_NAME "readonly"
#define SD_CARD_READWRITE_PROP_NAME "readWrite"
#define SD_CARD_CREATE_EMPTY_FOR_WRITE_PROP_NAME "createEmptyForWrite"
#define SD_CARD_CREATE_EMPTY_FOR_READWRITE_PROP_NAME "createEmptyForReadWrite"
#define SD_CARD_APPEND_WRITE_PROP_NAME "appendWrite"
#define SD_CARD_APPEND_READWRITE_PROP_NAME "appendReadWrite"
#define SD_CARD_CREATE_EMPTY_FOR_WRITE_IF_NOT_EXISTS_PROP_NAME "createEmptyForWriteIfNotExists"
#define SD_CARD_CREATE_EMPTY_FOR_READWRITE_IF_NOT_EXISTS_PROP_NAME "createEmptyForReadWriteIfNotExists"

#define SD_CARD_MAX_F_IO_ATTEMPTS 5
#define SD_CARD_USE_APPEND -1
#define DOUBLE_STO_STRING_PRECISION 50
#define WORK_BUFFER_SIZE 512

const jerry_object_native_info_t* get_native_file_obj_type_info (void);


static void *work_buffer __attribute__((unused));

void register_sd_card_object (jerry_value_t global_object);

#endif
