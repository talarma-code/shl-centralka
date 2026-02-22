#include "ApplicationTask.h"
#include <Arduino.h>
#include "MatterLikeDebugger.h"
#include "SystemDebuger.h"
#include "ActiveQueueRef.h"
#include "Log.h"
#include <esp_system.h>
#include <sys/time.h>
#include <time.h>

#define LED_PIN 2  // wbudowana dioda LED
static const uint8_t MAC_LOCAL_HEATER[]  = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x31}; // talar1 - heater
static const uint8_t MAC_CENTRALKA[]   = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x30}; // talar0 - centrala

ApplicationTask::ApplicationTask(QueueHandle_t mainTaskQueueHandle, QueueHandle_t haQueueHandle, QueueHandle_t dataRecorderQueueHandle) : 
    ActiveTask("application", 8192, 1),
    heaterEspNow(LED_PIN), 
    heaterFsm(heaterEspNow),
    mainTaskQueue(mainTaskQueueHandle),
    haQueueRef(haQueueHandle),
    dataRecorderQueueRef(dataRecorderQueueHandle),
    timer(APPLICATION_SYSTEM_TIMER_ID, 5000, SystemTimerT<ApplicationMessagePacket, TimerToApplicationMessage>::Mode::OneShot, ActiveQueueRef<ApplicationMessagePacket>(mainTaskQueue.nativeHandle()), TimerToApplicationMessage())
{

}

void ApplicationTask::setup() {
    Serial.begin(115200);
    Serial.println("\n=== SHL Centralka Start ===");

    logLastResetReason();
    
    transport.begin(MAC_CENTRALKA, MAC_LOCAL_HEATER);
    transport.onPacketReceived(this);
    heaterEspNow.registerTransport(&transport);
    powerMeter.registerTransport(&transport);

    // Setup physical OR-WE-504 Modbus meter
    orwe520PowerMeter.setup();
    sdm120ctPowerMeter.setup();

    timer.start(25000);
    rtc.setup();
    setupSystemTime();

}

void ApplicationTask::logLastResetReason() {
    esp_reset_reason_t reason = esp_reset_reason();
    const char* reasonStr = "UNKNOWN";

    switch (reason) {
        case ESP_RST_POWERON:    reasonStr = "POWERON"; break;
        case ESP_RST_EXT:        reasonStr = "EXTERNAL"; break;
        case ESP_RST_SW:         reasonStr = "SW"; break;
        case ESP_RST_PANIC:      reasonStr = "PANIC"; break;
        case ESP_RST_INT_WDT:    reasonStr = "INT_WDT"; break;
        case ESP_RST_TASK_WDT:   reasonStr = "TASK_WDT"; break;
        case ESP_RST_WDT:        reasonStr = "OTHER_WDT"; break;
        case ESP_RST_DEEPSLEEP:  reasonStr = "DEEPSLEEP"; break;
        case ESP_RST_BROWNOUT:   reasonStr = "BROWNOUT"; break;
        case ESP_RST_SDIO:       reasonStr = "SDIO"; break;
        default:                 reasonStr = "UNKNOWN"; break;
    }

    LOG_ERROR("Last reset reason: %s (%d)", reasonStr, static_cast<int>(reason));
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

bool ApplicationTask::is23Pm() {
    time_t now = time(nullptr);
    if (now == (time_t)-1) {
        return false;
    }

    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_hour == 23 && timeinfo.tm_min < 3) {
        if (last23PmYear != timeinfo.tm_year || last23PmDayOfYear != timeinfo.tm_yday) {
            last23PmYear = timeinfo.tm_year;
            last23PmDayOfYear = timeinfo.tm_yday;
            return true;
        }
    }

    return false;
}

