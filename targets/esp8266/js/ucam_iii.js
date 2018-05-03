function uCamIII (reset_pin) {
  Serial.init(115200);
  this.sync_attempts_max = 5;
  this.command_length = 6;

  this.buffer = new ArrayBuffer (122);
  this.imageBuffer = new Uint8Array (this.buffer);
  this.sync_command = [0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00];
  this.sync_ack_reply_ext = [0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00];
  this.sync_final_command = [0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00];

  this.initial_command = [0xAA, 0x01, 0x00, 0x07, 0x07, 0x07];
  this.generic_ack_reply = [0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00];
  this.pack_size = [0xAA, 0x06, 0x08, 0x80, 0x00, 0x00];
  this.snapshot = [0xAA, 0x05, 0x00, 0x00, 0x00, 0x00];
  this.get_picture = [0xAA, 0x04, 0x01, 0x00, 0x00, 0x00];
  this.reset_command = [0xAA, 0x08, 0x00, 0x00, 0x00, 0xFF];

  this.image_pos = 0;
  this.imageSize = 0
  this.reset_pin = reset_pin;
  GPIO.pinMode (reset_pin, GPIO.OUTPUT);
}

uCamIII.prototype.init = function () {
  var attempts = 0;
  this._resetDevice();
  while (attempts < this.sync_attempts_max) {
    print ("Attempting to connect: " + (attempts + 1));
    if (this._attemptSync()) {
      print ("Camera is connedted!");
      DELAY.millis(2000);
      return true;
    }

    this._resetDevice();
    attempts++;
  }

  return false;
}

uCamIII.prototype._commandID = function (cmd) {
  if (Array.isArray (cmd) && cmd.length > 1) {
    return cmd[1];
  }
  throw new Error ("Ivalid usage of commandID function!");
}

uCamIII.prototype._sendCommand = function (cmd) {
  DELAY.millis(100);
  Serial.write(cmd);
  DELAY.millis(500);

  return this._waitForBytes(cmd, this.generic_ack_reply);
}

uCamIII.prototype._attemptSync = function () {
  var attempts = 0;

  while (attempts < 60) {
    Serial.flush();
    Serial.write(this.sync_command);

    DELAY.millis(5 + attempts);

    if (this._waitForBytes(this.sync_command, this.generic_ack_reply)) {
      if (this._waitForBytes(this.sync_command, this.sync_ack_reply_ext)) {
        DELAY.millis (50);
        Serial.write(this.sync_final_command);
        return true;
      }
    }
    attempts++;
  }

  return false;
}

uCamIII.prototype._waitForBytes = function (sentCommand, ackReply) {
  if (ackReply !== this.sync_ack_reply_ext) {
    ackReply[2] = this._commandID(sentCommand);
  }

  var found_bytes = 0;
  var i = 0;

  while (Serial.available() && i < this.command_length) {
    if (Serial.read() == ackReply[i] || ackReply[i] == 0) {
      found_bytes++;
    }
    i++;
  }

  return found_bytes == this.command_length;
}

uCamIII.prototype.takePicture = function () {
  if (this._sendInitial()) {
    if (this._setPackageSize()) {
      if (this._doSnapshot()) {
        if (this._getPicture()) {
          print ("The picture has been taken!");
          return true;
        }
      }
    }
  }

  return false;
}

uCamIII.prototype._sendInitial = function () {
  Serial.flush();

  return this._sendCommand(this.initial_command);
}

uCamIII.prototype._setPackageSize = function () {
  return this._sendCommand(this.pack_size);
}

uCamIII.prototype._doSnapshot = function () {
  return this._sendCommand(this.snapshot);
}

uCamIII.prototype._resetDevice = function () {
  GPIO.write(this.reset_pin, GPIO.LOW);
  DELAY.millis(900);
  GPIO.write(this.reset_pin, GPIO.HIGH);
}

uCamIII.prototype._getPicture = function () {
  var ack = [];
  if (this._sendCommand(this.get_picture)) {
    for (var i = 0; i < 6; i++) {
      while (!Serial.available());
      ack[i] = Serial.read();
    }

    var imageSize = 0;
    imageSize = (imageSize << 8) | ack[5];
    imageSize = (imageSize << 8) | ack[4];
    imageSize = (imageSize << 8) | ack[3];

    this.imageSize = imageSize;
    this.image_pos = this.imageSize;

    if (imageSize > 0){
      return true;
    }
  }
  return false;
}

uCamIII.prototype.storePicture = function (path) {
  var ack = [0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00];
  var bytes;

  if (this.image_pos == 0) {
    return false;
  }

  var image = SD.open(path, SD.appendWrite);
  var counter = 0;
  while (this.image_pos > 0) {
    if (this.image_pos < this.imageBuffer.buffer.byteLength) {
      bytes = this.image_pos + this.command_length;
    }
    else {
      bytes = this.imageBuffer.buffer.byteLength + this.command_length;
    }
    ack[4] = counter++;
    Serial.write(ack);
    DELAY.millis(45);
    var attempts = 0;
    while (Serial.available() != bytes && attempts++ < this.sync_attempts_max) {
      DELAY.millis(30);
    }

    if (attempts == this.sync_attempts_max){
      print ("Failed to store data!");
      return false;
    }

    var image_bytes = 0;
    for (var i = 0; i < bytes; i++) {
      var s = Serial.read();
      if (i >= 4 && i < bytes - 2) {
        this.imageBuffer[i - 4] = s;
        this.image_pos--;
        image_bytes++;
      }
    }
    SD.write(image, this.imageBuffer, SD.asBinary, image_bytes);
  }
  SD.close (image);

  ack[4] = 0xF0;
  ack[5] = 0xF0;
  Serial.write(ack);
  print ("The picture has been stored!");
  return true;
}
