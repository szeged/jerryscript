# SensorCollection

This module stands for using multiple data providing sensors with the similar inteface for data providing.

## Constructor
### sensorCollection()

  Creates a new `sensorCollection` object.

**Example**

```js
var sensColl = new sensorCollection();
```

## Methods
### uCam.addSensor(sensor)
  - `sensor` {object} - Sensor representing object.
  - Returns: {undefined}

  Adds a new sensor to sensorCollection.

**Example**

```js
var tempSensor = new max6675(0, 4, 5, "C");
var sensColl = new sensorCollection();
sensColl.addSensor(tempSensor);
```

### uCam.storeAllSensorsData(path)
  - `path` {string} - Location of the file to be saved.
  - Returns: {boolean} - `true` | `false` based on all sensor data been stored succesfully.

  Attempts to store all previously added sensor data. This operation calls `toString()` operation for all sensors.

**Example**

```js
var tempSensor = new max6675(0, 4, 5, "C");
var aSensor = new anotherSensor();
var sensColl = new sensorCollection();
sensColl.addSensor(tempSensor);
sensColl.addSensor(aSensor);
if (!sensColl.storeAllSensorsData("data.txt")){
  // Handle error
}

// All sensor data has been stored.

```
