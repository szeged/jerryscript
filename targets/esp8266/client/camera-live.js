var net = require('net');
var fs = require('fs');

class ImageProcesser {
  constructor() {
    this._imageId = 0;
    this._imageSize = 0;
    this._imageBuffer = null;
    this._imageBufferData = [];
    this._onGoingDataSize = 0;
    this._startDate = null;
  }

  set imageSize(data) {
    this._imageSize = parseInt(data.toString(), 16);
    if (this._startDate === null) {
      this._startDate = new Date();
    }
    console.log (this._imageSize);
  }

  set imageBuffer(data) {
    if (this._imageSize == 0) {
      throw Error('Missing imageSize');
    }
    this._imageBufferData[0] = data;

    if (data.length == this._imageSize) {
      this.processImage();
    } else {
      this._onGoingDataSize = data.length;
      console.log ("first image data: " + data.length);
    }
  }

  get onGoingDataSend() {
    return this._onGoingDataSize != 0;
  }

  appendData(buff) {
    console.log ("image data: " + buff.length);
    this._onGoingDataSize += buff.length;
    this._imageBufferData.push(buff);

    if (this._imageSize == this._onGoingDataSize) {
      this.processImage();
    }

    if (this._onGoingDataSize > this._imageSize) {
      console.log('err');
      this.reset();
    }
  }

  idString(size) {
    var str = String(this._imageId++);
    while (str.length < (size || 2)) {
      str = "0" + str;
    }

    var dirName = './live_pictures/' + this._startDate.toISOString().replace(/[\:\.\-\T]/g, "_").slice(0, -5);

    if (!fs.existsSync(dirName)) {
      fs.mkdirSync(dirName, { resursive : true});
    }

    return dirName + '/pic_' + str + '.jpg';
  }

  processImage() {
    console.log("proc image")
    this._imageBuffer = Buffer.concat (this._imageBufferData, this._imageSize);
    console.log (this._imageSize, this._imageBuffer);
    var wstream = fs.createWriteStream(this.idString(4), {});
    wstream.write(this._imageBuffer);
    wstream.end();
    this.reset();
  }

  reset() {
    this._imageSize = 0;
    this._imageBuffer = null;
    this._imageBufferData = [];
    this._onGoingDataSize = 0;
  }

  resetDate() {
    this._startDate = null;
  }
}

var server = net.createServer(function(socket) {
  var imageProcesser = new ImageProcesser();
  socket.on('data', function(data) {
    parseData(imageProcesser, data, socket);
  });

  socket.on('error', function(err) {
    console.log('[ERROR] ' + err + ' on client_ID: ' + socket.id);
    socket.end();
  });
}).listen(5010);

const messageType = {
  imageSize : 0,
  imageBuffer : 1,
  closeConnection : 2,
}

function parseData(imageProcesser, data, socket) {

  if (imageProcesser.onGoingDataSend) {
    imageProcesser.appendData(data);
    return;
  }

  switch (data[0]) {
    case messageType.imageSize: {
      imageProcesser.imageSize = data.slice(1);
      break;
    }
    case messageType.closeConnection: {
      socket.end();
      console.log ('socket closed');
      imageProcesser.resetDate();
      break;
    }
    case messageType.imageBuffer: {
      imageProcesser.imageBuffer = data.slice(1);
      break;
    }
    default: {
      throw Error ('Unexpected message');
    }
  }
}
