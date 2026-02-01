#include "HeaterFsm.h"

HeaterFsm::HeaterFsm(HeaterEspNow& heater)
    : _heater(heater) {}

HeaterFsm::Result HeaterFsm::step(const MatterPacketWithMac& pkt) {
    Result res{};
    res.hasCommand = false;

    HeaterCommandPacket cmd{};
    cmd.type = HeaterCommandType::Unused;
    cmd.state = false;
    cmd.power = 0;

    const MatterLikePacket& ml = pkt.packet;

    // ACK packet from remote node
    if (MatterLike::isAckResponsePacket(ml)) {
        cmd.type = HeaterCommandType::Ack;
        res.hasCommand = true;
    }
    // OnOff state report
    else if (ml.payload.clusterId == CLUSTER_ONOFF &&
             ml.payload.commandId == CMD_REPORT_ATTRIBUTE &&
             ml.payload.attributeId == ATTR_ONOFF_STATE) {
        cmd.type = HeaterCommandType::State;
        cmd.state = (ml.payload.value != 0);
        res.hasCommand = true;
    }
    // Electrical measurement - active power report
    else if (ml.payload.clusterId == CLUSTER_ELECTRICAL_MEAS &&
             ml.payload.commandId == CMD_REPORT_ATTRIBUTE &&
             ml.payload.attributeId == ATTR_EM_ACTIVE_POWER) {
        cmd.type = HeaterCommandType::Power;
        if (ml.payload.value > 0) {
            cmd.power = static_cast<uint32_t>(ml.payload.value);
        } else {
            cmd.power = 0;
        }
        res.hasCommand = true;
    }

    if (res.hasCommand) {
        res.command = cmd;
    }

    return res;
}
