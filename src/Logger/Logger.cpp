#include "Logger.h"
#include <Arduino.h>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

void Logger::init(QueueHandle_t handle) {
    if (handle != nullptr) {
        logQueueRef = ActiveQueueRef<SystemLogPacket>(handle);
        initialized = true;
    } else {
        initialized = false;
    }
}

void Logger::log(LogLevel level, uint32_t timestamp, const char* message) {
    if (!initialized || !message) {
        return;
    }

    if (level < logLevel) {
        return;
    }

    SystemLogPacket pkt;
    pkt.level = level;
    pkt.timestamp = timestamp;
    pkt.logMessage.assign(message);

    dispatch(pkt);
}

void Logger::dispatch(const SystemLogPacket& pkt) {
    if (output == Output::SdOnly || output == Output::SdAndUart) {
        logQueueRef.send(pkt, 0);
    }

    if (output == Output::UartOnly || output == Output::SdAndUart) {
        unsigned long currentMillis = millis();
        unsigned long sec = static_cast<unsigned long>(pkt.timestamp);
        unsigned long ms = currentMillis % 1000UL;
        Serial.printf("[%lu.%03lu] [%u] %s\n",
                      sec,
                      ms,
                      static_cast<uint8_t>(pkt.level),
                      pkt.logMessage.c_str());
    }
}
