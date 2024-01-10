//
// Created by sulfate on 10/24/23.
//

#ifndef UDPFWD_SOCKETADDRESSUTILS_H
#define UDPFWD_SOCKETADDRESSUTILS_H

#include <type_traits>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "INetAddress.h"
#include "INetSocketAddress.h"

namespace net {

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

}


#endif //UDPFWD_SOCKETADDRESSUTILS_H
