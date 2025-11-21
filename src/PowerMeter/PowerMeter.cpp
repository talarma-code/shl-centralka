#include "PowerMeter.h"
#include "MatterLike.h"
#include "MatterLikePacket.h"


static const uint8_t MAC_HEATER_1[]  = {0x74, 0x61, 0x6C, 0x61, 0x72, 0x31}; // talar1 - heater


void PowerMeter::setup() {

}

// Zwraca napięcie w jednostkach np. 0.1 V
uint32_t PowerMeter::voltage(uint8_t slaveId) {
    MatterLikePacket packet = buildVoltageReadPacket();
    if (_transpot){
        _transpot->send(MAC_HEATER_1, packet);
    }
    else {
        Serial.println("HeaterEspNow::_transpot not registered ");
    }
    
    return 2300; // przykładowa wartość: 230.0 V
}
MatterLikePacket PowerMeter::buildVoltageReadPacket(
    uint16_t nodeIdDefault,
    uint8_t endpointIdDefault,
    uint32_t fabricIdDefault,
    uint32_t messageCounterDefault,
    uint16_t sessionIdDefault
) {
    MatterLikePacket pkt;

    pkt.header.messageCounter = messageCounterDefault;
    pkt.header.sessionId      = sessionIdDefault;
    pkt.header.flags          = 0x00;

    pkt.payload.fabricId      = fabricIdDefault;
    pkt.payload.nodeId        = nodeIdDefault;
    pkt.payload.endpointId    = endpointIdDefault;
    pkt.payload.clusterId     = CLUSTER_ELECTRICAL_MEAS;
    pkt.payload.attributeId   = ATTR_EM_RMS_VOLTAGE;
    pkt.payload.commandId     = ATTR_EM_RMS_VOLTAGE;
    pkt.payload.value         = 0;

    return pkt;
}


// Zwraca natężenie prądu w jednostkach np. 0.01 A
uint32_t PowerMeter::electricCurrent(uint8_t slaveId) {
    // TODO: odczyt z miernika
    return 150; // przykładowa wartość: 1.50 A
}

// Zwraca moc czynną w W (lub kW * 1000)
uint32_t PowerMeter::activePower(uint8_t slaveId) {
    // TODO: odczyt z miernika
    return 3450; // przykładowa wartość: 3.45 kW
}

// Zwraca częstotliwość w Hz * 100
uint32_t PowerMeter::frequency(uint8_t slaveId) {
    // TODO: odczyt z miernika
    return 5000; // przykładowa wartość: 50.00 Hz
}

// Zwraca sumaryczną energię czynną w Wh
uint32_t PowerMeter::totalActivePower(uint8_t slaveId) {
    // TODO: odczyt z miernika
    return 123456; // przykładowa wartość: 123.456 kWh
}

void PowerMeter::registerTransport(IMatterLikeTransport* transportLayer) {
    _transpot = transportLayer;
}
