#include "RtcDs3231.h"

#define I2C_LINE_SDA 21
#define I2C_LINE_SCL 22

bool RtcDs3231::setup()
{
    Wire.begin(I2C_LINE_SDA, I2C_LINE_SCL);
    if (!rtc.begin())
    {
        Serial.println("❌ RTC DS3231 not found, chceck connection");
        return false;
    }

    // check did RTC lost power and time is not set correctly
    if (rtc.lostPower())
    {
        Serial.println("⚠️ RTC lost power - set time as compilation time ...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    Serial.println("RTC initialized correctly ✔");
    return true;
}

DateTime RtcDs3231::now()
{
    return rtc.now();
}

void RtcDs3231::set(const DateTime &dataTime)
{
    rtc.adjust(dataTime);
}

void RtcDs3231::print(const DateTime &dataTime)
{
    Serial.print("Czas RTC: ");
    Serial.print(dataTime.year());
    Serial.print("-");
    Serial.print(dataTime.month());
    Serial.print("-");
    Serial.print(dataTime.day());
    Serial.print(" ");

    Serial.print(dataTime.hour());
    Serial.print(":");
    Serial.print(dataTime.minute());
    Serial.print(":");
    Serial.println(dataTime.second());
}