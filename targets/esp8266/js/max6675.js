function max6675(pin_cs, pin_sck, pin_so, unit_type) {
  this.pin_cs = pin_cs;
  this.pin_sck = pin_sck;
  this.pin_so = pin_so;
  this.unit_type = unit_type;

  switch (unit_type) {
    default:
    case "C":
    case "c": this.converter = this._toCelsius;
              break;
    case "K":
    case "k": this.converter = this._toKelvin;
              break;
    case "F":
    case "f": this.converter = this._toFarenheit;
              break;
  }

  GPIO.pinMode(this.pin_cs, GPIO.OUTPUT);
  GPIO.pinMode(this.pin_sck, GPIO.OUTPUT);
  GPIO.pinMode(this.pin_so, GPIO.INPUT);

  DELAY.micros (100);
  GPIO.write(this.pin_cs, GPIO.HIGH);
}

max6675.prototype._rawDataToTemp = function(value) {
  return ((value >> 3) * 0.25);
}

max6675.prototype.readTemp = function() {
  //Conversion Time: 0.25s
  DELAY.millis (250);

  //Perform multiple number of measures for more accurate result
  var sum = 0;
  var test_cases = 4;
  for (var i = 0; i < test_cases; i++) {
    GPIO.write(this.pin_cs, GPIO.LOW);
    DELAY.micros(10);

    var value = this._readByte() << 8;
    value |= this._readByte();

    GPIO.write(this.pin_cs, GPIO.HIGH);
    sum += value;
  }

  return this.converter(this._rawDataToTemp(sum / test_cases));
}

max6675.prototype.toString = function() {
  return "Temperature: " + this.readTemp();
}

max6675.prototype._readByte = function() {
  var value = 0;

  for (var i = 7; i >= 0; i--)
  {
    GPIO.write(this.pin_sck, GPIO.LOW)
    DELAY.micros(100);
    if (GPIO.read(this.pin_so))
    {
        value |= (1 << i);
    }
    GPIO.write(this.pin_sck, GPIO.HIGH)
    DELAY.micros(100);
  }

  return value;
}

max6675.prototype._toCelsius = function(value) {
  return value + "C";
}

max6675.prototype._toKelvin = function(celsius) {
  return (celsius + 273.15) + " K";
}

max6675.prototype._toFarenheit = function(celsius) {
  return (celsius * 9.0 / 5.0 + 32) + " F";
}
