//
// Created by sulfate on 10/20/23.
//

#include "INetSocketAddress.h"

std::string net::INetSocketAddress::ToString() const {
    if (IsIpv6()) {
        return ::fmt::format("[{}]:{}", address.ToString(), port);
    } else {
        return ::fmt::format("{}:{}", address.ToString(), port);
    }
}
