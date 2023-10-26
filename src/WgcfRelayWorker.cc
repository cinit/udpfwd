//
// Created by sulfate on 10/24/23.
//

#include "WgcfRelayWorker.h"

#include <type_traits>
#include <vector>
#include <atomic>
#include <random>
#include <fmt/format.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "platform/log/Log.h"
#include "platform/Time.h"
#include "platform/ErrnoRestorer.h"
#include "SocketAddressUtils.h"
#include "WireGuardPacketStruct.h"
#include "DebugConfig.h"

static constexpr auto LOG_TAG = "WgcfRelay";

#define DLOGV(fmt, ...) \
    do { \
        if (::debugconfig::IsDebug()) [[unlikely]] { \
            LOGV(fmt, ##__VA_ARGS__); \
        } \
    } while (false)

namespace wgrelay {

using SocketHandle = int; // file descriptor

uint64_t GenerateSessionId() {
    static std::atomic_uint32_t sNext = 1;
    return uint64_t(1u) << 32u | uint64_t(sNext.fetch_add(1, std::memory_order_relaxed));
}

std::string PrettySessionId(uint64_t sessionId) {
    if (sessionId >> 32u == 0x2u) [[likely]] {
        return fmt::format("r.{:06x}", sessionId & 0xffffffffu);
    } else {
        return fmt::format("0x{:x}", sessionId);
    }
}

int SetupSockets(WgcfRelayContext& context, net::INetSocketAddress listenAddr, net::INetSocketAddress forwardAddr) {
    using platform::ErrnoRestorer;
    context.inboundSocketAddress = listenAddr;
    context.destinationAddress = forwardAddr;
    // bind sockets
    sockaddr_in6 addr = {};
    if (!ToLinuxSocketAddress(listenAddr, addr)) {
        LOGE("Invalid inbound address: {}", listenAddr);
        return -EINVAL;
    }
    int inboundSocket = socket(addr.sin6_family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (inboundSocket < 0) {
        ErrnoRestorer err;
        LOGE("socket address {} failed: {}", listenAddr, strerror(errno));
        return -int(err);
    }
    if (listenAddr.address == net::constants::IPV6_ANY) {
        // make sure socket can receive both ipv4 and ipv6 packets
        int opt = 0;
        if (setsockopt(inboundSocket, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) < 0) {
            LOGW("setsockopt IPV6_V6ONLY to 0 failed: {}", strerror(errno));
        }
    }
    if (bind(inboundSocket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {
        ErrnoRestorer err;
        LOGE("bind address {} failed: {}", listenAddr, strerror(errno));
        return -int(err);
    }
    context.inboundSocket = inboundSocket;
    // setup epoll
    int epollHandle = epoll_create1(EPOLL_CLOEXEC);
    if (epollHandle < 0) {
        ErrnoRestorer err;
        LOGE("epoll_create1 failed: {}", strerror(errno));
        return -int(err);
    }
    context.epollHandle = epollHandle;
    epoll_event eventsToWait = {};
    eventsToWait.events = EPOLLIN;
    eventsToWait.data.fd = inboundSocket;
    if (epoll_ctl(epollHandle, EPOLL_CTL_ADD, inboundSocket, &eventsToWait) < 0) {
        ErrnoRestorer err;
        LOGE("epoll_ctl add inbound socket {} failed: {}", listenAddr, strerror(errno));
        return -int(err);
    }
    return 0;
}


uint64_t GetSessionIdFromClientPacket(std::span<const uint8_t> packet) {
    using namespace wgrelay::wireguard;
    if (packet.size() < 32) {
        // invalid packet
        return 0;
    }
    auto header = reinterpret_cast<const WireGuardPacketHeader*>(packet.data());
    uint32_t reservedValue = header->typeAndReserved >> 8u;
    if (reservedValue != 0) {
        // for clients supporting client id, use reserved value as session id
        return uint64_t(2u) << 32u | reservedValue;
    }
    // TODO: 2023-10-25 support clients without client id
    return 0;
}

ssize_t SendUdpPacket(SocketHandle handle, std::span<const uint8_t> packet, const net::INetSocketAddress& address) {
    using namespace net;
    using namespace net::constants;
    sockaddr_in6 destAddr = {};
    if (!ToLinuxSocketAddress(address, destAddr)) {
        return -EINVAL;
    }
    auto rc = sendto(handle, packet.data(), packet.size(), 0,
                     reinterpret_cast<const sockaddr*>(&destAddr), sizeof(destAddr));
    if (rc < 0) {
        return -errno;
    }
    return rc;
}

void HandlePacketFromServer(WgcfRelayContext& context, std::span<const uint8_t> packet, uint64_t sessionId) {
    using platform::ErrnoRestorer;
    auto it = context.sessions.find(sessionId);
    if (it == context.sessions.end()) {
        LOGW("Unknown session id {} from server", PrettySessionId(sessionId));
        return;
    }
    auto session = it->second;
    auto clientAddress = session.sourceAddress;
    // forward packet to client
    // check whether source addr exists
    if (!session.sourceAddress.IsValid()) {
        LOGD("Source address for session {} is invalid, dropping packet", PrettySessionId(sessionId));
        return;
    }
    ssize_t sentSize = SendUdpPacket(context.inboundSocket, packet, clientAddress);
    if (sentSize < 0) {
        if (sentSize == -EAGAIN) {
            // dropped RX ++
            session.droppedRxSinceLastReport++;
        } else {
            LOGW("Session {} sendto client {} failed: {}", PrettySessionId(sessionId), clientAddress,
                 strerror(-sentSize));
        }
        return;
    }
    // update session info
    session.lastRxTimestampSeconds = platform::time::CurrentTimeSeconds();
    DLOGV("Session {} forwarded {} bytes from server {} to client {}",
          PrettySessionId(sessionId), packet.size(), context.destinationAddress, clientAddress);
}

auto NewOutboundUdpSocket(net::INetAddress::INetType type) {
    using platform::ErrnoRestorer;
    struct BindResult {
        int result;
        uint16_t port;
    };
    bool useIpv4 = type == net::INetAddress::INetType::kIpv4;
    int sock = socket(useIpv4 ? AF_INET : AF_INET6, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sock < 0) {
        ErrnoRestorer err;
        LOGE("socket failed: {}", strerror(errno));
        return BindResult{-int(err), 0};
    }
    union {
        sockaddr_in6 addr6 = {};
        sockaddr_in addr4;
    };
    socklen_t addrLen;
    if (useIpv4) {
        addr4.sin_family = AF_INET;
        addr4.sin_port = 0; // random port
        addrLen = sizeof(sockaddr_in);
    } else {
        addr6.sin6_family = AF_INET6;
        addr6.sin6_port = 0; // random port
        addrLen = sizeof(sockaddr_in6);
    }
    if (bind(sock, reinterpret_cast<const sockaddr*>(&addr6), addrLen) < 0) {
        ErrnoRestorer err;
        close(sock);
        LOGE("bind random port failed: {}", strerror(errno));
        return BindResult{-int(err), 0};
    }
    // get port
    addrLen = sizeof(addr6);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&addr6), &addrLen) < 0) {
        ErrnoRestorer err;
        close(sock);
        LOGE("getsockname failed: {}", strerror(errno));
        return BindResult{-int(err), 0};
    }
    uint16_t port = ntohs(useIpv4 ? addr4.sin_port : addr6.sin6_port);
    return BindResult{sock, port};
}

void HandlePacketFromClient(WgcfRelayContext& context, std::span<const uint8_t> packet, uint64_t sessionId,
                            net::INetSocketAddress fromAddress) {
    using namespace wgrelay::wireguard;
    using platform::ErrnoRestorer;
    using platform::time::CurrentTimeSeconds;
    using namespace net;
    using namespace net::constants;
    using INetType = INetAddress::INetType;
    uint32_t reservedValue = reinterpret_cast<const WireGuardPacketHeader*>(packet.data())->typeAndReserved >> 8u;
    auto it = context.sessions.find(sessionId);
    // if session id is not found, create a new session
    if (it == context.sessions.end()) {
        // create new session
        auto outboundSocketResult = NewOutboundUdpSocket(context.destinationAddress.address.GetType());
        if (outboundSocketResult.result < 0) {
            LOGE("Failed to create outbound socket for session {}", PrettySessionId(sessionId));
            return;
        }
        // add to epoll
        epoll_event eventsToWait = {};
        eventsToWait.events = EPOLLIN;
        eventsToWait.data.fd = outboundSocketResult.result;
        if (epoll_ctl(context.epollHandle, EPOLL_CTL_ADD, outboundSocketResult.result, &eventsToWait) < 0) {
            ErrnoRestorer err;
            LOGE("epoll_ctl add outbound socket {} failed: {}", context.destinationAddress, strerror(errno));
            close(outboundSocketResult.result);
            return;
        }
        auto outboundSocket = outboundSocketResult.result;
        auto outboundPort = outboundSocketResult.port;
        auto&& session = WgcfRelaySession{
                .sessionId = sessionId,
                .wireguardReserved = reservedValue,
                .clientPeerId = 0, // wip, not supported yet
                .serverPeerId = 0, // wip, not supported yet
                .outboundSocket = outboundSocket,
                .lastTxTimestampSeconds = CurrentTimeSeconds(),
                .lastRxTimestampSeconds = CurrentTimeSeconds(),
                .outboundSocketAddress = INetSocketAddress(
                        (context.destinationAddress.address.GetType() == INetType::kIpv4) ? IPV4_ANY : IPV6_ANY,
                        outboundPort),
                .sourceAddress = fromAddress,
        };
        context.sessions.try_emplace(sessionId, session);
        context.fileDescriptorToSessionIdMap.emplace(outboundSocket, sessionId);
        LOGI("New session {} created for client {} with outbound {} to server {}",
             PrettySessionId(sessionId), fromAddress, session.outboundSocketAddress, context.destinationAddress);
    }
    // now we must have session
    it = context.sessions.find(sessionId);
    if (it == context.sessions.end()) {
        LOGE("Session {} not found after creation, race condition?", PrettySessionId(sessionId));
        return;
    }
    auto& session = it->second;
    if (session.sourceAddress != fromAddress) {
        LOGI("Session {} (outbound {}) source address changed from {} to {}",
             PrettySessionId(sessionId), session.outboundSocketAddress, session.sourceAddress, fromAddress);
        session.sourceAddress = fromAddress;
    }
    // forward packet to server
    ssize_t sentSize = SendUdpPacket(session.outboundSocket, packet, context.destinationAddress);
    if (sentSize < 0) {
        if (sentSize == -EAGAIN) {
            // dropped TX ++
            session.droppedTxSinceLastReport++;
        } else {
            LOGW("Session {} sendto server {} size {} failed: {}",
                 PrettySessionId(sessionId), context.destinationAddress, packet.size(), strerror(-sentSize));
        }
        return;
    }
    // update session info
    session.lastTxTimestampSeconds = CurrentTimeSeconds();
    DLOGV("Session {} forwarded {} bytes from client {} to server {}",
          PrettySessionId(sessionId), packet.size(), fromAddress, context.destinationAddress);
}

int RunWorker(WgcfRelayContext& context) {
    using platform::time::CurrentTimeSeconds;
    using platform::ErrnoRestorer;
    std::vector<epoll_event> events(16);
    std::array<uint8_t, 65536> packetBuffer = {};
    while (true) {
        int numEvents = epoll_wait(context.epollHandle, events.data(), (int) events.size(), 100);
        if (numEvents < 0) {
            if (errno != EINTR) {
                ErrnoRestorer err;
                LOGE("epoll_wait failed: {}", strerror(errno));
                return -int(err);
            }
        }
        if (numEvents <= 0) {
            uint64_t nowSec = CurrentTimeSeconds();
            // report dropped packets if any
            // check for expired sessions, eg > 10 min
            for (auto it = context.sessions.begin(); it != context.sessions.end();) {
                if (it->second.droppedRxSinceLastReport != 0 || it->second.droppedTxSinceLastReport != 0) {
                    auto& session = it->second;
                    LOGI("Session {} client {} outbound {} has {} dropped rx, {} dropped tx",
                         PrettySessionId(session.sessionId), session.sourceAddress, session.outboundSocketAddress,
                         session.droppedRxSinceLastReport, session.droppedTxSinceLastReport);
                    // reset counter
                    session.droppedRxSinceLastReport = 0;
                    session.droppedTxSinceLastReport = 0;
                }
                if ((nowSec - it->second.lastRxTimestampSeconds > 600) ||
                    (nowSec - it->second.lastTxTimestampSeconds > 600)) {
                    auto lastTxRel = int64_t(it->second.lastTxTimestampSeconds) - int64_t(nowSec);
                    auto lastRxRel = int64_t(it->second.lastRxTimestampSeconds) - int64_t(nowSec);
                    LOGD("Session {} (last source {} outbound {}) expired, last tx {}s, last rx {}s",
                         PrettySessionId(it->second.sessionId), it->second.sourceAddress,
                         it->second.outboundSocketAddress, lastTxRel, lastRxRel);
                    int sock = it->second.outboundSocket;
                    if (sock >= 0) {
                        // remove from epoll
                        epoll_event eventsToWait = {};
                        eventsToWait.events = EPOLLIN;
                        eventsToWait.data.fd = sock;
                        if (epoll_ctl(context.epollHandle, EPOLL_CTL_DEL, sock, &eventsToWait) < 0) {
                            ErrnoRestorer err;
                            LOGE("epoll_ctl del outbound socket {} fd {} failed: {}",
                                 it->second.outboundSocketAddress, sock, strerror(errno));
                        }
                        close(sock);
                    }
                    context.fileDescriptorToSessionIdMap.erase(it->second.outboundSocket);
                    it = context.sessions.erase(it);
                } else {
                    ++it;
                }
            }
            continue;
        }
        for (int i = 0; i < numEvents; ++i) {
            const auto& event = events[i];
            sockaddr_in6 addr = {};
            socklen_t addrLen = sizeof(addr);
            ssize_t packetSize = recvfrom(event.data.fd, packetBuffer.data(), packetBuffer.size(), 0,
                                          reinterpret_cast<sockaddr*>(&addr), &addrLen);
            if (packetSize < 0) {
                ErrnoRestorer err;
                if (errno == EAGAIN || errno == EINTR) {
                    continue;
                }
                LOGE("recvfrom failed: {}", strerror(errno));
                return -int(err);
            }
            if (addrLen > sizeof(addr)) {
                LOGW("recvfrom returned invalid address length {}", addrLen);
                continue;
            }
            net::INetSocketAddress fromAddress;
            if (!ToSocketAddress(addr, fromAddress)) {
                LOGW("recvfrom returned invalid address family {}", addr.sin6_family);
                continue;
            }
            if (event.data.fd == context.inboundSocket) {
                // from client
                auto sid = GetSessionIdFromClientPacket(std::span<const uint8_t>(packetBuffer.data(), packetSize));
                if (sid == 0) {
                    // invalid WireGuard packet
                    LOGW("Invalid WireGuard packet from client {}, size {}", fromAddress, packetSize);
                    continue;
                }
                HandlePacketFromClient(context, std::span<const uint8_t>(packetBuffer.data(), packetSize),
                                       sid, fromAddress);
            } else {
                // from server
                // check whether the source address matches
                if (fromAddress != context.destinationAddress) {
                    LOGW("Unexpected packet size {} from server {}, expected {}", packetSize, fromAddress,
                         context.destinationAddress);
                    continue;
                }
                // lookup session id by socket handle
                auto it = context.fileDescriptorToSessionIdMap.find(event.data.fd);
                if (it == context.fileDescriptorToSessionIdMap.end()) {
                    LOGW("Unknown socket handle {} from server {}", event.data.fd, fromAddress);
                    continue;
                }
                HandlePacketFromServer(context, std::span<const uint8_t>(packetBuffer.data(), packetSize),
                                       it->second);
            }
        }
    }
}

}
