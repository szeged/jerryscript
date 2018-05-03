# Scheduler

This module stands for order and execute several task while attempts to deep sleep as much as possible for saving energy.

## Constructor
### Scheduler()
  Creates a new `sensorCollection` object.

**Example**

```js
var espScheduler = new Scheduler();
```

## Methods
### Scheduler.addTask(task, interval)
  - `task` {fucntion} - Task to be called.
  - `interval` {integer} - Task call interval in millisec.
  - Returns: {undefined}

  Adds a new task to scheduler task list.

**Example**

```js
function mytask() {
  //do some stuff
}

function mytask2() {
  //do some stuff
}

var espScheduler = new Scheduler(0);
espScheduler.addTask(mytask, 3600);
espScheduler.addTask(mytask2, 45000);
```

### Scheduler.removeAllTasks()
  - Returns: {undefined}

  Removes all tasks from task list.

**Example**

```js
function mytask() {
  //do some stuff
}

function mytask2() {
  //do some stuff
}

var espScheduler = new Scheduler(0);
espScheduler.addTask(mytask, 3600);
espScheduler.addTask(mytask2, 45000);
espScheduler.removeAllTasks();
espScheduler.addTask(mytask2, 45000);
```

### Scheduler.resetClock()
  - Returns: {undefined}

  Restarts the scheduler own clock by setting it zero.

**Example**

```js
function mytask() {
  //do some stuff
}

var espScheduler = new Scheduler(0);
espScheduler.addTask(mytask, 3600);
espScheduler.removeAllTasks();
espScheduler.resetClock();
```

### Scheduler.elapsedTime(callback)
  - `callback` {fucntion} - Callback function.
  - Returns: {integer} - `callback` funtion run time in milliseconds.

  Measures the `callback` function run time.

**Example**

```js
function mytask() {
  //do some stuff
}

var espScheduler = new Scheduler(0);
var time = espScheduler.elapsedTime(mytask);
```

### Scheduler.nextTask()
  - Returns: {object} - operation call result.

  Perfroms all of the previously set tasks that can be executed continuously. The operation returns an object that contains an {integer} which represents the available time that can be spent in deep sleep state until the next task should be executed and a {boolean} value which is `true` if there was no error during the task(s) execution.


**Example**

```js
function mytask() {
  //do some stuff
}

function mytask2() {
  //do some stuff
}

function mytask3() {
  //do some stuff
}

var espScheduler = new Scheduler(0);
espScheduler.addTask(mytask, 3600);
espScheduler.addTask(mytask2, 4000);
espScheduler.addTask(mytask, 560000);
var result = espScheduler.nextTask();
if (result.taskSuccess){
  // There was no error
  DELAY.deepSleep(result.nextDeepSleepTime); // deepSleep for (3600 - operation time) ms
} else {
  // Handle error
}

```
