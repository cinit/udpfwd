//
// Created by sulfate on 2024-01-09.
//

#ifndef UDPFWD_ETHERPOOL_H
#define UDPFWD_ETHERPOOL_H

#include <cstdint>
#include <array>
#include <memory>

namespace vether {

static constexpr uint16_t kMaxFrameSize = 16 * 1024;

class EtherPool {
public:
    EtherPool() = delete;

    [[nodiscard]] static std::unique_ptr<std::array<uint8_t, kMaxFrameSize>> ObtainBuffer();

    static void ReturnBuffer(std::unique_ptr<std::array<uint8_t, kMaxFrameSize>> buffer) noexcept;

};

}

#endif //UDPFWD_ETHERPOOL_H
