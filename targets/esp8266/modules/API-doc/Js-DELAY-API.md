# DELAY

This module stands for handling system time, delaying etc.

## Methods
### DELAY.millis(ms)
  - `ms` {integer} - Sleep time in milliseconds.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Suspends the execution for `ms` milliseconds.

**Example**

```js
var success = DELAY.millis(150);
// Wait for 150 ms
// Do something else
```

### DELAY.micros(us)
  - `us` {integer} - Sleep time in microseconds.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  Suspends the execution for `us` microseconds.

**Example**

```js
var success = DELAY.micros(150);
// Wait for 150 us
// Do something else
```
### DELAY.systemTime()
  - Returns: {integer} - Elapsed time since booting.

  Return the elapsed time in milliseconds since the system starting

**Example**

```js
var startTime = DELAY.systemTime();
// Do some stuff here
var endTime = DELAY.systemTime();
print ("Elapsed time: ", endTime - startTime);

```

### DELAY.deepSleep(ms)
  - `ms` {integer} - Deep sleep time in milliseconds.
  - **NOTE:** GPIO16 must be connected to the board RES pin.
  - Returns: {boolean} - `true` | `false` based on the operation success.

  The board goes to deepSleep for `ms` milliseconds. In this state comsumes considerable less energy by disabling all system units except the RTC clock. After `ms` milliseconds the GPIO16 sends signal by pulling itself down. This signal used for waking up to board by resetting itself. After the reset the whole JavaScript initialization starts from the beginning. To save important data from the previous context `FS` module can be used.

**Example**

```js
// Do some stuff
var success = DELAY.deepSleep(1000 * 60 * 60); // deepSleep for an hour
```
