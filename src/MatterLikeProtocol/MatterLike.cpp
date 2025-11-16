#include "MatterLike.h"

// Initialize static message counter
uint32_t MatterLike::messageCounter = 1;

// -------------------------------
// Public methods
// -------------------------------

MatterLikePacket MatterLike::createTurnOnPacket(uint16_t nodeId, uint8_t endpointId) {
    MatterLikePayload payload = createOnOffPayload(nodeId, endpointId, CMD_ON);
    return createPacket(messageCounter++, payload, 0x01);
}

MatterLikePacket MatterLike::createTurnOffPacket(uint16_t nodeId, uint8_t endpointId) {
    MatterLikePayload payload = createOnOffPayload(nodeId, endpointId, CMD_OFF);
    return createPacket(messageCounter++, payload, 0x01);
}

MatterLikePacket MatterLike::createReadOnOffPacket(uint16_t nodeId, uint8_t endpointId) {
    MatterLikePayload payload = createOnOffPayload(nodeId, endpointId, CMD_TOGGLE);
    return createPacket(messageCounter++, payload);
}

MatterLikePacket MatterLike::createReadElectricalPacket(uint16_t nodeId, uint8_t endpointId, uint16_t attributeId) {
    MatterLikePayload payload = createElectricalPayload(nodeId, endpointId, attributeId, 0x03);
    return createPacket(messageCounter++, payload);
}

MatterLikePacket MatterLike::createReportAttributePacket(uint16_t nodeId, uint8_t endpointId, uint16_t clusterId, uint16_t attributeId, int32_t value) {
    MatterLikePayload payload;
    if(clusterId == 0x0006) {
        payload = createOnOffPayload(nodeId, endpointId, 0x04, value);
    } else {
        payload = createElectricalPayload(nodeId, endpointId, attributeId, 0x04, value);
    }
    return createPacket(messageCounter++, payload);
}

MatterLikePacket MatterLike::createAckPacket(const MatterLikePacket& receivedPacket) {
    MatterLikePayload payload;
    payload.fabricId = receivedPacket.payload.fabricId;
    payload.nodeId = receivedPacket.payload.nodeId;
    payload.endpointId = receivedPacket.payload.endpointId;
    payload.clusterId = receivedPacket.payload.clusterId;
    payload.attributeId = receivedPacket.payload.attributeId;
    payload.commandId = 0x05; // 0x05 = ACK command
    payload.value = 0;        // no value needed

    return createPacket(receivedPacket.header.messageCounter, payload, 0x00); // flags=0
}

// -------------------------------
// Private helpers
// -------------------------------

MatterLikeHeader MatterLike::createHeader(uint32_t msgCounter, uint16_t sessionId, uint8_t flags) {
    MatterLikeHeader header;
    header.messageCounter = msgCounter;
    header.sessionId = sessionId;
    header.flags = flags;
    return header;
}

MatterLikePayload MatterLike::createOnOffPayload(uint16_t nodeId, uint8_t endpointId, uint8_t commandId, int32_t value) {
    MatterLikePayload payload;
    payload.fabricId = DEFAULT_FABRIC_ID;
    payload.nodeId = nodeId;
    payload.endpointId = endpointId;
    payload.clusterId = CLUSTER_ONOFF; // OnOff Cluster
    payload.attributeId = 0x0000;
    payload.commandId = commandId;
    payload.value = value;
    return payload;
}

MatterLikePayload MatterLike::createElectricalPayload(uint16_t nodeId, uint8_t endpointId, uint16_t attributeId, uint8_t commandId, int32_t value) {
    MatterLikePayload payload;
    payload.fabricId = DEFAULT_FABRIC_ID;
    payload.nodeId = nodeId;
    payload.endpointId = endpointId;
    payload.clusterId = 0x0B04; // Electrical Measurement Cluster
    payload.attributeId = attributeId;
    payload.commandId = commandId;
    payload.value = value;
    return payload;
}

MatterLikePacket MatterLike::createPacket(uint32_t msgCounter, const MatterLikePayload& payload, uint8_t flags) {
    MatterLikePacket packet;
    packet.header = createHeader(msgCounter, DEFAULT_SESSION_ID, flags);
    packet.payload = payload;
    return packet;
}
