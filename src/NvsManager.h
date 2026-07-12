#pragma once

#include <Arduino.h>
#include "nvs_flash.h"

class NvsManager {
public:
    // Initialize NVS. Returns esp_err_t from nvs_flash_init/erase.
    static esp_err_t init(bool eraseIfNeeded = true);
};
