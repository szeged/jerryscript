# FS

This module stands for using the internal flash memory for storing data.

## Methods
### FS.exists(path)
  - `path` {string} - Absolute path to file to be checked.
  - Returns: {boolean} - `true` | `false` based on the operation result.

  Determines the existence of the given `path`.

**Example**

```js
if (FS.exist("myFile.txt")){
  // Do something with it
} else {
  // Do something else
}
```

### FS.write(path, data)
  - `path` {string} - Absolute path to file to be created.
  - `data` {Any} - Data to be written. (The `data` will be stringified),
  - Returns: {boolean} - `true` | `false` based on the operation result.

  Writes `data` into a new file with the specified `path`.

**Example**

```js
var filepath = "myFile.txt";
if (FS.exist(filepath)){
  // Do something with it
} else {
  FS.write(filepath, "This string is going to be stored.")
}
```

### FS.read(path)
  - `path` {string} - Absolute path to file to be read.
  - **NOTE:** - The max length of the content that can be read is 1024B due to memory limitation.
  - Returns: {string} - Read data.

  Reads the file content with the specified `path`.

**Example**

```js
var filepath = "myFile.txt";
var content;
if (FS.exist(filepath)){
  content = FS.read (filepath);
} else {
  FS.write(filepath, "This string is going to be stored.")
}
```

### FS.remove(path)
  - `path` {string} - Absolute path to file to be removed.
  - Returns: {boolean} - `true` | `false` based on the operation result.

  Removes the file with the specified `path`.

**Example**

```js
var filepath = "myFile.txt";
var content;
if (FS.exist(filepath)){
  content = FS.read (filepath);
  //File is no longer needed
  FS.remove(filepath)
} else {
  FS.write(filepath, "This string is going to be stored.")
}
```
