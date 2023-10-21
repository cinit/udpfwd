//
// Created by sulfate on 10/20/23.
//

#include "UdpForwardWorker.h"

#include <type_traits>
#include <vector>
#include <fmt/format.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include "platform/log/Log.h"
#include "platform/ErrnoRestorer.h"

static constexpr auto LOG_TAG = "UdpForwardWorker";

namespace udpfwd {

static bool kDebug = false;

template<typename SockAddrType>
requires(sizeof(SockAddrType) >= sizeof(sockaddr_in6))
bool ToLinuxSocketAddress(const net::INetSocketAddress& in, SockAddrType& out) {
    if (in.IsIpv4()) {
        auto& out4 = reinterpret_cast<sockaddr_in&>(out);
        memset(&out4, 0, sizeof(sockaddr_in));
        out4.sin_family = AF_INET;
        out4.sin_port = htons(in.port);
        memcpy(&out4.sin_addr, in.address.GetBytes().data(), 4);
        return true;
    } else if (in.IsIpv6()) {
        auto& out6 = reinterpret_cast<sockaddr_in6&>(out);
        memset(&out6, 0, sizeof(sockaddr_in6));
        out6.sin6_family = AF_INET6;
        out6.sin6_port = htons(in.port);
        memcpy(&out6.sin6_addr, in.address.GetBytes().data(), 16);
        return true;
    }
    return false;
}

template<typename SockAddrType>
requires(std::is_same_v<SockAddrType, sockaddr_in> || std::is_same_v<SockAddrType, sockaddr_in6>
         || std::is_same_v<SockAddrType, sockaddr> || std::is_same_v<SockAddrType, sockaddr_storage>)
bool ToSocketAddress(const SockAddrType& in, net::INetSocketAddress& out) {
    const auto& addr = reinterpret_cast<const sockaddr&>(in);
    if (addr.sa_family == AF_INET) {
        const auto& in4 = reinterpret_cast<const sockaddr_in&>(in);
        out.address = net::INetAddress(std::span<const uint8_t, 4>(
                reinterpret_cast<const uint8_t*>(&in4.sin_addr), 4));
        out.port = ntohs(in4.sin_port);
        return true;
    } else if (addr.sa_family == AF_INET6) {
        const auto& in6 = reinterpret_cast<const sockaddr_in6&>(in);
        out.address = net::INetAddress(std::span<const uint8_t, 16>(
                reinterpret_cast<const uint8_t*>(&in6.sin6_addr), 16));
        out.port = ntohs(in6.sin6_port);
        return true;
    }
    return false;
}

int SetupSockets(UdpForwardContext& context, std::vector<UdpForwardTargetInfo> targets) {
    using platform::ErrnoRestorer;
    // bind sockets and setup epoll
    int epollHandle = epoll_create1(EPOLL_CLOEXEC);
    if (epollHandle < 0) {
        ErrnoRestorer err;
        LOGE("epoll_create1 failed: {}", strerror(errno));
        return -int(err);
    }
    epoll_event eventsToWait = {};
    eventsToWait.events = EPOLLIN;
    context.epollHandle = epollHandle;
    for (int i = 0; i < targets.size(); ++i) {
        const auto& target = targets[i];
        sockaddr_in6 addr = {};
        if (!ToLinuxSocketAddress(target.inbound, addr)) {
            LOGE("Invalid inbound address: {}", target.inbound);
            return -EINVAL;
        }
        int inboundSocket = socket(addr.sin6_family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        if (inboundSocket < 0) {
            ErrnoRestorer err;
            LOGE("socket address {} failed: {}", target.inbound, strerror(errno));
            return -int(err);
        }
        if (bind(inboundSocket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {
            ErrnoRestorer err;
            LOGE("bind address {} failed: {}", target.inbound, strerror(errno));
            return -int(err);
        }
        eventsToWait.data.fd = inboundSocket;
        if (epoll_ctl(epollHandle, EPOLL_CTL_ADD, inboundSocket, &eventsToWait) < 0) {
            ErrnoRestorer err;
            LOGE("epoll_ctl add inbound socket {} failed: {}", target.inbound, strerror(errno));
            return -int(err);
        }
        if (!ToLinuxSocketAddress(target.outbound, addr)) {
            LOGE("Invalid outbound address: {}", target.outbound);
            return -EINVAL;
        }
        int outboundSocket = socket(addr.sin6_family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        if (outboundSocket < 0) {
            ErrnoRestorer err;
            LOGE("socket address {} failed: {}", target.outbound, strerror(errno));
            return -int(err);
        }
        if (bind(outboundSocket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {
            ErrnoRestorer err;
            LOGE("bind address {} failed: {}", target.outbound, strerror(errno));
            return -int(err);
        }
        eventsToWait.data.fd = outboundSocket;
        if (epoll_ctl(epollHandle, EPOLL_CTL_ADD, outboundSocket, &eventsToWait) < 0) {
            ErrnoRestorer err;
            LOGE("epoll_ctl add outbound socket {} failed: {}", target.outbound, strerror(errno));
            return -int(err);
        }
        context.sessions.emplace_back(UdpForwardSession{
                .inboundSocketAddress = target.inbound,
                .outboundSocketAddress = target.outbound,
                .destinationAddress = target.destination,
                .inboundSocket = inboundSocket,
                .outboundSocket = outboundSocket,
        });
        context.fileDescriptorToSessionIndex.emplace(inboundSocket, i);
        context.fileDescriptorToSessionIndex.emplace(outboundSocket, i);
    }
    return 0;
}

int RunWorker(UdpForwardContext& context) {
    using platform::ErrnoRestorer;
    using net::INetSocketAddress;
    using net::INetAddress;
    std::vector<epoll_event> events(16);
    while (true) {
        int epollCount = epoll_wait(context.epollHandle, events.data(), (int) events.size(), -1);
        if (epollCount < 0) {
            ErrnoRestorer err;
            LOGE("epoll_wait failed: {}", strerror(errno));
            return -int(err);
        }
        for (int i = 0; i < epollCount; ++i) {
            auto& event = events[i];
            if (event.events & EPOLLERR) {
                LOGE("epoll_wait returned EPOLLERR");
                return -EIO;
            }
            if (event.events & EPOLLHUP) {
                LOGE("epoll_wait returned EPOLLHUP");
                return -EIO;
            }
            if (event.events & EPOLLIN) {
                // read from inbound socket
                sockaddr_in6 addr = {};
                socklen_t addrLen = sizeof(addr);
                char buf[65536];
                ssize_t packetSize = recvfrom(event.data.fd, buf, sizeof(buf), 0,
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
                auto it = context.fileDescriptorToSessionIndex.find(event.data.fd);
                if (it == context.fileDescriptorToSessionIndex.end()) {
                    LOGE("epoll_wait returned unknown fd {}", event.data.fd);
                    return -EINVAL;
                }
                INetSocketAddress fromAddress;
                if (!ToSocketAddress(addr, fromAddress)) {
                    LOGW("recvfrom returned invalid address family {}", addr.sin6_family);
                    continue;
                }
                enum class PacketType {
                    kNone = 0,
                    kSourceToInboundSocket,
                    kDestinationToOutboundSocket,
                };
                PacketType packetType;
                auto& session = context.sessions[it->second];
                if (event.data.fd == session.inboundSocket) {
                    packetType = PacketType::kSourceToInboundSocket;
                } else if (event.data.fd == session.outboundSocket) {
                    packetType = PacketType::kDestinationToOutboundSocket;
                } else {
                    LOGW("epoll_wait returned unknown fd {}, from {}", event.data.fd, fromAddress);
                    continue;
                }
                if (packetType == PacketType::kSourceToInboundSocket) {
                    // check whether source addr changed
                    const auto& orig = session.sourceAddress;
                    const auto& now = fromAddress;
                    if (orig != now) {
                        LOGI("Source address for port {}/{} -> {} changed to {} (originally {})",
                             session.inboundSocketAddress, session.outboundSocketAddress,
                             session.destinationAddress, now, orig);
                        session.sourceAddress = now;
                    }
                    // forward to destination
                    sockaddr_in6 destAddr = {};
                    if (!ToLinuxSocketAddress(session.destinationAddress, destAddr)) {
                        LOGE("Invalid destination address: {}", session.destinationAddress);
                        return -EINVAL;
                    }
                    ssize_t sentSize = sendto(session.outboundSocket, buf, packetSize, 0,
                                              reinterpret_cast<const sockaddr*>(&destAddr),
                                              sizeof(destAddr));
                    if (sentSize < 0) {
                        ErrnoRestorer err;
                        LOGW("sendto {} failed: {}", session.outboundSocketAddress, strerror(errno));
                        continue;
                    }
                    if (kDebug) {
                        LOGV("Forwarded {} bytes from {} to {}",
                             packetSize, session.sourceAddress, session.destinationAddress);
                    }
                } else {
                    // forward to source
                    if (fromAddress != session.destinationAddress) {
                        LOGD("Unsolicited packet size {} from {} to {} dropped (expected {})",
                             packetSize, fromAddress, session.outboundSocketAddress, session.destinationAddress);
                        continue;
                    }
                    // check whether source addr exists
                    if (!session.sourceAddress.IsValid()) {
                        LOGD("Source address for port {}/{} -> {} not yet known, dropping packet",
                             session.inboundSocketAddress, session.outboundSocketAddress,
                             session.destinationAddress);
                        continue;
                    }
                    sockaddr_in6 destAddr = {};
                    if (!ToLinuxSocketAddress(session.sourceAddress, destAddr)) {
                        LOGE("Invalid source address: {}", session.sourceAddress);
                        return -EINVAL;
                    }
                    ssize_t sentSize = sendto(session.inboundSocket, buf, packetSize, 0,
                                              reinterpret_cast<const sockaddr*>(&destAddr), sizeof(destAddr));
                    if (sentSize < 0) {
                        ErrnoRestorer err;
                        LOGW("sendto {} failed: {}", session.inboundSocketAddress, strerror(errno));
                        continue;
                    }
                    if (kDebug) {
                        LOGV("Forwarded {} bytes from {} to {}",
                             packetSize, session.destinationAddress, session.sourceAddress);
                    }
                }
            }
        }
    }
}

}
