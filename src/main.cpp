#include <Arduino.h>
#include "Application.h"

Application application;


void setup() {
  application.setup();
}

void loop() {
  application.loop();
}
