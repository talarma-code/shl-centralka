#pragma once

#include <Arduino.h>
#include <esp_now.h>
#include "MatterLikePacket.h" // Your header with structures

class MatterLike {
public:
    // -------------------------------
    // Public API: Create SmartPlug packets
    // -------------------------------

    static MatterLikePacket createTurnOnPacket(uint16_t nodeId, uint8_t endpointId);
    static MatterLikePacket createTurnOffPacket(uint16_t nodeId, uint8_t endpointId);
    static MatterLikePacket createReadOnOffPacket(uint16_t nodeId, uint8_t endpointId);
    
    static MatterLikePacket createReadElectricalPacket(uint16_t nodeId, uint8_t endpointId, uint16_t attributeId);
    static MatterLikePacket createReportAttributePacket(uint16_t nodeId, uint8_t endpointId, uint16_t clusterId, uint16_t attributeId, int32_t value);

    static MatterLikePacket createAckPacket(const MatterLikePacket& receivedPacket);

private:
    // -------------------------------
    // Private helpers
    // -------------------------------
    static constexpr uint32_t DEFAULT_FABRIC_ID = 0xA1B2C3D4;
    static constexpr uint16_t DEFAULT_SESSION_ID = 0x0042;

    static MatterLikeHeader createHeader(uint32_t messageCounter, uint16_t sessionId = DEFAULT_SESSION_ID, uint8_t flags = 0);
    static MatterLikePayload createOnOffPayload(uint16_t nodeId, uint8_t endpointId, uint8_t commandId, int32_t value = 0);
    static MatterLikePayload createElectricalPayload(uint16_t nodeId, uint8_t endpointId, uint16_t attributeId, uint8_t commandId, int32_t value = 0);
    static MatterLikePacket createPacket(uint32_t messageCounter, const MatterLikePayload& payload, uint8_t flags = 0);

    // Optional: internal message counter
    static uint32_t messageCounter;
};
