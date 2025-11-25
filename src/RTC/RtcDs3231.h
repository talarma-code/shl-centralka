#pragma onec

#include <Wire.h>
#include <Arduino.h>
#include "RTClib.h"

class RtcDs3231
{

public:
    bool setup();
    DateTime now();
    void set(const DateTime &dataTime);
    void print(const DateTime &dataTime);

private:
    RTC_DS3231 rtc;
};
