//
// Created by sulfate on 2023-11-16.
//

#include "Runtime.h"

#include <string>

#include <unistd.h>

#ifdef __BIONIC__

#include <android/set_abort_message.h>

#endif

namespace platform::runtime {

[[noreturn]] void Abort(std::string_view msg) noexcept {
    auto copy = std::string(msg);
#ifdef __BIONIC__
    android_set_abort_message(copy.c_str());
#else
    write(STDERR_FILENO, copy.c_str(), copy.size());
    write(STDERR_FILENO, "\n", 1);
#endif
    abort();
}

int GetPid() noexcept {
    return (int) getpid();
}

int GetTid() noexcept {
    return (int) gettid();
}

int GetUid() noexcept {
    return (int) getuid();
}

int GetEuid() noexcept {
    return (int) geteuid();
}

} // namespace runtime
