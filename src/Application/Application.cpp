#include "Application.h"
#include <Arduino.h>
#include "MatterLikeDebugger.h"
#include "SystemDebuger.h"
#include "ActiveQueueRef.h"
#include <esp_system.h>
#include <sys/time.h>

#define LED_PIN 2  // wbudowana dioda LED
static const uint8_t MAC_LOCAL_HEATER[]  = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x31}; // talar1 - heater
static const uint8_t MAC_CENTRALKA[]   = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x30}; // talar0 - centrala

Application::Application(QueueHandle_t haQueueHandle, QueueHandle_t dataRecorderQueueHandle) : heaterEspNow(LED_PIN), 
    mainTaskQueue(10),
    timer(APPLICATION_SYSTEM_TIMER_ID, 5000, SystemTimerT<ApplicationMessagePacket, TimerToApplicationMessage>::Mode::OneShot, ActiveQueueRef<ApplicationMessagePacket>(mainTaskQueue.nativeHandle()), TimerToApplicationMessage()),
    haQueueRef(haQueueHandle),
    dataRecorderQueueRef(dataRecorderQueueHandle)
{

}

void Application::setup() {
    Serial.begin(115200);
    Serial.println("\n=== SHL Centralka Start ===");
    
    transport.begin(MAC_CENTRALKA, MAC_LOCAL_HEATER);
    transport.onPacketReceived(this);
    heaterEspNow.registerTransport(&transport);
    powerMeter.registerTransport(&transport);

    timer.start(25000);
    rtc.setup();
    setupSystemTime();

}

void Application::setupSystemTime() {
    DateTime now = rtc.now();

    // Ustaw systemowy czas (time(nullptr)) na podstawie RTC
    time_t epoch = (time_t)now.unixtime();
    struct timeval tv;
    tv.tv_sec = epoch;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);

    Serial.print("System time set from RTC: ");
    rtc.print(now);
}

void Application::loop() {
    ApplicationMessagePacket evt;

    if (mainTaskQueue.receive(evt, 100)) {
        switch (evt.type) {
            case ApplicationCommandType::Timer:
                Serial.println("Sent power measurement request");
                haQueueRef.send(SystemMessagePacket{
                    .type = SystemDataType::Measurements,
                    .payload = {
                        .measurementData = generateRandomMeasurement()
                    }
                });
                timer.start(5000); // restart timer
                break;

            default:
                Serial.println("Unknown timer event");
                break;
        }
    }
    
    if (Serial.available() > 0) {
        char characterRecived = Serial.read();   // Odczytaj 1 bajt

        if (characterRecived == 'o') {
            Serial.println("odebralem i wyslam ramke On");
            heaterEspNow.turnOn();
        }

        if (characterRecived == 'f') {
            Serial.println("odebralem i wyslam ramke Off");
            heaterEspNow.turnOff();
            //action Off
        }
        if (characterRecived == 'v') {
            Serial.println("odebralem i wyslam ramke v");
            powerMeter.voltage(1);
            //action Off
        }

        SystemDebuger::printSystemStats();

    }
}

//this call is from ISR context - avoid havy operations here
void Application::handlePacket(const MatterPacketWithMac &pkt) {
 Serial.println(F("=== MatterLikePacket ==="));
    MatterLikeDebugger::print(pkt);
}




// Złoty środek (praktyka)
// Typowy, sprawdzony układ:
// Task	Priority	Dlaczego
// App logic	2	reaguje na zdarzenia
// SD writer	3	opróżnia kolejkę
// WiFi / timers	20+	system
// Idle	0	system


// char buffer[32];
// DateTime(unixTime).toString(buffer, "YYYY-MM-DD hh:mm:ss");

MeasurementDataPacket Application::generateRandomMeasurement() {
    static bool seeded = false;
    if (!seeded) {
        randomSeed((uint32_t)esp_random());
        seeded = true;
    }

    MeasurementDataPacket m{};
    m.timestamp = (uint32_t)(millis() / 1000UL);

    m.L1Power = (uint32_t)random(0, 4001);
    m.L2Power = (uint32_t)random(0, 4001);
    m.HeaterPower = (uint32_t)random(0, 3001);
    m.HomeTotalPower = m.L1Power + m.L2Power + m.HeaterPower;

    m.L1Voltage_x10 = (uint16_t)random(2150, 2461);
    m.L2Voltage_x10 = (uint16_t)random(2150, 2461);

    m.HeaterEnableForSeconds = (uint16_t)random(0, 601);
    m.measurementType = MeasurementDataType::Now;

    return m;
}