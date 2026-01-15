#pragma once

#include "IntertaskDataModel.h"
#include "ActiveQueueRef.h"

class Logger {
public:
    enum class Output : uint8_t {
        SdOnly,
        UartOnly,
        SdAndUart
    };

    static Logger& instance();
    void init(QueueHandle_t logQueueHandle);
    void log(LogLevel level, uint32_t timestamp, const char* message);

    void setLevel(LogLevel level) { logLevel = level; }
    LogLevel getLevel() const { return logLevel; }
    void setOutput(Output o) { output = o; }
    Output getOutput() const { return output; }

    template<typename... Args>
    void logf(LogLevel level, uint32_t timestamp, const char* format, Args... args) {
        if (!initialized || !format) {
            return;
        }

        if (level < logLevel) {
            return;
        }

        SystemLogPacket pkt{};
        pkt.level = level;
        pkt.timestamp = timestamp;
        pkt.logMessage.clear();
        pkt.logMessage.snprintf(format, args...);

        dispatch(pkt);
    }

private:
    Logger() = default;

    void dispatch(const SystemLogPacket& pkt);

    ActiveQueueRef<SystemLogPacket> logQueueRef{};
    volatile LogLevel logLevel = LogLevel::detailDebug; // domyslnie logujemy wszystko
    volatile Output output = Output::SdOnly;             // domyslnie tylko SD
    bool initialized = false;
};
