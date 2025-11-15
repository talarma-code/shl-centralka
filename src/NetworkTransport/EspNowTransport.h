#pragma once
#include <Arduino.h>
#include <esp_now.h>
#include "IMatterLikeTransport.h"
#include "../MatterLikeProtocol/MatterLikePacket.h"

class EspNowTransport : public IMatterLikeTransport {
public:

    bool begin() override;
    bool send(const uint8_t *peerMac, const MatterLikePacket &packet) override;
    void onPacketReceived(IMatterReceiver *receiver) override;

private:
    static void onDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int len);
    static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

    static IMatterReceiver *userReceiver;

};
