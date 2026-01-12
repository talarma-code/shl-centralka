#include <Arduino.h>
#include "Application.h"
#include "HaCommunicationTask.h"

Application application;
HaCommunicationTask haCommunicationTask;


void setup() {
  application.setup();
  delay(1000); // Give some time for setup
  haCommunicationTask.start();
}

void loop() {
  application.loop();
}
