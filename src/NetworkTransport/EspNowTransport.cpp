#include "EspNowTransport.h"
#include "../MatterLikeProtocol/MatterLikePacket.h"
#include <cstring>
#include <WiFi.h>
#include <esp_wifi.h>


static uint8_t MAX_ESP_NOW_FRAME = 250;

IMatterReceiver* EspNowTransport::userReceiver = nullptr;

static const uint8_t MAC_LOCAL[]   = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x30}; // talar0 - centrala
static const uint8_t MAC_HEATER[]  = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x31}; // talar1 - heater


bool EspNowTransport::begin() {
    // WiFi.mode(WIFI_STA); // must be in STA for ESP-NOW

    // if (esp_now_init() != ESP_OK) {
    //     Serial.println("ESP-NOW init failed");
    //     return false;
    // }

        delay(300);

    Serial.println("\n=== SHL Main Controller Start ===");

    WiFi.mode(WIFI_STA);

    // Ustaw własny MAC (NOWOŚĆ: esp_wifi_set_mac wymaga specjalnego include!)
    if (esp_wifi_set_mac(WIFI_IF_STA, (uint8_t*)MAC_LOCAL) != ESP_OK) {
        Serial.println("⚠ Could not set custom MAC!");
    }

    Serial.print("Local MAC set to: ");
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
                   MAC_LOCAL[0], MAC_LOCAL[1], MAC_LOCAL[2],
                   MAC_LOCAL[3], MAC_LOCAL[4], MAC_LOCAL[5]);

    // Start ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("❌ ESP-NOW init failed!");
        return false;
    }

      // Dodaj odbiornik (heater) jako PEER
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, MAC_HEATER, 6);
    peerInfo.channel = 0;  // ten sam co WiFi
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
        Serial.println("Peer added OK");
    } else {
        Serial.println("❌ Peer add FAILED!");
    }

    esp_now_register_recv_cb(onDataRecv);
    esp_now_register_send_cb(onDataSent);



    Serial.println("ESP-NOW initialized successfully");
    return true;
}

bool EspNowTransport::send(const uint8_t *peerMac, const MatterLikePacket &packet) {

    if (sizeof(MatterLikePacket) > MAX_ESP_NOW_FRAME) { // ESP-NOW max payload ~250B
        Serial.println("Packet too large for ESP-NOW!");
        return false;
    }

    Serial.print("Sending to MAC: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", peerMac[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();

    uint8_t buffer[sizeof(MatterLikePacket)];
    memcpy(buffer, &packet, sizeof(MatterLikePacket));
   
    esp_err_t result = esp_now_send(peerMac, buffer, sizeof(MatterLikePacket));
    return (result == ESP_OK);
}


void EspNowTransport::onPacketReceived(IMatterReceiver *receiver) {
    userReceiver = receiver;
}

void EspNowTransport::onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (!userReceiver) {
        Serial.printf("EspNowTransport - userReceiver NOT REGISTERED!!!\n");
        return;
    }

    if (len != sizeof(MatterLikePacket)) {
        Serial.printf("Invalid ESP-NOW packet size: %d bytes (expected %u)\n",
                      len, sizeof(MatterLikePacket));
        return;
    }

    MatterLikePacket pkt;
    memcpy(&pkt, data, sizeof(MatterLikePacket));

    userReceiver->handlePacket(pkt, info->src_addr);
}


void EspNowTransport::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.printf("ESP-NOW send status: %s\n",
                  status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}
