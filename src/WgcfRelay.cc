//
// Created by sulfate on 10/24/23.
//

#include "WgcfRelay.h"

#include <cstdint>
#include <string>
#include <string_view>

#include "platform/log/Log.h"
#include "platform/IoUtils.h"
#include "platform/ErrorMsg.h"
#include "support/text/TextUtils.h"
#include "support/text/ParseNumber.h"

#include "WgcfRelayContext.h"
#include "WgcfRelayWorker.h"

static constexpr auto LOG_TAG = "WgcfRelay";

namespace wgrelay {

using platform::GetErrorMsg;

int RunWgcfRelay(std::string_view listenAddressString, std::string_view forwardAddressString) {
    using net::INetSocketAddress;
    using net::INetAddress;
    using namespace support::text;
    // find last :
    auto pos = listenAddressString.find_last_of(':');
    if (pos == std::string::npos) {
        LOGE("Invalid listen address: {}", listenAddressString);
        return 1;
    }
    auto listenAddress = INetSocketAddress(INetSocketAddress(
            listenAddressString.substr(0, pos),
            ParseUInt16(listenAddressString.substr(pos + 1)).value_or(0)));
    if (!listenAddress.IsValid() || listenAddress.port == 0) {
        LOGE("Invalid listen address: {}", listenAddressString);
        return 1;
    }
    pos = forwardAddressString.find_last_of(':');
    if (pos == std::string::npos) {
        LOGE("Invalid forward address: {}", forwardAddressString);
        return 1;
    }
    auto forwardAddress = INetSocketAddress(INetSocketAddress(
            forwardAddressString.substr(0, pos),
            ParseUInt16(forwardAddressString.substr(pos + 1)).value_or(0)));
    if (!forwardAddress.IsValid() || forwardAddress.port == 0) {
        LOGE("Invalid forward address: {}", forwardAddressString);
        return 1;
    }
    WgcfRelayContext context;
    if (SetupSockets(context, listenAddress, forwardAddress) != 0) {
        LOGE("Failed to setup sockets");
        return 1;
    }
    RunWorker(context);
    return 0;
}

}
