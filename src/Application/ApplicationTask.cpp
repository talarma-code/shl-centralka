

#include <esp_system.h>
#include <sys/time.h>
#include <time.h>
#include <Arduino.h>

#include "ApplicationTask.h"
#include "GlobalTypes.h"

#include "SystemDebuger.h"
#include "ActiveQueueRef.h"
#include "Log.h"
#include "Measurements.h"
#include "ShlProtocol.h"


#define LED_PIN 2  // wbudowana dioda LED
static const uint8_t MAC_LOCAL_HEATER[]  = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x31}; // talar1 - heater
static const uint8_t MAC_CENTRALKA[]   = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x30}; // talar0 - centrala

ApplicationTask::ApplicationTask(QueueHandle_t mainTaskQueueHandle, QueueHandle_t haQueueHandle, QueueHandle_t dataRecorderQueueHandle) : 
    ActiveTask("application", 8192, 1),
    mainTaskQueue(mainTaskQueueHandle),
    haQueueRef(haQueueHandle),
    dataRecorderQueueRef(dataRecorderQueueHandle),
    timer(APPLICATION_SYSTEM_TIMER_ID, 5000, SystemTimerT<ApplicationMessagePacket, TimerToApplicationMessage>::Mode::OneShot, ActiveQueueRef<ApplicationMessagePacket>(mainTaskQueue.nativeHandle()), TimerToApplicationMessage())
{

}

const char* ApplicationTask::stateToString(state s) {
    switch (s) {
        case state::Idle: return "Idle";
        case state::Measurements: return "Measurements";
        case state::HeaterControl: return "HeaterControl";
        case state::HaNotification: return "HaNotification";
        case state::RtcSync: return "RtcSync";
        case state::HistoricalDataSync: return "HistoricalDataSync";
        default: return "Unknown";
    }
}

const char* ApplicationTask::commandTypeToString(ApplicationCommandType type) {
    switch (type) {
        case ApplicationCommandType::Timer: return "Timer";
        case ApplicationCommandType::ProtocolPacket: return "ProtocolPacket";
        case ApplicationCommandType::HeaterCommand: return "HeaterCommand";
        case ApplicationCommandType::RtcSync: return "RtcSync";
        default: return "Unknown";
    }
}

