#include "ResetReason.h"


const char* lastResetReason() {
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
    return reasonStr;
}

int lastResetReasonCode() {
    return static_cast<int>(esp_reset_reason());
}