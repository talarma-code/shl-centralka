#include <Arduino.h>
#include "Application.h"
#include "HaCommunicationTask.h"
#include "DataRecorderTask.h"


HaCommunicationTask haCommunicationTask;
DataRecorderTask dataRecorderTask;
Application application(haCommunicationTask.getQueueHandle(), dataRecorderTask.recordQueue());

void setup() {
  application.setup();
  delay(500); // Give some time for setup
  haCommunicationTask.start();
  delay(500);
  dataRecorderTask.start();
}

void loop() {
  application.loop();
}
