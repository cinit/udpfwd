//
// Created by sulfate on 10/24/23.
//

#include <cstdint>
#include <string>
#include <vector>
#include <string_view>

#include "platform/log/Log.h"

#include "UdpForward.h"
#include "WgcfRelay.h"
#include "DebugConfig.h"

static constexpr auto LOG_TAG = "main";

int main(int argc, char* argv[]) {
    if (argc < 2) {
        LOGE("Too few arguments");
        LOGE("Usage for udpfwd:");
        LOGE("    {} -f [-d] path/to/config.txt", argv[0]);
        LOGE("Usage for wgcfrelay:");
        LOGE("    {} -r [-d] listen:port forward:port", argv[0]);
        LOGE("    Example: {} -r [::]:51820 10.0.0.1:37000", argv[0]);
        return 1;
    }
    bool debug = false;
    std::vector<std::string_view> args;
    for (int i = 1; i < argc; i++) {
        std::string_view arg = argv[i];
        if (arg == "-d") {
            debug = true;
        } else {
            args.emplace_back(arg);
        }
    }
    if (debug) {
        ::debugconfig::SetDebug(true);
    }
    if (args[0] == std::string_view("-f")) {
        if (args.size() != 2) {
            LOGE("Unexpected number of arguments, run command with no arguments for usage");
            return 1;
        }
        return udpfwd::RunUdpForwarder(args[1]);
    } else if (args[0] == std::string_view("-r")) {
        if (args.size() != 3) {
            LOGE("Unexpected number of arguments, run command with no arguments for usage");
            return 1;
        }
        return wgrelay::RunWgcfRelay(args[1], args[2]);
    } else {
        LOGE("Unknown command: {}", args[0]);
        LOGE("Run command with no arguments for usage");
        return 1;
    }
}
