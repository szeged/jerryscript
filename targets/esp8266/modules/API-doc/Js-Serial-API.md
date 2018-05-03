# Serial

The board `RX` and `TX` pins are should be connected to another device to establish serial communication.
To use Serial module `REDIRECT_STDOUT={ON, NONE}` must be used in the build process.

## Methods
### Serial.init(baud)
  - `baud` {integer} - Baud to be used.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Initializes the serial module with `baud` bps.

**Example**

```js
var success = Serial.init(115200);
```

### Serial.write(byte | array)
  - `byte` {integer} - Data to be written. Data must be in 0 - 255 range.
  - `array` {Array} - Data array to be written. Array elements must be in 0 - 255 range.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Writes `byte` or `array` to the serial buffer.

**Example**

```js
var success = Serial.write(0x25);

var array = [0x00, 0x12, 0x98, 0xFF];
success = Serial.write(array);
```

### Serial.read()
  - `byte` {integer} - Data to be written. Data must be in (0 - 255).
  - `array` {Array} - Data array to be written. Array elements must be in (0 - 255).
  - **Note:** This operation is blocking!
  - Returns: {integer} - Byte read from serial buffer.

  Reads a byte from serial buffer.

**Example**

```js
while (!Serial.available()){
  // wait
}
var intValue = Serial.read();
```

### Serial.available()
  - Returns: {integer} - Available bytes in serial buffer.

  Reads available bytes count from serial buffer.

**Example**

```js
while (!Serial.available()){
  // wait
}
// Do some other stuff
```

### Serial.available(count)
  - `count` {integer} - Waits at least `count` bytes of data.
  - **Note:** This function blocks the execution still `count` amount bytes data is not available.
  - Returns: {integer} - Available bytes in serial buffer.

  Reads available bytes count from serial buffer.

**Example**

```js
var availableBytes = Serial.available(128);
// Do some other stuff
```

### Serial.flush()
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Flushes all data from serial buffer.

**Example**

```js
var success = Serial.flush();
// Do some other stuff
var success = Serial.write(0x25);
```