void ApplicationTask::loop() {
    ApplicationMessagePacket evt;

    if (mainTaskQueue.receive(evt, 100)) {
        switch (_state)
        {
        case state::Idle:
            handleIdleState(evt);
            break;
        case state::Measurements:
            handleMeasurementsState(evt);
            break;
        case state::HeaterControl:
            handleHeaterControlState(evt);
            break;
        case state::HaNotification:
            handleHaNotificationState(evt);
            break;
        case state::RtcSync:
            handleRtcSyncState(evt);
            break;
        case state::HistoricalDataSync:
            handleHistoricalDataSyncState(evt);
            break;
        default:
            break;
        }






        switch (evt.type) {
            case ApplicationCommandType::Timer:
            {
                MeasurementDataPacket data = generateRandomMeasurement();
                //Serial.println("Sent power measurement request");
                sendMeasurementToHa(data);

                orwe520PowerMeter.update(); // Update pulse count and power calculation 
                Serial.println("--------------------------------------------");
                Serial.print("ORWE520 Current Power [kW]: ");
                Serial.println(orwe520PowerMeter.currentPowerKW());
                Serial.print("ORWE520 Total Energy [kWh]: ");
                Serial.println(orwe520PowerMeter.totalEnergyKWh());
                Serial.print("ORWE520 Total Pulses: ");
                Serial.println(orwe520PowerMeter.totalPulses());

                Serial.println("======================================");
                Serial.print("SDM120CT Voltage [V]: ");
                Serial.println(sdm120ctPowerMeter.voltage(1));

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
            
            case ApplicationCommandType::RtcSync: {
                uint32_t epochTime = evt.payload.rtcSyncCommandPacket.epochTime;
                DateTime dt(epochTime);
                rtc.set(dt);
                Serial.print("RTC synchronized to epoch time: ");
                rtc.print(dt);
                break;
            }
            default:
                Serial.println("Unknown timer event");
                break;
        }
    }

    
    //This is only test code - will be removed 
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
        if (characterRecived == 'r') {
            SystemMessagePacket drMsg;
            drMsg.type = SystemDataType::RtcSync;
            haQueueRef.send(drMsg);
        }
        
        SystemDebuger::printSystemStats();
        delay(1000);
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

void ApplicationTask::handleIdleState(ApplicationMessagePacket evt) {
    if (evt.type == ApplicationCommandType::Timer) {
        if (is23Pm()) {
            sendRtcSyncCommand();
            _state = state::RtcSync;
            timer.start(INTERVAL_5_SECONDS_MS); 
            LOG_INFO("Transitioning to RtcSync state");
        } else {
            _state = state::Measurements;
            timer.start(200); 
            LOG_INFO("Transitioning to Measurements state");
        }
    }
}

void ApplicationTask::sendRtcSyncCommand() {
    SystemMessagePacket drMsg;
    drMsg.type = SystemDataType::RtcSync;
    haQueueRef.send(drMsg);
}

void ApplicationTask::handleMeasurementsState(ApplicationMessagePacket evt) {
    

}

void ApplicationTask::handleHeaterControlState(ApplicationMessagePacket evt) {
    (void)evt;
}

void ApplicationTask::handleHaNotificationState(ApplicationMessagePacket evt) {
    (void)evt;
}

void ApplicationTask::handleRtcSyncState(ApplicationMessagePacket evt) {

    if (evt.type == ApplicationCommandType::Timer) {
        rtcRetrayCount++;
        if (rtcRetrayCount > 3) {
            LOG_ERROR("RTC sync retry limit reached, try next day");
            rtcRetrayCount = 0;
            _state = state::Idle;
            timer.start(INTERVAL_3_MINUTES_MS);
        } else {
            LOG_INFO("RTC sync retry #%u", rtcRetrayCount);
            sendRtcSyncCommand();
            timer.start(INTERVAL_5_SECONDS_MS); 
        }
    }

    if (evt.type == ApplicationCommandType::RtcSync) {
        uint32_t epochTime = evt.payload.rtcSyncCommandPacket.epochTime;
        DateTime dt(epochTime);
        rtc.set(dt);
        LOG_INFO("RTC synchronized to epoch time: %u", epochTime);
        _state = state::Idle;       //we don't have to run timer, itis already runing
    }
    
}

void ApplicationTask::handleHistoricalDataSyncState(ApplicationMessagePacket evt) {
    (void)evt;
}