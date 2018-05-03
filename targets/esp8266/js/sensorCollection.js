function sensorCollection () {
  this.sensorList = [];
}

sensorCollection.prototype.addSensor = function (sensor) {
  this.sensorList.push(sensor);
};

sensorCollection.prototype.storeAllSensorsData = function (path) {
  var file = SD.open(path, SD.appendWrite);
  var success = true;
  for (var i = 0; i < this.sensorList.length; i++) {
    try {
      SD.write (file, this.sensorList[i].toString(), SD.asText);
    } catch (e) {
      success = false;
    }
  }
  SD.close (file);
  return success;
};
