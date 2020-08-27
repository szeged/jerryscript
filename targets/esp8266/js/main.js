var tempSensor = new max6675(0, 4, 5, "C");
var uCam = new uCamIII(5);
var sensors = new sensorCollection();
var configFileBeenStored = false;
var sdFileNames = ["config_", "picture_", "sensor_data_"];
var defaultConfig = false;
var arducamInited = false;
var serverIp = "10.109.169.1";
var arducamImageSizes = [
  ArduCAM.EMPTY,
  ArduCAM.IMG_SIZE_320x240,
  ArduCAM.IMG_SIZE_640x480,
  ArduCAM.IMG_SIZE_1024x768,
  ArduCAM.IMG_SIZE_1280x960,
  ArduCAM.IMG_SIZE_1600x1200,
  ArduCAM.IMG_SIZE_1920x1080,
  ArduCAM.IMG_SIZE_2048x1536,
  ArduCAM.IMG_SIZE_2592x1944
];

var espScheduler = new scheduler();
sensors.addSensor(tempSensor);

/* Helper functions */

function initWifiForTask(callback){
  if (!WIFI.available()){
    try {
      WIFI.connect ("ESP8266", "Barackospite");
    } catch (e) {
      print (e);
      return;
    }
  }
  try {
    callback();
  } catch (e) {
    print (e);
  }
  WIFI.disconnect();
}

function storeConfigFile(directoryName){
  var file_name = directoryName +  "_u/" + directoryName + "_config_u.json";
  var file = SD.open(file_name, SD.createEmptyForWrite);
  SD.write (file, JSON.stringify(espScheduler.configObj), SD.asText);
  SD.close (file);
  configFileBeenStored = true;
}

/* Tasks */

function sensorsTask(directoryName) {
  print ("Sensor task Start");
  SD.mount(15);
  if (!configFileBeenStored){
    SD.mkdir (directoryName + "_u");
  }
  var file_name = directoryName + "_u/" + directoryName + "_sensor_u.txt";
  sensors.storeAllSensorsData(file_name);
  if (!configFileBeenStored){
    storeConfigFile(directoryName);
  } else {
    configFileBeenStored = false;
  }
  SD.unmount();
}

function cameraTask(directoryName) {
  print ("Camera task Start");
  SD.mount(15);
  if (!configFileBeenStored){
    SD.mkdir (directoryName + "_u");
  }
  var image_name = directoryName + "_u/" + directoryName + "_pic_u.jpg";
  if (uCam.init()) {
    if (uCam.takePicture()){
      uCam.storePicture(image_name);
    } else {
      print ("Cannot take picture with uCam-iii!");
    }
  } else {
    print ("Cannot sync with uCam-iii!");
  }

  if (!configFileBeenStored) {
    storeConfigFile(directoryName);
  } else {
    configFileBeenStored = false;
  }

  SD.unmount();
}

function arducamTask (directoryName, jsref) {
  print ("ArduCAM task Start");
  var ref = jsref || espScheduler;
  SD.mount (ArduCAM.SD_CS);
  // if (!configFileBeenStored) {
  //   SD.mkdir(directoryName + "_u");
  // }
  SD.mkdir(directoryName + "_u");
  var image_name = directoryName + "_u/" + directoryName + "_pic_u.jpg";

  try {
    ArduCAM.init ();
    ArduCAM.setImageSize (arducamImageSizes[ref.configObj.image_size]);
    ArduCAM.capture ();
    var fd = SD.open (image_name, SD.createEmptyForWrite);
    ArduCAM.store (fd);
    SD.close (fd);
  } catch (e) {
    print("An error occurred in ArduCAM task: ");
    print(e.toString())
  }

  // if (!configFileBeenStored) {
  //   storeConfigFile(directoryName);
  // } else {
  //   configFileBeenStored = false;
  // }
  storeConfigFile(directoryName);

  SD.unmount();
}

function live(sec, jsref) {
  var ref = jsref || espScheduler;
  if (ref.configObj.camera_type == "ArduCAM") {
    try {
      ArduCAM.setImageSize (arducamImageSizes[ref.configObj.image_size]);
    } catch (e) {
      print (e)
    }
  }

  if (!FS.write("live.txt", sec.toString())){
    throw Error ("Cannot start live: could not write live.txt");
  }

  if (!FS.write("cam.txt", ref.configObj.camera_type.toString())) {
    throw Error ("Cannot start live: could not write cam.txt");
  }

  DELAY.deepSleep(10);
}

