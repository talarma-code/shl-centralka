#include "EspNowTransport.h"
#include <WiFi.h>

void (*EspNowTransport::userCallback)(const MatterLikePacket &, const uint8_t *) = nullptr;

bool EspNowTransport::begin() {
     WiFi.mode(WIFI_STA);; // musimy być w trybie Wi-Fi STA

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init failed");
        return false;
    }

    // Rejestrujemy nowy typ callbacku
    esp_err_t res = esp_now_register_recv_cb(onDataRecv);
    if (res != ESP_OK) {
        Serial.println("ESP-NOW register recv callback failed");
        return false;
    }

    esp_now_register_send_cb(onDataSent);

    Serial.println("ESP-NOW initialized successfully");
    return true;
}

bool EspNowTransport::send(const uint8_t *peerMac, const MatterLikePacket &packet) {
    esp_err_t result = esp_now_send(peerMac, reinterpret_cast<const uint8_t *>(&packet), sizeof(packet));
    return (result == ESP_OK);
}

void EspNowTransport::sendAck(const uint8_t *peerMac, const MatterLikePacket &receivedPacket) {
    MatterLikePacket ack = receivedPacket;
    ack.payload.commandId = 0xFF; // custom ACK command
    ack.payload.value = 1;
    send(peerMac, ack);
}

void EspNowTransport::onPacketReceived(void (*callback)(const MatterLikePacket &, const uint8_t *senderMac)) {
    userCallback = callback;
}

// ----------------------- private callbacks -----------------------

void EspNowTransport::onDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int len) {
    if (len == sizeof(MatterLikePacket)) {
        MatterLikePacket packet;
        memcpy(&packet, data, sizeof(MatterLikePacket));

        if (userCallback) {
            userCallback(packet, esp_now_info->src_addr);
        }
    } else {
        Serial.printf("Invalid ESP-NOW packet length: %d bytes\n", len);
    }
}

void EspNowTransport::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("ESP-NOW send status to ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac_addr[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.print(" -> ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}
