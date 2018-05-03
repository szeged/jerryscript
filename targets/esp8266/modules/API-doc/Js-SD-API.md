# SD

This module stands for SD card communication. The SD card must contain a valid FAT32 file system.

To connect SD-card reader module follow the next wiring specification:

| Board pin | SD card reader pin |
| ------ | ------ |
| D5 (GPIO14) | SCK |
| D6 (GPIO12) | MISO |
| D7 (GPIO13) | MOSI |
| Any accessible GPIO_PIN| CS |

Also connect VCC and GRD appropriately.

## Constants

### SD_FLAGS
All Posix file flags represented with a constant according to this table:

| Posix flag | SD_FLAGS |
| ---------- | -------- |
|  `r`  | `SD.readonly` |
|  `r+` | `SD.readWrite` |
|  `w`  | `SD.creatEmptyForWrite` |
|  `w+` | `SD.creatEmptyForReadWrite` |
|  `a`  | `SD.appendWrite` |
|  `a+` | `SD.appendReadWrite` |
|  `wx` | `SD.createEmptyForWriteIfNotExists` |
| `w+x` | `SD.createEmptyForReadWriteIfNotExists` |

### SD_TEXT_MODES
 - SD.asText (default) - Writes the content in text format.
 - SD.asBinary - Writes the content in binary format (e.g. images).

## Methods

**Note**: All operations can throw an error if any kind of problem occurs in the low level disk I/O layer or in case of invalid request is sent to the file system.

### SD.mount(csPin)
  - `csPin` {GPIO_PIN} - One of GPIO{15, 5, 4, 0, 2, 16}.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Mounts the SD card for further operations.

**Example**

```js
var success = SD.mount(15);
//Do some stuff
SD.unmount();
```

### SD.unmount()
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Unmounts the SD card.

**Example**

```js
var success = SD.mount(15);
//Do some stuff
SD.unmount();
```

### SD.open(path, flag)
  - `path` {string} - Relative or absolute path to file to be opened.
  - `flag` {SD_FLAG} - Any kind of SD_FLAG.
  - Returns: {object} - File descriptor representing object.

  Opens a file located in `path` with the specified `flag`.

**Example**

```js
var success = SD.mount(15);
var myFile = SD.open("myFile.txt", SD.readonly);
print (myFile.openPath); //"myFile.txt"
print (myFile.openFlag); //"readonly"
//Use myFile
SD.close(myFile);
SD.unmount();
```

### SD.close(file)
  - `file` - Valid file descriptor representing object.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Closes a previously opened file descriptor representing object.

**Example**

```js
var success = SD.mount(15);
var myFile = SD.open("myFile.txt", SD.readonly);
//Use myFile
SD.close(myFile);
SD.unmount();
```

### SD.write(file, data, mode, count)
  - `file` {object} - Valid file descriptor representing object.
  - `data` {Any} - Data to be written.
  - `mode` {SD_TEXT_MODE} - Any kind of SD_TEXT_MODE.
  - `count` {integer} - Optional. Number of bytes to be written.
  - **Note:** in SD.asText mode `count` default value is the length of the stringified `data`.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Writes `count` bytes from `data` to the previously opened `file` in `write` mode.

**Example**

```js
var success = SD.mount(15);
var myFile = SD.open("myFile.txt", SD.appendWrite);
success = SD.write(myFile, "This text is going to be saved", SD.asText);
var binaryData = [0x00, 0x12, 0x24, 0x45];
success = SD.write(myFile, binaryData, SD.asBinary, binaryData.length);
SD.close(myFile);
SD.unmount();
```

### SD.read(file, data, mode, count)
  - `file` {object} - Valid file descriptor representing object.
  - `mode` {SD_TEXT_MODE} - Any kind of SD_TEXT_MODE.
  - `count` {integer} - Optional. Desired number of bytes to be read.
  - **Note:** in SD.asText mode `count` default value is size of `file`.
  - Returns: {string} - Read data from `file`.

  Reads `count` bytes from the previously opened `file` in `write` mode.

**Example**

```js
var success = SD.mount(15);
var myFile = SD.open("myFile.txt", SD.appendWrite);
var string = SD.read(myFile, SD.asText);
string = SD.read(myFile, SD.asBinary, 10);
SD.close(myFile);
SD.unmount();
```

