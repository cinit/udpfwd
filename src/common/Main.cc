//
// Created by sulfate on 2023-10-24.
//

#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <string_view>
#include <functional>

#include "platform/log/Log.h"
#include "DebugConfig.h"

#include "udpfwd/UdpForward.h"
#include "wgcf/WgcfRelay.h"


static constexpr auto LOG_TAG = "main";


class Applet {
public:
    const char* name;
    std::function<int(std::string_view, std::vector<std::string_view>)> entry;
};

static int udpfwd_entry(std::string_view argv0, std::vector<std::string_view> args) {
    if (args.empty()) {
        LOGE("Usage for udpfwd:");
        LOGE("    {} udpfwd -f [-d] path/to/config.txt", argv0);
        LOGE("    Example: {} udpfwd -f config.txt", argv0);
        return 1;
    }
    bool debug = false;
    std::erase_if(args, [&debug](std::string_view arg) {
        if (arg == "-d") {
            debug = true;
            return true;
        }
        return false;
    });
    if (debug) {
        ::debugconfig::SetDebug(true);
    }
    if (args.size() != 1) {
        LOGE("Unexpected number of arguments, run command with no arguments for usage");
        return 1;
    }
    return udpfwd::RunUdpForwarder(args[0]);
}

static int wgcfrelay_entry(std::string_view argv0, std::vector<std::string_view> args) {
    if (args.empty()) {
        LOGE("Usage for wgcfrelay:");
        LOGE("    {} udpfwd -r [-d] listen:port forward:port", argv0);
        LOGE("    Example: {} udpfwd -r [::]:51820 10.0.0.1:37000", argv0);
        return 1;
    }
    bool debug = false;
    std::erase_if(args, [&debug](std::string_view arg) {
        if (arg == "-d") {
            debug = true;
            return true;
        }
        return false;
    });
    if (debug) {
        ::debugconfig::SetDebug(true);
    }
    // 2 arguments expected
    if (args.size() != 2) {
        LOGE("Unexpected number of arguments, run command with no arguments for usage");
        return 1;
    }
    return wgrelay::RunWgcfRelay(args[0], args[1]);
}

static const std::array applets = {
        Applet{"udpfwd", udpfwd_entry},
        Applet{"wgcfrelay", wgcfrelay_entry}
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        LOGE("Usage: {} <applet> [args...]", argv[0]);
        LOGE("Available applets:");
        for (const auto& applet: applets) {
            LOGE("    {}", applet.name);
        }
        return 1;
    }
    std::string_view appletName = argv[1];
    auto it = std::find_if(applets.begin(), applets.end(), [&appletName](const Applet& applet) {
        return applet.name == appletName;
    });
    if (it == applets.end()) {
        LOGE("Unknown applet: {}", appletName);
        LOGE("Run {} with no arguments for usage", argv[0]);
        return 1;
    }
    std::vector<std::string_view> args;
    for (int i = 2; i < argc; i++) {
        args.emplace_back(argv[i]);
    }
    return it->entry(argv[0], args);
}
