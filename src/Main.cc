//
// Created by sulfate on 10/24/23.
//

#include <cstdint>
#include <string>
#include <string_view>

#include "platform/log/Log.h"

#include "UdpForward.h"
#include "WgcfRelay.h"

static constexpr auto LOG_TAG = "main";

int main(int argc, char* argv[]) {
    using namespace udpfwd;
    if (argc < 2) {
        LOGE("Too few arguments");
        LOGE("Usage for udpfwd:");
        LOGE("    {} -f path/to/config.txt", argv[0]);
        LOGE("Usage for wgcfrelay:");
        LOGE("    {} -r listen:port forward:port", argv[0]);
        LOGE("    Example: {} -r [::]:51820 10.0.0.1:37000", argv[0]);
        return 1;
    }
    if (argv[1] == std::string_view("-f")) {
        if (argc != 3) {
            LOGE("Unexpected number of arguments, run command with no arguments for usage");
            return 1;
        }
        return RunUdpForwarder(argv[2]);
    } else if (argv[1] == std::string_view("-r")) {
        if (argc != 4) {
            LOGE("Unexpected number of arguments, run command with no arguments for usage");
            return 1;
        }
        return RunWgcfRelay(argv[2], argv[3]);
    } else {
        LOGE("Unknown command: {}", argv[1]);
        LOGE("Run command with no arguments for usage");
        return 1;
    }
}
