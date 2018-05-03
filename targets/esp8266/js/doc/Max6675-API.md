# MAX-6675 temperature sensor

## Constructor
### max6675(cs, sck, so, unit)
  - `cs` {GPIO_PIN} - `GPIO_PIN` number that connects to the sensor `CS` pin
  - `sck` {GPIO_PIN} - `GPIO_PIN` number that connects to the sensor `SCK` pin
  - `so` {GPIO_PIN} - `GPIO_PIN` number that connects to the sensor `SO` pin
  - `unit` {string} - One of {`"C"`, `"F"`, `"K"`}. Default is celsius.

  Creates a new `max6675` object.

**Example**

```js
var tempSensor = new max6675(0, 4, 5, "C");
```

## Methods
### max6675.readTemp()
  - Returns: {integer} - The actual temperature represented in the prevoiusly set unit.

  Reads the actual temperature from the sensor. This method performs 4 measures and returns the average of them for more accurate data.

**Example**

```js
var tempSensor = new max6675(0, 4, 5, "C");
var temp = tempSensor.readTemp(); //24.25
```

### max6675.toString()
  - Returns: {string} - The actual temperature in stringified context.

  Reads the actual temperature from the sensor and returns a string template accourding to it.

**Example**

```js
var tempSensor = new max6675(0, 4, 5, "C");
var tempStr = tempSensor.toString(); // "Temperature: 24.25 C"
```
