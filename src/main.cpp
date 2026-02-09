#include <Arduino.h>
#include "ApplicationTask.h"
#include "HaCommunicationTask.h"
#include "DataRecorderTask.h"
#include "ActiveTask.h"
#include "esp_task_wdt.h"


HaCommunicationTask haCommunicationTask;
DataRecorderTask dataRecorderTask;
ApplicationTask application(haCommunicationTask.getQueueHandle(), dataRecorderTask.recordQueue());

void setup() {

  // Enable watchdog monitoring for selected FreeRTOS tasks
  application.enableWatchdog(true);
  dataRecorderTask.enableWatchdog(true);

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
