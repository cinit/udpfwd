#include <cstdint>
#include <string>
#include <string_view>


#include "platform/log/Log.h"
#include "platform/IoUtils.h"
#include "platform/ErrorMsg.h"
#include "support/text/TextUtils.h"
#include "support/text/ParseNumber.h"

#include "UdpForwardContext.h"
#include "UdpForwardWorker.h"

static constexpr auto LOG_TAG = "main";

using platform::GetErrorMsg;


int main(int argc, char* argv[]) {
    using namespace platform::io;
    using namespace platform::io::sizesuffix;
    using namespace support::text;
    using namespace udpfwd;
    if (argc != 2) {
        LOGE("Expected 1 argument, got {}", argc - 1);
        LOGE("Usage: {} path/to/config.txt", argv[0]);
        return 1;
    }
    if (!IsRegularFileExists(argv[1])) {
        LOGE("File {} does not exist", argv[1]);
        return 1;
    }
    std::string cfgContent;
    if (int rc = ReadAsString8AtMostOrFail(cfgContent, argv[1], 1_MB) != 0) {
        LOGE("Failed to read file {}: {}", argv[1], GetErrorMsg(rc));
        return 1;
    }
    auto lines = SplitStringWithRegex(cfgContent, std::regex("[\r\n;]"));
    // format: inbound-addr,inbound-port,outbound-addr,outbound-port,dest-addr,dest-port
    std::vector<UdpForwardTargetInfo> targets;
    for (const auto& line: lines) {
        if (line.empty()) {
            continue;
        }
        auto parts = SplitStringWithRegex(line, std::regex(","));
        if (parts.size() != 6) {
            LOGE("Invalid line: {}", line);
            return 1;
        }
        UdpForwardTargetInfo target;
        target.inbound.address = net::INetAddress::FromString(parts[0]);
        target.outbound.address = net::INetAddress::FromString(parts[2]);
        target.destination.address = net::INetAddress::FromString(parts[4]);
        if (ParseUInt16(parts[1], target.inbound.port) && ParseUInt16(parts[3], target.outbound.port)
            && ParseUInt16(parts[5], target.destination.port) && target.inbound.address.IsValid()
            && target.outbound.address.IsValid() && target.destination.address.IsValid()) {
            targets.emplace_back(target);
        } else {
            LOGE("Invalid line: {}", line);
            LOGE("Expected format: inbound-addr,inbound-port,outbound-addr,outbound-port,dest-addr,dest-port");
            return 1;
        }
    }
    UdpForwardContext context;
    if (SetupSockets(context, targets) != 0) {
        LOGE("Failed to setup sockets");
        return 1;
    }
    RunWorker(context);
    return 0;
}
