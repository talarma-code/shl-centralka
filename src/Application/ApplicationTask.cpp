#include "ApplicationTask.h"
#include <Arduino.h>
#include "MatterLikeDebugger.h"
#include "SystemDebuger.h"
#include "ActiveQueueRef.h"
#include "Log.h"
#include <esp_system.h>
#include <sys/time.h>

#define LED_PIN 2  // wbudowana dioda LED
static const uint8_t MAC_LOCAL_HEATER[]  = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x31}; // talar1 - heater
static const uint8_t MAC_CENTRALKA[]   = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x30}; // talar0 - centrala

ApplicationTask::ApplicationTask(QueueHandle_t haQueueHandle, QueueHandle_t dataRecorderQueueHandle) : 
    ActiveTask("application", 8192, 1),
    heaterEspNow(LED_PIN), 
    heaterFsm(heaterEspNow),
    mainTaskQueue(10),
    timer(APPLICATION_SYSTEM_TIMER_ID, 5000, SystemTimerT<ApplicationMessagePacket, TimerToApplicationMessage>::Mode::OneShot, ActiveQueueRef<ApplicationMessagePacket>(mainTaskQueue.nativeHandle()), TimerToApplicationMessage()),
    haQueueRef(haQueueHandle),
    dataRecorderQueueRef(dataRecorderQueueHandle)
{

}

void ApplicationTask::setup() {
    Serial.begin(115200);
    Serial.println("\n=== SHL Centralka Start ===");
    
    transport.begin(MAC_CENTRALKA, MAC_LOCAL_HEATER);
    transport.onPacketReceived(this);
    heaterEspNow.registerTransport(&transport);
    powerMeter.registerTransport(&transport);

    // Setup physical OR-WE-504 Modbus meter
    //orwe504Meter.setup();

    timer.start(25000);
    rtc.setup();
    setupSystemTime();

}

void ApplicationTask::setupSystemTime() {
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

void ApplicationTask::loop() {
    ApplicationMessagePacket evt;

    if (mainTaskQueue.receive(evt, 100)) {
        switch (evt.type) {
            case ApplicationCommandType::Timer:
            {
                MeasurementDataPacket data = generateRandomMeasurement();
                Serial.println("Sent power measurement request");
                sendMeasurementToHa(data);

                SystemMessagePacket drMsg;
                drMsg.type = SystemDataType::Measurements;
                drMsg.payload.measurementData = data;
                dataRecorderQueueRef.send(drMsg);
                timer.start(5000); // restart timer
                break;
            }

            case ApplicationCommandType::MatterPacket: {
                auto res = heaterFsm.step(evt.payload.matterPacket);
                if (res.hasCommand) {
                    // For now we just have the HeaterCommandPacket ready
                    // for future processing or forwarding.
                    HeaterCommandPacket cmd = res.command;
                    (void)cmd; // suppress unused warning until used
                }
                break;
            }

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

void ApplicationTask::sendMeasurementToHa(const MeasurementDataPacket& data) {
    SystemMessagePacket haMsg;
    haMsg.type = SystemDataType::Measurements;
    haMsg.payload.measurementData = data;
    if(!haQueueRef.send(haMsg, 50)) {           
        haQueueFullStreak++;
        LOG_ERROR("HaQueue full, streak: %u", haQueueFullStreak);

        if (haQueueFullStreak >= 3) {        
            esp_restart();
        }
    } else {
        haQueueFullStreak = 0;       
    }
}

//this call is from ISR context - avoid havy operations here
void ApplicationTask::handlePacket(const MatterPacketWithMac &pkt) {
    ApplicationMessagePacket msg{};
    msg.type = ApplicationCommandType::MatterPacket;
    msg.payload.matterPacket = pkt;
    mainTaskQueue.sendFromISR(msg, nullptr);
}


void ApplicationTask::measure()
{
             const uint8_t slaveId = 1; // Modbus address of OR-WE-504

                float voltage      = orwe504Meter.voltage(slaveId);
                float current      = orwe504Meter.electricCurrent(slaveId);
                float freq         = orwe504Meter.frequency(slaveId);
                float pActive      = orwe504Meter.activePower(slaveId);
                float pReactive    = orwe504Meter.reactivePower(slaveId);
                float pApparent    = orwe504Meter.apparentPower(slaveId);
                float powerFactor  = orwe504Meter.powerFactor(slaveId);
                float energyActive = orwe504Meter.totalActivePower(slaveId);

                Serial.println("[OR-WE-504] Measurements:");
                Serial.print("  Voltage [V]:         "); Serial.println(voltage, 2);
                Serial.print("  Current [A]:         "); Serial.println(current, 3);
                Serial.print("  Frequency [Hz]:      "); Serial.println(freq, 2);
                Serial.print("  Active Power [W]:    "); Serial.println(pActive, 2);
                Serial.print("  Reactive Power [var]:"); Serial.println(pReactive, 2);
                Serial.print("  Apparent Power [VA]: "); Serial.println(pApparent, 2);
                Serial.print("  Power factor [-]:    "); Serial.println(powerFactor, 3);
                Serial.print("  Active energy [kWh]: "); Serial.println(energyActive, 3);

                // Opcjonalnie: restart timera, aby cyklicznie odświeżać pomiary
                timer.start(5000);
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

MeasurementDataPacket ApplicationTask::generateRandomMeasurement() {
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