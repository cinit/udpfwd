//
// Created by sulfate on 2024-01-09.
//

#ifndef UDPFWD_ASSERTIONS_H
#define UDPFWD_ASSERTIONS_H

#include "Log.h"
#include "platform/Runtime.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define CHECK(condition) \
    do { \
        if (!(condition)) [[unlikely]] { \
            ::platform::runtime::Abort("CHECK failed: " #condition " at " __FILE__ ":" TOSTRING(__LINE__)); \
        } \
    } while (false)

#endif //UDPFWD_ASSERTIONS_H
