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
  - Perfroms all of the previously set tasks that can be executed continuously. If any kind of error occurs during the task execution the the scheduler resets itself to the default configuration.

  - **NOTE:** The function can send the board to deep-sleep mode!


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

while (true)
{
  espScheduler.nextTask();
}

```
