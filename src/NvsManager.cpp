#include "NvsManager.h"
#include "Log.h"

esp_err_t NvsManager::init(bool eraseIfNeeded)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        if (eraseIfNeeded) {
            LOG_INFO("NVS partition needs erase, erasing...");
            err = nvs_flash_erase();
            if (err != ESP_OK) {
                LOG_ERROR("NVS erase failed: %d", err);
                return err;
            }
            err = nvs_flash_init();
        }
    }
    if (err != ESP_OK) {
        LOG_ERROR("NVS init failed: %d", err);
    } else {
        LOG_INFO("NVS initialized correctly");
    }
    return err;
}
