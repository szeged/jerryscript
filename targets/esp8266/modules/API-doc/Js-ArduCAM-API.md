# ArduCAM

This module stands for handling low level interactions with the ArduCAM OV5642 5MP Plus device.

This device uses both SPI and I2C interface. The appropriate wiring is the following:

| Board pin | ArduCAM pin |
| ------ | ------ |
| D7 (GPIO13) | MOSI |
| D6 (GPIO12) | MISO |
| D5 (GPIO14) | SCK |
| D4 (GPIO2) | CS |
| D2 (GPIO4) | SCL |
| D1 (GPIO5) | SDA |

GND and VCC also need to be connected appropriately.

Note that the SD card reader also uses the SPI interface. The two devices can share the MOSI, MISO and SCK pins, but CS needs to be different for each of them. An example configuration would look like this:

| Board pin | ArduCAM pin | SD reader pin |
| ------ | ------ | ------ |
| D7 (GPIO13) | MOSI | MOSI |
| D6 (GPIO12) | MISO | MISO |
| D5 (GPIO14) | SCK | SCK |
| D4 (GPIO2) | CS |  |
| D3 (GPIO0) |  | CS |
| D2 (GPIO4) | SCL |  |
| D1 (GPIO5) | SDA |  |

D1, D2, D3 and D4 pins can be configured freely. To change the above functions to your custom configuration, you need to change the `I2C_SDA_PIN`, `I2C_SCL_PIN`, `CAMERA_CS` and `SD_CS` values in `modules/include/esp_arducam_js.h`.

## Constants

### ArduCAM.SD_CS

A number value holding the number of the CS pin used by the SD card reader.

### IMAGE_SIZE

Image sizes are represented by constants according to this table:

| Image size | IMAGE_SIZE |
| ---------- | -------- |
| EMPTY | `ArduCAM.EMPTY` |
| 320x240 | `ArduCAM.IMG_SIZE_320x240` |
| 640x480 | `ArduCAM.IMG_SIZE_640x480` |
| 1024x768 | `ArduCAM.IMG_SIZE_1024x768` |
| 1280x960 | `ArduCAM.IMG_SIZE_1280x960` |
| 1600x1200 | `ArduCAM.IMG_SIZE_1600x1200` |
| 1920x1080 | `ArduCAM.IMG_SIZE_1920x1080` |
| 2048x1536 | `ArduCAM.IMG_SIZE_2048x1536` |
| 2592x1944 | `ArduCAM.IMG_SIZE_2592x1944` |

**Note**: the `EMPTY` image size exists to keep compatibility with the uCam device. In the case of ArduCAM, this will fallback to 320x240.

## Methods

### ArduCAM.init()
  - Returns: `true` on success, thrown `Error` otherwise.

  Initializes the device.

**Example**

```js
try {
  ArduCAM.init ();
} catch (e) {
  print("Error occurred while initializing ArduCAM: ");
  print(e.toString())
}
````

### ArduCAM.setImageSize(size)
  - `size` {IMAGE_SIZE} - one of the possible `IMAGE_SIZE` values.
  - Returns: `true` on success, thrown `Error` otherwise.

  Sets the internal `image_size` variable of the module (otherwise unreachable from Javascript code).

**Note**: this will not communicate with the device in any way. The actual configuration of the camera's image size take will happen directly before capturing.

**Example**

```js
var imageSizeMap = {
  "EMPTY": 0,
  "320x240": 1,
  "640x480": 2,
  "1024x768": 3,
  "1280x960": 4,
  "1600x1200": 5,
  "1920x1080": 6,
  "2048x1536": 7,
  "2592x1944": 8
}

try {
  ArduCAM.setImageSize(imageSizeMap["640x480"]);
} catch (e) {
  print("An error occurred while setting image size: ")
  print (e.toString())
}
```

### ArduCAM.capture()
  - Returns: `true` on success, thrown `Error` otherwise.

  Configures the device image size through the I2C interface, then gives the device the command to take a picture.

**Note**: the camera must be initialized before capturing. It is also recommended to call `ArduCAM.setImageSize()`, otherwise the low level implementation will default to `320x240`.

**Example**

```js
var imageSizeMap = {
  "EMPTY": 0,
  "320x240": 1,
  "640x480": 2,
  "1024x768": 3,
  "1280x960": 4,
  "1600x1200": 5,
  "1920x1080": 6,
  "2048x1536": 7,
  "2592x1944": 8
}

try {
  ArduCAM.init();
  ArduCAM.setImageSize(imageSizeMap["640x480"]);
  ArduCAM.capture();
} catch (e) {
  print("An error occurred: ")
  print(e)
}
```

### ArduCAM.store(fd)
  - `fd` {object} - A valid file descriptor.
  - Returns: `true` on success, thrown `Error` otherwise.

  Reads the image from the camera's storage and saves it to the SD card.

**Note**: The camera and the reader will need to share the same SPI interface as shown at the top of this document.

**Example**

```js
var imageSizeMap = {
  "EMPTY": 0,
  "320x240": 1,
  "640x480": 2,
  "1024x768": 3,
  "1280x960": 4,
  "1600x1200": 5,
  "1920x1080": 6,
  "2048x1536": 7,
  "2592x1944": 8
}

try {
  ArduCAM.init();
  ArduCAM.setImageSize(imageSizeMap["640x480"]);
  ArduCAM.capture();

  SD.mount(ArduCAM.SD_CS);
  var fd = SD.open("temp.jpg", SD.createEmptyForWrite);

  ArduCAM.store(fd);
  SD.close(fd);
  SD.unmount();
} catch (e) {
  print("An error occurred: ")
  print(e)
}
```

### `ArduCAM._print()`
  - Returns: `undefined`

  A helper function meant for testing. Prints the bytes of the image to the serial console (can be accessed by `minicom`).

**Example**

```js
var imageSizeMap = {
  "EMPTY": 0,
  "320x240": 1,
  "640x480": 2,
  "1024x768": 3,
  "1280x960": 4,
  "1600x1200": 5,
  "1920x1080": 6,
  "2048x1536": 7,
  "2592x1944": 8
}

try {
  ArduCAM.init();
  ArduCAM.setImageSize(imageSizeMap["640x480"]);
  ArduCAM.capture();
  ArduCAM._print();
} catch (e) {
  print("An error occurred: ")
  print(e)
}
```

On your PC:
```sh
minicom -D /dev/ttyUSB0 # ttyUSB0 can change depending on what other devices you have plugged in
```
