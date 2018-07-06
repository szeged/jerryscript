function scheduler(){
  this.taskList = [];
  this.taskCounter = 0;
  this.delta = 2 * 1000;
  this.actualDate;
  this.error = false;
  this.configObj;
  this.fsConfigFile = "config.json";
  this.fsTaskFile = "tasks.json";
  this.fsTimeStampFile = "timestamp.txt";

  if (FS.exists(this.fsConfigFile)){
    this.configObj = JSON.parse (FS.read (this.fsConfigFile));
    if (FS.exists (this.fsTimeStampFile)){
      this.actualDate = new Date (Number (FS.read (this.fsTimeStampFile)));
    } else {
      this.actualDate = new Date (DELAY.systemTime());
    }
    print ("TimeStamp " + this.actualDate.getTime());
  } else {
    configUpdateTask(undefined, this);
    if (this.configObj == undefined){
      this.configObj = { get_interval : 1000 * 60 * 5, // 5 min
                    pic_interval : 1000 * 30, // 30 sec
                    measure_interval : 1000 * 30, // 30 sec
                    data_send_interval : 1000 * 1 * 10, // 10 min
                    delete_after_send : false
                  };
    }
  }

  this.firstTime = this.actualDate + DELAY.systemTime();

  if (FS.exists(this.fsTaskFile)){
    this.taskList = JSON.parse (FS.read (this.fsTaskFile));
  }
};

scheduler.prototype.addTask = function (func, config_interval) {
  if (this.taskList.length) {
    this.taskList[this.taskCounter++].task = func;
  } else {
    if (typeof func === "function"){
      if (typeof config_interval === "number"){
        this.taskList.push({task: func, interval: config_interval, lastTime: this.firstTime});
      } else{
        throw new Error ("Argument 2 must be a number");
      }
    } else{
      throw new Error ("Argument 1 must be a function");
    }
  }
};

scheduler.prototype._orderTasks = function () {
  this.taskList.sort((a, b) => {
    return a.interval - b.interval
  });
};

scheduler.prototype._restoreDefault = function () {
    print ("Error, restore files");
    FS.remove (this.fsConfigFile);
    FS.remove (this.fsTaskFile);
    DELAY.deepSleep (1);
};

scheduler.prototype.removeAllTasks = function () {
  while (this.taskList.length > 0){
    this.taskList.pop();
  }
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
        print('Deepsleep for ', nextTaskTime);
        deepSleepTime = nextTaskTime;
        break;
      }

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

        if (this.error) {
          break;
        }
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
