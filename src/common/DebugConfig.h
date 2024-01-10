//
// Created by sulfate on 10/25/23.
//

#ifndef UDPFWD_DEBUGCONFIG_H
#define UDPFWD_DEBUGCONFIG_H

namespace debugconfig {

bool IsDebug() noexcept;

void SetDebug(bool debug) noexcept;

}

#define DLOGV(fmt, ...) \
    do { \
        if (::debugconfig::IsDebug()) [[unlikely]] { \
            LOGV(fmt, ##__VA_ARGS__); \
        } \
    } while (false)

#endif //UDPFWD_DEBUGCONFIG_H
