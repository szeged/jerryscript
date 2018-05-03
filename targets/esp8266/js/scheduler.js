function scheduler(){
  this.taskList = [];
  this.hasTaskList = false;
  this.taskCounter = 0;
  this.delta = 2 * 1000;
  this.actualDate = new Date (DELAY.systemTime());
  this.error = false;
  this.configObj;
  this.fsConfigFile = "config.json";
  this.fsTaskFile = "tasks.json";
  this.fsTimeStampFile = "timestamp.txt";

  if (FS.exists(this.fsTaskFile)){
    this.taskList = JSON.parse (FS.read (this.fsTaskFile));
    this.hasTaskList = true;
  }

  this.firstTime = this.actualDate.getTime() + DELAY.systemTime();

  if (FS.exists(this.fsConfigFile)){
    this.configObj = JSON.parse (FS.read (this.fsConfigFile));
    if (FS.exists (this.fsTimeStampFile)){
      this.actualDate = new Date (Number (FS.read (this.fsTimeStampFile)));
    } else {
      this.actualDate = new Date (DELAY.systemTime());
    }
    print ("TimeStamp " + this.actualDate.getTime());
  } else {
    this.configObj = { get_interval : 1000 * 60 * 5, // 5 min
                  pic_interval : 1000 * 30, // 30 sec
                  measure_interval : 1000 * 30, // 30 sec
                  data_send_interval : 1000 * 60 * 10, // 10 min
                  delete_after_send : false
                };
    this.actualDate = new Date (DELAY.systemTime());
    this.firstTime = this.actualDate.getTime() + DELAY.systemTime();
    configUpdateTask(undefined, this);
  }
};

scheduler.prototype.addTask = function (func, config_interval) {
  if (this.hasTaskList) {
    this.taskList[this.taskCounter++].task = func;
  } else {
    if (typeof func === "function"){
      if (typeof config_interval === "number"){
        this.taskList.push({task: func, interval: config_interval, lastTime: 0});
      } else{
        throw new Error ("Argument 2 must be a number");
      }
    } else{
      throw new Error ("Argument 1 must be a function");
    }
  }
};

scheduler.prototype._orderTasks = function () {
  this.taskList.sort(function (a, b) {
    return a.interval - b.interval
  });
};

scheduler.prototype._restoreDefault = function () {
    print ("Error, restore files");
    FS.remove (this.fsConfigFile);
    FS.remove (this.fsTaskFile);
    DELAY.deepSleep (1);
};

scheduler.prototype._currentTimestamp = function () {
  var date;
  if (WIFI.available()){
    date = new Date();
    this.actualDate = date;
  } else {
    date = this.actualDate;
  }
  var date_str = date.toDateString() + "_" +  date.toTimeString();
  return date_str.replace(/-/g, '_').replace(/:/g, '_').replace(/\./g, '_');
};

scheduler.prototype._currentTime = function () {
  return this.actualDate.getTime() + DELAY.systemTime() - this.firstTime;
};

scheduler.prototype.elapsedTime = function (callback){
  var startTime = DELAY.systemTime();
  callback.call(this);
  var endTime = DELAY.systemTime();
  return Math.floor(endTime - startTime);
}

scheduler.prototype.nextTask = function() {
  var lastDate = this._currentTime();
  if (!this.hasTaskList){
    for (var i = 0; i < this.taskList.length; i++){
      this.taskList[i].lastTime = lastDate;
    }
  }
  var deepSleepTime;
  var taskQueue = [];
  while (true) {
    // 1 second has passed (at least)
    var currentDate = this._currentTime();
    var nextTaskTime = this.taskList[0].interval - (currentDate - this.taskList[0].lastTime);
    if (currentDate - lastDate >= 1000) {
      for (var i = 0; i < this.taskList.length; i++) {
        var elapsed = currentDate - this.taskList[i].lastTime;
        if (elapsed >= this.taskList[i].interval) {
          taskQueue.push(this.taskList[i]);
          this.taskList[i].lastTime = this._currentTime();
        } else {
          nextTaskTime = Math.min(this.taskList[i].interval - elapsed, nextTaskTime);
        }
      }
      if (nextTaskTime >= this.delta) {
        if (taskQueue.length == 0){
          print('Deepsleep for ', nextTaskTime);
          deepSleepTime = nextTaskTime;
          break;
        }
      } else {
        DELAY.millis (nextTaskTime);
      }

      var executionTime = DELAY.systemTime();
      if (taskQueue.length > 0) {
        taskQueue.sort(function(a, b) {
          return a.lastTime - b.lastTime || b.interval < a.interval;
        });
        for (var i = 0; i < taskQueue.length; i++) {
          try {
            // Execute the stored task
            taskQueue[i].task(this._currentTimestamp ());
          } catch (e) {
            // The current task has throw an Error
            this.error = true;
            break;
          }
        }
      }
      if (this.error) {
        break;
      }

      executionTime = DELAY.systemTime() - executionTime;

      var overrunTime = Math.max (nextTaskTime - executionTime, 0);
      if (overrunTime >= this.delta) {
        print('Deepsleep for ', overrunTime);
        deepSleepTime = overrunTime;
        break;
      } else {
        DELAY.millis (overrunTime);
      }

      taskQueue = [];
      lastDate = currentDate;
    }
  }

  deepSleepTime -= this.elapsedTime (function () {
    if (!FS.write(this.fsConfigFile, JSON.stringify (this.configObj))){
      this._restoreDefault();
    }
  });

  deepSleepTime -= this.elapsedTime (function () {
    if (!FS.write(this.fsTaskFile, JSON.stringify (this.taskList))){
      this._restoreDefault();
    }
  });

  if (!FS.write(this.fsTimeStampFile, this._currentTime() + 26 + Math.max (deepSleepTime, 0))){
    this._restoreDefault();
  }

  if (this.error) {
    this._restoreDefault();
  }

  if (deepSleepTime){
    DELAY.deepSleep(deepSleepTime);
  }
}