### SD.fileSize(file)
  - `file` {object} - Valid file descriptor representing object.
  - Returns: {integer} - Size of the `file`.

  Determines the size of the given `file`.

**Example**

```js
var success = SD.mount(15);
var myFile = SD.open("myFile.txt", SD.appendWrite);
var size = SD.fileSize(myFile);
SD.close(myFile);
SD.unmount();
```

### SD.exists(path)
  - `path` {string} - Location of the file to be checked.
  - Returns: {boolean} - `true` | `false` based on the existance of the given `path`.

  Determines the existance of the given `path`.

**Example**

```js
var success = SD.mount(15);
if (SD.exists("path/to/myFile.txt")){
  //Do something
} else {
  //Do something else
}
SD.unmount();
```

### SD.endOfFile(file)
  - `file` {object} - Valid file descriptor representing object.
  - Returns: {boolean} - `true` | `false` based on the end of the `file` has been reached.

  Determines whether the end of the file has been reached.

**Example**

```js
var success = SD.mount(15);
var myFile = SD.open("myFile.txt", SD.appendWrite);
while (!SD.endOfFile(myFile)){
  var str = SD.read(myFile, SD.asBinary, 10);
  //Do something with str
}
SD.close(myFile);
SD.unmount();
```

### SD.rename(oldPath, newPath)
  - `oldPath` {string} - Location of the file/directory to be renamed.
  - `newPath` {string} - Location of the file/directory to rename to.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Renames `oldPath` to `newPath`.

**Example**

```js
var success = SD.mount(15);
success = SD.rename("myFile.txt", "myNewFile.txt");
success = SD.rename("/directory/myFile.txt", "/anotherdirectory/myNewFile.txt");
success = SD.rename("/mydirectory", "/myAnotherdirectory");
SD.unmount();
```

### SD.delete(path)
  - `path` {string} - Location of the file to be deleted.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Deletes the file/directory in the given `path`.

**Example**

```js
var success = SD.mount(15);
if (SD.exists("path/to/myFile.txt")){
  SD.delete("path/to/myFile.txt");
} else {
  //Do something else
}
SD.unmount();
```

### SD.find(path, pattern)
  - `path` {string} - Location of the directory to be search in.
  - `pattern` {string} - Pattern for element mathcing.
  - Returns: {string} - Found file/directory absolute path.

  Find matching file/directory in `path` for `pattern`.

### SD.findNext()
  - **Note:** SD.find must be called before using this operation.
  - Returns: {string} - Found file/directory absolute path.

  Find next matching file/directory according to the previous search.

**Example**

```js
var success = SD.mount(15);
var path = SD.find ("myDirectory", "dsc_*.jpg");
while (path != undefined)
{
  //Do something with path
  path = SD.findNext();
}
SD.unmount();
```

### SD.mkdir(path)
  - `path` {string} - Location of the directory to be created.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Creates a directory with the given `path`.

**Example**

```js
var success = SD.mount(15);
success = SD.mkdir("path/to/newDirectory");
SD.unmount();
```

### SD.openDirectory(path)
  - `path` {string} - Location of the directory to be opened.
  - Returns: {object} - Directory descriptor representing object.

  Opens a directory with the given `path`.

### SD.readDirectoryStructure(directory)
  - `directory` {object} - Valid directory descriptor representing object.
  - Returns: {object} - Directory structure descriptor representing object.

  Reads a directory with the given `directory` descriptor.

### SD.isDirectory(directoryStructure)
  - `directoryStructure` {object} - Valid directory structure descriptor representing object.
  - Returns: {boolean} - `true` | `false` based on the operation result.

  Checks whether the given `directoryStructure` represents a directory.

**Example**

This example show inform about the directory structure on the card. With recursive implementation the whole file system can be visited.
```js
var success = SD.mount(15);
var path = "/";
var directory = SD.openDirectory (path); // root directory ("/")
// directory.path = "/"
var directoryStructure = SD.readDirectoryStructure(directory);
while (directoryStructure != undefined){
  if (SD.isDirectory (directoryStructure)){
    // directoryStructure.elementName = "/subDir"
    var subDirectoryPath = (path == "/" ? "" : path) + "/" + directoryStructure.elementName;
    // Do something with the sub-directory
  } else {
    // directoryStructure.elementName = "/myFile"
    // Do something with the file
  }
  // read the next structure element
  directoryStructure = SD.readDirectoryStructure(directory);
}

SD.unmount();
```
