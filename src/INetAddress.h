//
// Created by sulfate on 10/20/23.
//

#ifndef UDPFWD_INETADDRESS_H
#define UDPFWD_INETADDRESS_H

#include <cstdint>
#include <string>
#include <array>
#include <span>
#include <string_view>

#include "fmt/format.h"


namespace net {

class INetAddress {
public:
    enum class INetType : uint16_t {
        kNone = 0,
        kIpv4 = 4,
        kIpv6 = 6
    };

private:
    INetType mType = INetType::kNone;

    // big endian, as is
    union {
        std::array<uint8_t, 4> mAddr4;
        uint32_t mAddr4AsBigEndian;
        std::array<uint8_t, 16> mAddr6 = {};
    };

public:

    INetAddress() noexcept = default;

    INetAddress(INetType type, std::span<const uint8_t> addr) noexcept {
        if (type == INetType::kIpv4 && addr.size() == 4) {
            mType = type;
            mAddr4 = {addr[0], addr[1], addr[2], addr[3]};
        } else if (type == INetType::kIpv6 && addr.size() == 16) {
            mType = type;
            mAddr6 = {addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7],
                      addr[8], addr[9], addr[10], addr[11], addr[12], addr[13], addr[14], addr[15]};
        } else {
            mType = INetType::kNone;
        }
    }

    explicit INetAddress(std::span<const uint8_t, 4> addr) noexcept
            : mType(INetType::kIpv4), mAddr4{addr[0], addr[1], addr[2], addr[3]} {}

    explicit INetAddress(std::span<const uint8_t, 16> addr) noexcept
            : mType(INetType::kIpv6),
              mAddr6{addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7],
                     addr[8], addr[9], addr[10], addr[11], addr[12], addr[13], addr[14], addr[15]} {}

    INetAddress(const INetAddress&) noexcept = default;

    INetAddress& operator=(const INetAddress&) noexcept = default;

    [[nodiscard]] static INetAddress FromString(std::string_view address);

    [[nodiscard]] inline bool IsIpv4() const noexcept {
        return mType == INetType::kIpv4;
    }

    [[nodiscard]] inline bool IsIpv6() const noexcept {
        return mType == INetType::kIpv6;
    }

    [[nodiscard]] inline bool IsValid() const noexcept {
        return IsIpv4() || IsIpv6();
    }

    [[nodiscard]] std::string ToString() const;

    [[nodiscard]] std::span<const uint8_t> GetBytes() const noexcept {
        if (IsIpv4()) {
            return {mAddr4.data(), mAddr4.size()};
        } else if (IsIpv6()) {
            return {mAddr6.data(), mAddr6.size()};
        }
        return {};
    }

    [[nodiscard]] inline uint32_t GetIpv4() const noexcept {
        return mAddr4AsBigEndian;
    }

    [[nodiscard]] inline std::array<uint8_t, 16> GetIpv6() const noexcept {
        return mAddr6;
    }

    [[nodiscard]] inline INetType GetType() const noexcept {
        return mType;
    }

    [[nodiscard]] inline bool operator==(const INetAddress& other) const noexcept {
        if (mType != other.mType) {
            return false;
        }
        if (mType == INetType::kIpv4) {
            return mAddr4AsBigEndian == other.mAddr4AsBigEndian;
        } else if (mType == INetType::kIpv6) {
            return mAddr6 == other.mAddr6;
        }
        return true;
    }

};

}

// fmt helper

template<>
struct fmt::formatter<net::INetAddress> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const net::INetAddress& addr, FormatContext& ctx) {
        return format_to(ctx.out(), "{}", addr.ToString());
    }
};

#endif //UDPFWD_INETADDRESS_H
