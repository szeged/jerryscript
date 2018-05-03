# Wi-Fi

This module stands for establish Wi-Fi connection and send/receive data.

## Methods
### Wifi.connect(ssid, password)
  - `ssid` {string} - Baud to be used.
  - `password` {string} - Baud to be used.
  - **Note:** This function blocks the execution until no IP is given or fails with timeout.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Connects to Wi-Fi AP with the given `ssdi` and `password`. If the device able to connect to the AP synchronises it's RTC clock with the received data from an SNTP server.

**Example**

```js
var success = WIFI.connect("myssid", "mypassword");
var date = new Date (); // this would represent the actual time
```

### Wifi.disconnect()
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Closes the connection with the current Wi-Fi AP.

**Example**

```js
var success = WIFI.connect("myssid", "mypassword");
// Do some stuff
success = WIFI.disconnect();
```
### Wifi.receive(host, port, address)
  - `host` {string} - Host url.
  - `port` {integer} - Port to connect.
  - `address` {string} - Address to get.
  - **Note:** The maximum size of data that can be reveived is 1024B due to memory limitation.
  - Returns: {string} - Recevived data.

  Tries to receive data from the given `host`:`port`/`address`.

**Example**

```js
if (WIFI.available()){
  var result = WIFI.get("192.168.0.2", 80, "/config.json");
}
```

### Wifi.send(host, port, data, name, size)
  - `host` {string} - Host url.
  - `port` {integer} - Port to connect.
  - `data` {Any} - Data to be send.
  - `name` {string} - Name of the file that the receiver saves.
  - `size` {integer} - Data size.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Tries to send data to the given `host`:`port`.
  The function sends the data in the next format:

  #`FileNameLengthInHex`#&`FileName`&

  #`DataPackageSizeInHex`#&`DataPackage`& - repeatedly based on data size

**Example**

```js
var string = "someKindOfString";
var file = SD.open("/path/to/myFile", SD.readonly);
if (WIFI.available()){
  /**
   * The actual data to be written to socket:
   * #A#&myFile.txt&#10#&someKindOfString&
   */
  var success = WIFI.send("192.168.0.5", 8000, string, "myFile.txt", string.length);
  success = WIFI.send("192.168.0.5", 8000, file, file.openPath, Sd.fileSize(file));
}

SD.close(file);
```
