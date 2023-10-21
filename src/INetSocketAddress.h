//
// Created by sulfate on 10/20/23.
//

#ifndef UDPFWD_INETSOCKETADDRESS_H
#define UDPFWD_INETSOCKETADDRESS_H

#include <cstdint>
#include <string>
#include <array>
#include <string_view>

#include "INetAddress.h"

#include "fmt/format.h"

namespace net {

class INetSocketAddress {
public:

    INetAddress address = {};

    uint16_t port = 0;

    INetSocketAddress() noexcept = default;

    INetSocketAddress(const INetAddress addr, uint16_t p) noexcept: address(addr), port(p) {}

    INetSocketAddress(std::string_view addr, uint16_t p) noexcept: address(INetAddress::FromString(addr)), port(p) {}

    [[nodiscard]] inline bool IsIpv4() const noexcept {
        return address.IsIpv4();
    }

    [[nodiscard]] inline bool IsIpv6() const noexcept {
        return address.IsIpv6();
    }

    [[nodiscard]] inline bool IsValid() const noexcept {
        return address.IsValid();
    }

    [[nodiscard]] std::string ToString() const;

    [[nodiscard]] inline bool operator==(const INetSocketAddress& rhs) const noexcept {
        return address == rhs.address && port == rhs.port;
    }

};

}

// fmt helper
template<>
struct fmt::formatter<net::INetSocketAddress> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const net::INetSocketAddress& addr, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", addr.ToString());
    }
};

#endif //UDPFWD_INETSOCKETADDRESS_H
