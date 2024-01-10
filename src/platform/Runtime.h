//
// Created by sulfate on 2023-11-16.
//

#ifndef UTIL_RUNTIME_H
#define UTIL_RUNTIME_H

#include <string_view>
#include <cstdint>

namespace platform::runtime {

[[noreturn]] void Abort(std::string_view msg) noexcept;

[[nodiscard]] int GetPid() noexcept;

[[nodiscard]] int GetTid() noexcept;

[[nodiscard]] int GetUid() noexcept;

[[nodiscard]] int GetEuid() noexcept;

} // namespace runtime

#endif //UTIL_RUNTIME_H
