//
// Created by sulfate on 2024-01-09.
//

#ifndef UDPFWD_BUFFERPOOL_H
#define UDPFWD_BUFFERPOOL_H

#include <cstdint>
#include <array>
#include <memory>

namespace vether {

class BufferPool {
public:
    static constexpr size_t kMaxBufferSize = 16 * 1024;

    BufferPool() = delete;

    [[nodiscard]] static std::unique_ptr<std::array<uint8_t, kMaxBufferSize>> ObtainBuffer();

    static void ReturnBuffer(std::unique_ptr<std::array<uint8_t, kMaxBufferSize>> buffer) noexcept;

};

}

#endif //UDPFWD_BUFFERPOOL_H
