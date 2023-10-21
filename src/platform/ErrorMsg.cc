//
// Created by sulfate on 10/20/23.
//

#include "ErrorMsg.h"

#include <cmath>
#include <cerrno>
#include <cstring>

namespace platform {

std::string GetErrorMsg(int err) {
    return strerror(std::abs(err));
}

}
