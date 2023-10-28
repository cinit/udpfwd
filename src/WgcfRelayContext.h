//
// Created by sulfate on 10/24/23.
//

#ifndef UDPFWD_WGCFRELAYCONTEXT_H
#define UDPFWD_WGCFRELAYCONTEXT_H


#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <unordered_map>

#include "INetAddress.h"
#include "INetSocketAddress.h"


namespace wgrelay {

using SocketHandle = int; // file descriptor

class WgcfRelaySession {
public:
    uint64_t sessionId = 0;
    uint32_t wireguardReserved = 0; // fixed
    uint32_t clientPeerId = 0;
    uint32_t serverPeerId = 0;
    SocketHandle outboundSocket = -1;
    uint64_t lastTxTimestampSeconds = 0;
    uint64_t lastRxTimestampSeconds = 0;
    uint32_t droppedTxSinceLastReport = 0;
    uint32_t droppedRxSinceLastReport = 0;
    uint32_t totalTxPacketsSinceLastReport = 0;
    uint32_t totalRxPacketsSinceLastReport = 0;
    net::INetSocketAddress outboundSocketAddress; // fixed
    net::INetSocketAddress sourceAddress; // dynamic

    constexpr WgcfRelaySession() noexcept = default;

    constexpr WgcfRelaySession(uint64_t sessionId, uint32_t wireguardReserved,
                               uint32_t clientPeerId, uint32_t serverPeerId,
                               SocketHandle outboundSocket,
                               net::INetSocketAddress outboundSocketAddress,
                               net::INetSocketAddress sourceAddress,
                               uint64_t lastTxTimestampSeconds = 0,
                               uint64_t lastRxTimestampSeconds = 0) noexcept
            : sessionId(sessionId),
              wireguardReserved(wireguardReserved),
              clientPeerId(clientPeerId),
              serverPeerId(serverPeerId),
              outboundSocket(outboundSocket),
              lastTxTimestampSeconds(lastTxTimestampSeconds),
              lastRxTimestampSeconds(lastRxTimestampSeconds),
              outboundSocketAddress(outboundSocketAddress),
              sourceAddress(sourceAddress) {}

    // disable copy and move to prevent accidental copies
    WgcfRelaySession(const WgcfRelaySession&) = delete;

    WgcfRelaySession& operator=(const WgcfRelaySession&) = delete;

    // allow move
    WgcfRelaySession(WgcfRelaySession&&) = default;

    WgcfRelaySession& operator=(WgcfRelaySession&&) = default;

};

class WgcfRelayContext {
public:
    // std::shared_mutex lock; not used, currently single-threaded
    std::unordered_map<uint64_t, WgcfRelaySession> sessions;
    SocketHandle inboundSocket = -1;
    std::unordered_map<SocketHandle, uint64_t> fileDescriptorToSessionIdMap;
    net::INetSocketAddress inboundSocketAddress; // fixed
    net::INetSocketAddress destinationAddress; // fixed
    int epollHandle = -1;

    WgcfRelayContext() = default;

    WgcfRelayContext(const WgcfRelayContext&) = delete;

    WgcfRelayContext& operator=(const WgcfRelayContext&) = delete;

    WgcfRelayContext(WgcfRelayContext&&) = delete;

    WgcfRelayContext& operator=(WgcfRelayContext&&) = delete;
};

}

#endif //UDPFWD_WGCFRELAYCONTEXT_H
