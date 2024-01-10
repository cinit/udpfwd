//
// Created by sulfate on 2024-01-02.
//

#ifndef UDPFWD_ETHERPACKETSTRUCT_H
#define UDPFWD_ETHERPACKETSTRUCT_H

#include <cstdint>
#include <array>

#include "platform/arch/Endian.h"

namespace vether {

struct StreamPacketHeader {
    uint8_t flags = 0x10;
    uint8_t reserved1 = 0;
    // VXLAN does not have payload size here.
    union {
        std::array<uint8_t, 2> data;
        uint16_t u16be = 0;
    } payloadSize;
    union {
        uint32_t vni24be = 0;
        struct {
            std::array<uint8_t, 3> vni;
            uint8_t reserved2;
        };
    };

    [[nodiscard]] constexpr uint32_t GetNetId() const {
        return platform::arch::endian::ntoh32(vni24be) >> 8u;
    }
};

static_assert(sizeof(StreamPacketHeader) == 8, "StreamPacketHeader size mismatch");


static constexpr uint64_t AddressToIdentifier(std::array<uint8_t, 6> mac, uint16_t vlanId) {
    struct unused {
        std::array<uint8_t, 6> mac;
        uint16_t vlan;
    };
    static_assert(sizeof(unused) == 8, "size mismatch");
    // make them as if a union & memcpy
    // only consider little endian, LSB first
    // nobody uses big endian
    // 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07
    // M[0] M[1] M[2] M[3] M[4] M[5] V[0] V[1]
    // <<0  <<8  <<16 <<24 <<32 <<40 <<48 <<56
    return static_cast<uint64_t>(mac[0]) |
           static_cast<uint64_t>(mac[1]) << 8 |
           static_cast<uint64_t>(mac[2]) << 16 |
           static_cast<uint64_t>(mac[3]) << 24 |
           static_cast<uint64_t>(mac[4]) << 32 |
           static_cast<uint64_t>(mac[5]) << 40 |
           static_cast<uint64_t>(vlanId) << 48;
}

struct EtherFrameHeader {
    std::array<uint8_t, 6> destination;
    std::array<uint8_t, 6> source;
    uint16_t etherType;
    uint16_t vlanTag;

    [[nodiscard]] uint16_t constexpr GetVlanTag() const {
        using namespace platform::arch::endian;
        // We don't use 802.1ad QinQ, right?
        if (etherType == ntoh16(0x8100u)) {
            return ntoh16(vlanTag);
        } else {
            return 0;
        }
    }

    [[nodiscard]] uint16_t constexpr GetVlanId() const {
        return GetVlanTag() & 0x0FFFu;
    }

    [[nodiscard]] uint64_t constexpr GetDestinationIdentifier() const {
        return AddressToIdentifier(destination, GetVlanId());
    }

    [[nodiscard]] uint64_t constexpr GetSourceIdentifier() const {
        return AddressToIdentifier(source, GetVlanId());
    }
};

static_assert(sizeof(EtherFrameHeader) == 16, "EtherFrameHeader size mismatch");

}

#endif //UDPFWD_ETHERPACKETSTRUCT_H
