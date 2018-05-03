# UCAM-III camera module

## Wiring
| Board pin | Camera pin |
| ------ | ------ |
| RX | TX |
| TX | RX |
| GPIO_PIN* | RES |

\* Any accessible GPIO_PIN. Also connect VCC (5V) and GRD appropriately.

## Constructor
### uCamIII(reset)
  - `reset` {GPIO_PIN} - `GPIO_PIN` number that connects to the sensor `RES` pin.

  Creates a new `uCamIII` object.

**Example**

```js
var uCam = new uCamIII(5);
```

## Methods
### uCam.init()
  - Returns: {boolean} - `true` | `false` based on the synchronisation success.

  Attempts to syncronise with the camera by performing multipe ACK requests. The initialization process can take a long time and the device may resets during the operation to increase the reliability the next initialization process.

**Example**

```js
var uCam = new uCamIII(5);
if (uCam.init()){
  // Do something
} else {
  throw new Error ("Cannot connect to uCam-III");
}
```

### uCam.takePicture()
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Attempts to take a picture with the camera. If this operation is succeeded without error, the image would be stored in the device own memory.

**Example**

```js
var uCam = new uCamIII(5);
if (uCam.init()){
  if (uCam.takePicture()){
    // Do something
  } else {
    throw new Error ("Cannot take picture with uCam-III");
  }
} else {
  throw new Error ("Cannot connect to uCam-III");
}
```

### uCam.storePicture(path)
  - `path` {string} - Relative or absolute path for the new picture.
  - **NOTE:** For this operation the SD card must be mounted.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Attempts to store the previously taken picture. If the operation is succeeded without error, the image would be stored in the SD card with the specified `path`.

**Example**

```js
var uCam = new uCamIII(5);
if (uCam.init()){
  if (uCam.takePicture()){
    SD.mount(15);
    if(uCam.storePicture("mypicture.jpg")){
      print ("The picture has been stored.");
    } else {
      throw new Error ("Cannot store picture");
    }
    SD.unmount();
  } else {
    throw new Error ("Cannot take picture with uCam-III");
  }
} else {
  throw new Error ("Cannot connect to uCam-III");
}
```
