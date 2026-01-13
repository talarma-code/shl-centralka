#include <Arduino.h>
#include "Application.h"
#include "HaCommunicationTask.h"


HaCommunicationTask haCommunicationTask;
Application application(haCommunicationTask.getQueueHandle());

void setup() {
  application.setup();
  delay(1000); // Give some time for setup
  haCommunicationTask.start();
}

void loop() {
  application.loop();
}
