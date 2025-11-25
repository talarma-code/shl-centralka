#pragma once
#include <Arduino.h>

class LineHandlerInterface {
public:
    virtual void handle(const String &line) = 0;
};