void ApplicationTask::setup() {
    Serial.begin(115200);
    Serial.println("\n=== SHL Centralka Start ===");

    logLastResetReason();
    
    transport.begin(MAC_CENTRALKA, MAC_LOCAL_HEATER);
    transport.onPacketReceived(this);
    heaterFsm.registerTransport(&transport);
    heaterFsm.registerHeaterMac(MAC_LOCAL_HEATER);

    rtc.setup();
    setupSystemTime();
    powerMeterFsm.setup();

    timer.start(25000);
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
        LOG_DEBUG("App loop state=%s msg=%s", stateToString(_state), commandTypeToString(evt.type));
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
    }

    
    //This is only test code - will be removed 
    if (Serial.available() > 0) {
        char characterRecived = Serial.read();   // Odczytaj 1 bajt

        if (characterRecived == 'o') {
            Serial.println("odebralem i wyslam ramke On");
            heaterFsm.sendCommand(true);
        }

        if (characterRecived == 'f') {
            Serial.println("odebralem i wyslam ramke Off");
            heaterFsm.sendCommand(false);
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
void ApplicationTask::handlePacket(const ShlProtocolWithMacAddress &pkt) {
    ApplicationMessagePacket msg{};
    msg.type = ApplicationCommandType::ProtocolPacket;
    msg.payload.protocolPacket = pkt;
    mainTaskQueue.sendFromISR(msg, nullptr);
}

// Złoty środek (praktyka)
// Typowy, sprawdzony układ:
// Task	Priority	Dlaczego
// App logic	2	reaguje na zdarzenia
// SD writer	3	opróżnia kolejkę
// WiFi / timers	20+	system
// Idle	0	system


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

//todo: do rozwazenia praca w trybie czesciowego pomiaru, jak tylko jeden licznik dziala.
// mozna dodac jakis status zeby to bylo widoczne na HA 
void ApplicationTask::handleMeasurementsState(ApplicationMessagePacket evt) {
    if (evt.type == ApplicationCommandType::Timer) {
        MeasurementData data;
        auto measurmentStatus = powerMeterFsm.messurementReady(data);
        switch (measurmentStatus.next)
        {
            case PowerMeterFsm::Next::NextState: 
                calculateHeaterStatus(data);
                _state = state::HeaterControl;
                break;
            case PowerMeterFsm::Next::Stay:
                _state = state::Measurements;
                break;
            case PowerMeterFsm::Next::Error:
                haPowerMetersErrorNotification();
                _state = state::HeaterControl;  
                break;
        }     
        timer.start(measurmentStatus.delayMs);
    }       
}

void ApplicationTask::handleHeaterControlState(ApplicationMessagePacket evt) {
    auto res = heaterFsm.step(evt);
    switch (res.next) {
        case HeaterFsm::Next::RecivedResponse:
            LOG_INFO("Heater action acknowledged");
            _state = state::HaNotification;
            //DO NOT RUN TIMER, it is already running (as timeout timer) and will trigger transition to HaNotification state
            break;
        case HeaterFsm::Next::Stay:
            if (res.delayMs != DO_NOT_RUN_TIMER) {
                timer.start(res.delayMs);
            }
            break;
        case HeaterFsm::Next::Error:
            LOG_ERROR("Heater action not acknowledged");
            _state = state::HaNotification;
            timer.start(INTERVAL_100_MS);
            break;
    }
}

void ApplicationTask::handleHaNotificationState(ApplicationMessagePacket evt) {
    if (evt.type == ApplicationCommandType::Timer) {
        sendMeasurementToHa(lastMeasurementData);
        _state = state::Idle;
        timer.start(INTERVAL_3_MINUTES_MS); 
    }
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
        updateSystemTime(epochTime);
        updateRtcTime(epochTime);

        LOG_INFO("RTC synchronized to epoch time: %u (system time updated)", epochTime);
        _state = state::Idle;       //we don't have to run timer, itis already runing
    }
}

void ApplicationTask::handleHistoricalDataSyncState(ApplicationMessagePacket evt) {
    (void)evt;
}


void ApplicationTask::updateSystemTime(uint32_t epochTime) {
    time_t epoch = static_cast<time_t>(epochTime);
    struct timeval tv;
    tv.tv_sec = epoch;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);
}

void ApplicationTask::updateRtcTime(uint32_t epochTime) {
    DateTime dt(epochTime);
    rtc.set(dt);
}

DateTime ApplicationTask::getSystemDateTime() {
    time_t now = time(nullptr);
    if (now == (time_t)-1) {
        return DateTime(static_cast<uint32_t>(0));
    }

    return DateTime(static_cast<uint32_t>(now));
}

void ApplicationTask::collectDataForHaNotification(const MeasurementData& data, bool heaterStatus) {
    lastMeasurementData.timestamp = (uint32_t)(millis() / 1000UL);
    lastMeasurementData.L1Power = data.L1Power;
    lastMeasurementData.L2Power = data.L2Power; 
    lastMeasurementData.HomeTotalPower = data.HomePower;
    lastMeasurementData.L1Voltage_x10 = data.L1Voltage_x10; 
    lastMeasurementData.L2Voltage_x10 = data.L2Voltage_x10;
    lastMeasurementData.heaterRequestedStatus = heaterStatus ? HeaterStatus::On : HeaterStatus::Off;
    lastMeasurementData.measurementType = MeasurementDataType::Now;
}   

void ApplicationTask::calculateHeaterStatus(const MeasurementData& data) {
    bool heaterRequestedState = hourlySurplusAlgorithm.calculatePower(getSystemDateTime(), data.L1Power, data.L2Power, data.HomePower);
    heaterFsm.setHeaterState(heaterRequestedState);
    collectDataForHaNotification(data, heaterRequestedState);
}

void ApplicationTask::haPowerMetersErrorNotification() {
    //todo - implement error notification to HA, maybe with retry mechanism
    LOG_ERROR("Power meter measurement failed after retries, notifying HA");
}
