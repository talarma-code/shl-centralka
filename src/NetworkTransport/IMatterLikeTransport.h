#pragma once
#include <Arduino.h>
#include "IMatterReceiver.h"
#include "../MatterLikeProtocol/MatterLikePacket.h"


// #define MAC_SHL_CENTRALLA "40:91:51:20:C1:98"
// #define MAC_SHL_STEROWNIK_GRZALKI "88:57:21:C0:8F:28"

class IMatterLikeTransport {
public:
  virtual ~IMatterLikeTransport() = default;

  // Initialize transport (radio, sockets, etc.)
  virtual bool begin() = 0;

  // Send packet to a specific peer
  virtual bool send(const uint8_t *peerMac, const MatterLikePacket &packet) = 0;

  // Set user callback for received packets
  virtual void onPacketReceived(IMatterReceiver *receiver) = 0;
};