function configUpdateTask(timestamp, jsref) {
  print ("GET task Start");
  initWifiForTask(function () {
    var str = WIFI.receive (serverIp, 8000, "/config.json");
    var ref = jsref || espScheduler;
    ref.configObj = JSON.parse (str);

    print(JSON.stringify(ref.configObj))

    if (ref.configObj.camera_live_interval != 0) {
      FS.write("timestamp.txt", (new Date()).getTime() + 26 + ref.configObj.camera_live_interval);
      FS.remove("config.json");
      FS.remove("tasks.json");
      WIFI.post (serverIp, 8001, "camera_live_interval=0&camera_live_interval_unit=hour");
      live(ref.configObj.camera_live_interval / 1000, ref);
    }
    initTasks(jsref);
    ref.actualDate = new Date();
  });
}

function finalizeFileOrDirectory (path, isFile){
  if (espScheduler.configObj.delete_after_send){
    SD.delete(path);
  } else {
    if (isFile){
      newPath = path.replace(/(_u)(\..*)$/,"$2");
      SD.rename(path, newPath);
    } else {
      newPath = path.replace(/_u/g, "");
      SD.rename(path, newPath);
    }
  }
}

function sendFilesInDirectory(directory){
  var i = 0;
  var counter = 0;
  var extensionArray = [".txt", ".jpg", ".json"];
  for (i = 0; i < extensionArray.length; i++) {
    var path = SD.find (directory, "*_u" + extensionArray[i]);
    while (path != undefined)
    {
      var absPath = directory + "/" + path;
      print ("Sending: " + absPath);
      var file = SD.open (absPath, SD.readonly);
      var succeeded = false;

      try {
        succeeded = WIFI.send (serverIp, 5002, file, file.openPath, SD.fileSize (file));
      } catch (e) {
        print (e);
        print ("File was not synced!");
      }

      SD.close (file);
      if (succeeded){
        counter++;
        finalizeFileOrDirectory (absPath, true);
      }
      path = SD.findNext();
    }
  }
  return i == counter;
}

function customVisitFolderStructure (path){
  var directory = SD.openDirectory (path);
  var directoryStructure = SD.readDirectoryStructure(directory);
  while (directoryStructure != undefined){
    var subDirectoryPath = undefined;
    if (directoryStructure.elementName.charAt(0) != "." && /* Valid element*/
        SD.isDirectory (directoryStructure) && /* Valid folder */
        directoryStructure.elementName.indexOf("_u") != -1 /* Unsynced */){
          subDirectoryPath = (path == "/" ? "" : path) + "/" + directoryStructure.elementName;
           if (!sendFilesInDirectory(subDirectoryPath)){
             subDirectoryPath = undefined;
           }
    }

    directoryStructure = SD.readDirectoryStructure(directory);
    if (subDirectoryPath != undefined){
      finalizeFileOrDirectory(subDirectoryPath, false);
    }
  }
}

function sendDataTask(timestamp, jsref){
  print ("SEND task Start");
  var ref = jsref || espScheduler;
  SD.mount(ref.configObj.camera_type == "ArduCAM" ? ArduCAM.SD_CS : 15);
  initWifiForTask(function (timestamp){
    customVisitFolderStructure ("/");
  });
  SD.unmount();
}

function initTasks (jsref){
  if (jsref) {
    defaultConfig = true;
  }
  var ref = jsref || espScheduler;

  switch (ref.configObj.camera_type) {
    case "uCam-III": {
      ref.addTask(sensorsTask, ref.configObj.measure_interval);
      ref.addTask(cameraTask, ref.configObj.pic_interval);
      ref.addTask(configUpdateTask, ref.configObj.get_interval);
      ref.addTask(sendDataTask, ref.configObj.data_send_interval);
      break;
    }
    case "ArduCAM": {
      ref.addTask(arducamTask, ref.configObj.pic_interval);
      ref.addTask(configUpdateTask, ref.configObj.get_interval);
      break;
    }
    default: {
      print("INVALID CAMERA TYPE");
      while (true) {}
    }
  }
}

if (!defaultConfig){
  initTasks ();
}

function sysloop(ticknow) {
  espScheduler.nextTask();
};
