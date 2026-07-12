#include <Arduino.h>
#include "ApplicationTask.h"
#include "HaCommunicationTask.h"
#include "DataRecorderTask.h"
#include "ActiveTask.h"
#include "esp_task_wdt.h"
#include "ActiveQueue.h"
#include "Logger.h"


ActiveQueue<ApplicationMessagePacket> mainTaskQueue(10);

HaCommunicationTask haCommunicationTask(mainTaskQueue.nativeHandle());
DataRecorderTask dataRecorderTask;
ApplicationTask application(mainTaskQueue.nativeHandle(), haCommunicationTask.getQueueHandle(), dataRecorderTask.recordQueue());

void setup() {
  

  // Enable watchdog monitoring for selected FreeRTOS tasks
  application.enableWatchdog(true);
  dataRecorderTask.enableWatchdog(true);

  // Initialize logger before any LOG_* macros are used
  Logger::instance().init(dataRecorderTask.logsQueue());

  delay(500); // Give some time for setup
  haCommunicationTask.start();
  delay(500);
  dataRecorderTask.start(); 
  delay(500);
  application.start();
  delay(500);
}

void loop() {
  vTaskDelay(portMAX_DELAY); // sleep forever, all work is done in task
}
