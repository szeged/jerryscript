function scheduler(){
  this.taskList = [];
  this.systemTime = DELAY.systemTime();
  this.delta = 2 * 1000;
  this.actualDate;
  this.storedTime = 0;
  this.error = false;
  this.configObj;
  this.fsConfigFile = "config.json";
  this.fsRtcFile = "rtc.txt";
  this.fsTimeStampFile = "timestamp.txt";

  if (FS.exists (this.fsRtcFile)){
    this.storedTime = Number (FS.read (this.fsRtcFile));
    print ("Stored RTC: " + this.storedTime);
  }

  if (FS.exists(this.fsConfigFile)){
    this.configObj = JSON.parse (FS.read (this.fsConfigFile));
    if (FS.exists (this.fsTimeStampFile)){
      this.actualDate = new Date (Number (FS.read (this.fsTimeStampFile)));
    } else {
      this.actualDate = new Date (this.storedTime + DELAY.systemTime());
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
      this.storedTime = 1;
    }
  }
};

scheduler.prototype.addTask = function (func, config_interval) {
  if (typeof func === "function"){
    if (typeof config_interval === "number"){
      this.taskList.push({task : func, interval : config_interval});
    } else{
      throw new Error ("Argument 2 must be a number");
    }
  } else{
    throw new Error ("Argument 1 must be a function");
  }
};

scheduler.prototype._orderTasks = function () {
  this.taskList.sort((a, b) => {
    return a.interval - b.interval
  });
};

scheduler.prototype._restoreDefault = function () {
    print ("Error, restore files");
    FS.remove (this.fsRtcFile);
    FS.remove (this.fsConfigFile);
    this.error = true;
};

scheduler.prototype.removeAllTasks = function () {
  while (this.taskList.length > 0){
    this.taskList.pop();
  }

};
scheduler.prototype.resetClock = function () {
  this.storedTime = 0;
};

scheduler.prototype._currentTimestamp = function () {
  var date;
  if (WIFI.available()){
    date = new Date();
  } else {
    date = this.actualDate;
  }
  var date_str = date.toDateString() + "_" +  date.toTimeString();
  return date_str.replace(/-/g, '_').replace(/:/g, '_').replace(/\./g, '_');
};

scheduler.prototype.elapsedTime = function (callback){
  var startTime = DELAY.systemTime();
  callback.call(this);
  var endTime = DELAY.systemTime();
  return Math.floor(endTime - startTime);
}

scheduler.prototype._nextTask = function () {
  this._orderTasks();
  var taskStartTime = DELAY.systemTime();
  var timestamp = this._currentTimestamp(taskStartTime);
  var spentTime = 0;
  var deepSleepTime;
  var taskCounter = 0;
  var success = true;
  var should_start_again = false;
  for (taskCounter = 0; taskCounter < this.taskList.length; taskCounter++) {
    var remaningTime;
    if (this.storedTime == 0){
      remaningTime = this.taskList[taskCounter].interval - (taskStartTime + spentTime - this.systemTime);
      if (remaningTime > this.delta){ // Can deepSleep until next tick
        remaningTime = this.taskList[0].interval;
        spentTime = taskStartTime + spentTime - this.systemTime;
      }
    }
    else {
      var previousStartTime = (this.storedTime) % this.taskList[taskCounter].interval; // Next task could be start at the begining
      var actualStartTime = (this.storedTime + spentTime) % this.taskList[taskCounter].interval; // Next task can be executed now
      remaningTime = Math.min(previousStartTime, actualStartTime);
      if (remaningTime > this.delta){ // Can deepSleep until next tick
        remaningTime = this.taskList[0].interval;
      }
      print ("remaningTime: ", remaningTime);
    }

    if (remaningTime <= this.delta ){
      if (remaningTime > 0){
        print ("Going to normal sleep");
        spentTime += remaningTime;
        DELAY.millis(remaningTime);
      }
      spentTime += this.elapsedTime(function () {
        try {
          this.taskList[taskCounter].task(timestamp);
        } catch (e) {
          print (e);
          success = false;
        }
      });
    }
    else {
      if (spentTime > this.taskList[0].interval){
        /* Too much time spent in the previous task, should start it again
           but first check whether the next tasks can be executed */
        should_start_again = true;
        continue;
      }

      if (should_start_again){
        taskCounter = 0;
        spentTime = 0;
        taskStartTime = DELAY.systemTime();
        timestamp = this._currentTimestamp(taskStartTime);
        this.storedTime += this.taskList[0].interval;
        should_start_again = false;
        continue;
      }

      print ("Going to deepSleep");
      this.storedTime += spentTime;
      deepSleepTime = Math.max(remaningTime - spentTime, 0);
      break;
    }
    if (!success){
      break;
    }
  }
  configFileBeenStored = false;

  return {
    nextDeepSleepTime : this.taskList.length == taskCounter ? 0 : deepSleepTime,
    taskSuccess : success
  }
};

scheduler.prototype.success = function(time) {
  return time >= 1;
}

scheduler.prototype.nextTask = function() {
  var result;
  var schedulerSpentTime = this.elapsedTime(function (){
    result = this._nextTask();
    result.nextDeepSleepTime -= this.elapsedTime (function () {
      if (!FS.write(this.fsConfigFile, JSON.stringify (this.configObj))){
        this._restoreDefault();
      }
    });

    if (!result.taskSuccess) {
      this._restoreDefault();
    }

    result.nextDeepSleepTime -= this.elapsedTime(function () {
      var rtcTime = Math.round((Math.max(result.nextDeepSleepTime, 0) + this.storedTime) / 1000) * 1000;
      if (!FS.write(this.fsRtcFile, rtcTime)){
        this._restoreDefault();
      }
    });
  });

  if (!FS.write(this.fsTimeStampFile, this.actualDate.getTime() + schedulerSpentTime + 26 + Math.max (result.nextDeepSleepTime, 0))){
    this._restoreDefault();
  }

  return this.error ? 1 : Math.max (result.nextDeepSleepTime, 0);
}
