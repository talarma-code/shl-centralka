

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
#include "ResetReason.h"

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

    //setup RTC and system time first to have correct timestamps in logs from the beginning
    rtc.setup();
    setupSystemTime();

    logLastResetReason();
    
    transport.begin(MAC_CENTRALKA, MAC_LOCAL_HEATER);
    transport.onPacketReceived(this);
    heaterFsm.registerTransport(&transport);
    heaterFsm.registerHeaterMac(MAC_LOCAL_HEATER);
    powerMeterFsm.setup();

    timer.start(25000);
}

void ApplicationTask::logLastResetReason() {

    LOG_ERROR("Last reset reason: %s (%d)", lastResetReason(), lastResetReasonCode());
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
    resetWatchdog();

    
    //This is only test code - will be removed 
    if (Serial.available() > 0) {
        char characterRecived = Serial.read();   // Odczytaj 1 bajt

        //section for debug espNow communication with heaterFsm
        if (characterRecived == 'o') {
            Serial.println("odebralem i wyslam ramke On");
            heaterFsm.sendCommand(true);
        }

        if (characterRecived == 'f') {
            Serial.println("odebralem i wyslam ramke Off");
            heaterFsm.sendCommand(false);
            //action Off
        }

        if (characterRecived == 'd') {
            SystemDebuger::printSystemStats();
        }

        if (characterRecived == 'R') {
            Serial.println("test Rsync");
            SystemMessagePacket drMsg;
            drMsg.type = SystemDataType::RtcSync;
            haQueueRef.send(drMsg);
            _state = state::RtcSync;
        }

        //for sim7000 modem test 
        if (characterRecived == 's') {
            Serial.println("Soft reset modem SIM7000");
            SystemMessagePacket drMsg;
            drMsg.type = SystemDataType::ServiceCommand;
            drMsg.payload.serviceCommandData.command = ServiceCommandEnum::SoftwareResetModem;
            haQueueRef.send(drMsg);
        }

        if (characterRecived == 'h') {
            Serial.println("Hardware reset modem SIM7000");
            SystemMessagePacket drMsg;
            drMsg.type = SystemDataType::ServiceCommand;
            drMsg.payload.serviceCommandData.command = ServiceCommandEnum::HardwareResetModem;
            haQueueRef.send(drMsg);
        }




        //Modbus test section 

        if (characterRecived == '1') {
            float value;
            sdm120ctPowerMeter.voltage(value, 0x01);
            Serial.print("Voltage L1: ");   
            Serial.println(value);
        }
        if (characterRecived == '2') {
            float value;
            sdm120ctPowerMeter.voltage(value, 0x02);
            Serial.print("Voltage L2: ");   
            Serial.println(value);
        }

        if (characterRecived == '3') {
            float power;
            sdm120ctPowerMeter.activePower(power, 0x01);
            Serial.print("Active Power L1: ");   
            Serial.println(power);
        }
        if (characterRecived == '4') {
            float power;
            sdm120ctPowerMeter.activePower(power, 0x02);
            Serial.print("Active Power L2: ");   
            Serial.println(power);
        }
        if (characterRecived == '5') {
            float energy;
            sdm120ctPowerMeter.importActiveEnergy(energy, 0x01);
            Serial.print("Import Active Energy L1: ");   
            Serial.println(energy);
        }
        if (characterRecived == '6') {
            float energy;
            sdm120ctPowerMeter.importActiveEnergy(energy, 0x02);
            Serial.print("Import Active Energy L2: ");   
            Serial.println(energy);
        }
        
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
    if (evt.type == ApplicationCommandType::RtcSync) {
        LOG_INFO("Receive new timestamp from GSM network - set up RTC");
        uint32_t epoch = evt.payload.rtcSyncCommandPacket.epochTime;
        updateRtcTime(epoch);
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
            LOG_INFO("Heater action acknowledged, totalPower: %u W, voltage: %u.%u V", res.totalPower, res.voltage / 10, res.voltage % 10);
            espNowCommunicationErrorNotification(true);
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
            espNowCommunicationErrorNotification(false);
            _state = state::HaNotification;
            timer.start(INTERVAL_100_MS);
            break;
    }
}

void ApplicationTask::espNowCommunicationErrorNotification(bool status) {
    SystemMessagePacket haMsg;
    haMsg.type = SystemDataType::NotifyEspNowEvent;
    haMsg.payload.espNowEventData.timestamp = (uint32_t)(millis() / 1000UL);
    haMsg.payload.espNowEventData.heaterCommunicationStatus = status ? HeaterCommunicationStatus::Ok : HeaterCommunicationStatus::NoCommunication;
    if(!haQueueRef.send(haMsg, 50)) {           
        LOG_ERROR("Critical: HaQueue full, cannot send error notification!!!"); 
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
    
    Serial.println("Updating system time...");
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
    lastMeasurementData.L1EnergyProduced = data.L1TotalEnergy;
    lastMeasurementData.L2EnergyProduced = data.L2TotalEnergy;
    lastMeasurementData.HomeTotalEnergyConsumed = data.HomeTotalEnergy;
    lastMeasurementData.L1Voltage_x10 = data.L1Voltage_x10;
    lastMeasurementData.L2Voltage_x10 = data.L2Voltage_x10;

    lastMeasurementData.L1Power3minW = data.L1Power3minW;
    lastMeasurementData.L2Power3minW = data.L2Power3minW;
    lastMeasurementData.HomePower3minW = data.HomePower3minW;

    lastMeasurementData.L1PowerNowW = data.L1PowerNowW;
    lastMeasurementData.L2PowerNowW = data.L2PowerNowW;

    lastMeasurementData.heaterRequestedStatus = heaterStatus ? HeaterStatus::On : HeaterStatus::Off;
    lastMeasurementData.measurementType = MeasurementDataType::Now;
}   

void ApplicationTask::calculateHeaterStatus(const MeasurementData& data) {
    LOG_INFO("Calculating heater status with data: L1Power=%u W, L1TotalEnergy=%u W, L2Power=%u W, L2TotalEnergy=%u W, HomePower=%u W", data.L1EnergyInLastTimeWindow, data.L1TotalEnergy, data.L2EnergyInLastTimeWindow, data.L2TotalEnergy, data.HomeEnergyInLastTimeWindow);
    
    bool heaterRequestedState = hourlySurplusAlgorithm.calculatePower(getSystemDateTime(), data.L1EnergyInLastTimeWindow, data.L2EnergyInLastTimeWindow, data.HomeEnergyInLastTimeWindow);
    LOG_INFO("Heater requested state: %s", heaterRequestedState ? "ON" : "OFF");
    heaterFsm.setHeaterState(heaterRequestedState);
    collectDataForHaNotification(data, heaterRequestedState);
}

void ApplicationTask::haPowerMetersErrorNotification() {
    // SystemMessagePacket haMsg;
    // haMsg.type = SystemDataType::NotifyEspNowEvent;
    // if(!haQueueRef.send(haMsg, 50)) {           
    //     LOG_ERROR("Critical: HaQueue full, cannot send error notification!!!"); 
    // }
    LOG_ERROR("Power meter measurement failed after retries, notifying HA");
}
