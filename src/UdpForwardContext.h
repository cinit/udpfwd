//
// Created by sulfate on 10/20/23.
//

#ifndef UDPFWD_UDPFORWARDCONTEXT_H
#define UDPFWD_UDPFORWARDCONTEXT_H

#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <unordered_map>

#include "INetAddress.h"
#include "INetSocketAddress.h"


namespace udpfwd {

using SocketHandle = int; // file descriptor

struct UdpForwardTargetInfo {
    net::INetSocketAddress inbound;
    net::INetSocketAddress outbound;
    net::INetSocketAddress destination;
};

struct UdpForwardSession {
    net::INetSocketAddress inboundSocketAddress; // fixed
    net::INetSocketAddress outboundSocketAddress; // fixed
    net::INetSocketAddress sourceAddress; // dynamic
    net::INetSocketAddress destinationAddress; // fixed
    SocketHandle inboundSocket = -1;
    SocketHandle outboundSocket = -1;
};

class UdpForwardContext {
public:
    // std::shared_mutex lock; not used, currently single-threaded
    std::vector<UdpForwardSession> sessions;
    std::unordered_map<SocketHandle, int> fileDescriptorToSessionIndex;
    int epollHandle = -1;

    UdpForwardContext() = default;

    UdpForwardContext(const UdpForwardContext&) = delete;

    UdpForwardContext& operator=(const UdpForwardContext&) = delete;

    UdpForwardContext(UdpForwardContext&&) = delete;

    UdpForwardContext& operator=(UdpForwardContext&&) = delete;
};

}

#endif //UDPFWD_UDPFORWARDCONTEXT_H
