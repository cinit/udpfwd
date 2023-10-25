//
// Created by sulfate on 10/25/23.
//

#include "DebugConfig.h"


namespace debugconfig {

static bool sDebug = false;

void SetDebug(bool debug) noexcept {
    sDebug = debug;
}

bool IsDebug() noexcept {
    return sDebug;
}

}
