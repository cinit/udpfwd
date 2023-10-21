//
// Created by sulfate on 10/20/23.
//

#include "INetAddress.h"

#include <arpa/inet.h>
#include "fmt/format.h"

namespace net {

INetAddress INetAddress::FromString(std::string_view address) {
    // check if it's ipv4
    if (address.find(':') == std::string_view::npos) {
        // ipv4
        INetAddress addr;
        if (inet_pton(AF_INET, address.data(), &addr.mAddr4) == 1) {
            addr.mType = INetType::kIpv4;
            return addr;
        }
    } else {
        // ipv6
        if (address.starts_with('[') && address.ends_with(']')) {
            // remove brackets
            address.remove_prefix(1);
            address.remove_suffix(1);
        }
        INetAddress addr;
        if (inet_pton(AF_INET6, address.data(), addr.mAddr6.data()) == 1) {
            addr.mType = INetType::kIpv6;
            return addr;
        }
    }
    // failed
    return {};
}

std::string INetAddress::ToString() const {
    std::array<char, INET6_ADDRSTRLEN + 1> buf = {};
    if (IsIpv4()) {
        inet_ntop(AF_INET, &mAddr4, buf.data(), buf.size());
        return buf.data();
    } else if (IsIpv6()) {

        inet_ntop(AF_INET6, mAddr6.data(), buf.data(), buf.size());
        return buf.data();
    } else {
        return "(none)";
    }
}

}
