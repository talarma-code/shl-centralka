#pragma once
#include "heater/HeaterEspNow.h"

class Application {
public:
    Application();        // konstruktor
    void setup();         // setup Arduino
    void loop();          // loop Arduino

private:
    HeaterEspNow heater;
};