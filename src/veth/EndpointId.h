//
// Created by sulfate on 2024-01-12.
//

#ifndef UDPFWD_ENDPOINTID_H
#define UDPFWD_ENDPOINTID_H

#include <cstdint>

#include <fmt/format.h>

namespace vether {

class EndpointId {
public:
    uint64_t id;

    // +-----+-----+-----+-----+-----+-----+-----+-----+
    //     56    48    40    32    24    16     8     0
    // +-----+-----+-----+-----+-----+-----+-----+-----+
    // | u8  |        u24      |          u32          |
    // +-----+-----------------+-----------------------+
    // |type |   network id    |   stream id (or tap)  |
    // +-----+-----------------+-----------------------+

    explicit constexpr EndpointId(uint64_t id) noexcept: id(id) {}

    static constexpr uint8_t kTypeNone = 0;
    static constexpr uint8_t kTypeTap = 1;
    static constexpr uint8_t kTypeStream = 2;
    static constexpr uint8_t kTypeShadow = 4;

    explicit constexpr operator uint64_t() const noexcept {
        return id;
    }

    [[nodiscard]] constexpr bool operator==(const EndpointId& other) const noexcept = default;

    [[nodiscard]] constexpr bool operator==(uint64_t other) const noexcept {
        return id == other;
    }

    [[nodiscard]] constexpr uint8_t GetType() const noexcept {
        return uint8_t(id >> 56);
    }

    [[nodiscard]] constexpr uint32_t GetNetworkId() const noexcept {
        return uint32_t(id >> 24);
    }

    [[nodiscard]] constexpr uint32_t GetStreamId() const noexcept {
        return uint32_t(id);
    }

    [[nodiscard]] constexpr uint32_t GetTapId() const noexcept {
        return uint32_t(id);
    }

    [[nodiscard]] constexpr bool IsNone() const noexcept {
        return GetType() == kTypeNone;
    }

    [[nodiscard]] constexpr bool IsTap() const noexcept {
        return GetType() == kTypeTap;
    }

    [[nodiscard]] constexpr bool IsStream() const noexcept {
        return GetType() == kTypeStream;
    }

    [[nodiscard]] constexpr bool IsShadow() const noexcept {
        return GetType() == kTypeShadow;
    }

};

static_assert(sizeof(EndpointId) == 8, "EndpointId must be 8 bytes");

namespace EndpointIdConstants {

static constexpr auto kNone = EndpointId(0);
static constexpr auto kShadow = EndpointId(uint64_t(EndpointId::kTypeShadow) << 56);

}

}

// std::hash and std::equal_to

namespace std {

template<>
struct hash<vether::EndpointId> {
    size_t operator()(const vether::EndpointId& endpointId) const noexcept {
        return std::hash<uint64_t>()(uint64_t(endpointId));
    }
};

template<>
struct equal_to<vether::EndpointId> {
    bool operator()(const vether::EndpointId& lhs, const vether::EndpointId& rhs) const noexcept {
        return lhs == rhs;
    }
};

}

// fmt helper

template<>
struct fmt::formatter<vether::EndpointId> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const vether::EndpointId& endpointId, FormatContext& ctx) {
        return format_to(ctx.out(), "{:016x}", uint64_t(endpointId));
    }
};

#endif //UDPFWD_ENDPOINTID_H
