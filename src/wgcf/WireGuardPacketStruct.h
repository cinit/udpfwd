//
// Created by sulfate on 10/25/23.
//

#ifndef UDPFWD_WIREGUARDPACKETSTRUCT_H
#define UDPFWD_WIREGUARDPACKETSTRUCT_H

#include <cstdint>
#include <array>

namespace wgrelay::wireguard {

enum class PacketType : uint8_t {
    kUnknown = 0,
    kHandshakeInitiation = 1,
    kHandshakeResponse = 2,
    kCookieReplyMessage = 3,
    kTransportDataMessage = 4
};

struct WireGuardPacketHeader {
    union {
        struct {
            PacketType type; // 1
            std::array<uint8_t, 3> reserved;
        };
        uint32_t typeAndReserved;
    };
};

static_assert(sizeof(WireGuardPacketHeader) == 4, "WireGuardPacketHeader size mismatch");

struct HandshakeInitiation {
    union {
        struct {
            PacketType type; // 1
            std::array<uint8_t, 3> reserved;
        };
        uint32_t typeAndReserved;
    };
    uint32_t sender;
    std::array<uint8_t, 32> ephemeral;
    std::array<uint8_t, 48> encryptedStatic;
    std::array<uint8_t, 28> encryptedTimestamp;
    std::array<uint8_t, 16> mac1;
    std::array<uint8_t, 16> mac2;
};

static_assert(sizeof(HandshakeInitiation) == 148, "HandshakeInitiation size mismatch");

struct HandshakeResponse {
    union {
        struct {
            PacketType type; // 2
            std::array<uint8_t, 3> reserved;
        };
        uint32_t typeAndReserved;
    };
    uint32_t sender;
    uint32_t receiver;
    std::array<uint8_t, 32> ephemeral;
    std::array<uint8_t, 16> encryptedEmpty;
    std::array<uint8_t, 16> mac1;
    std::array<uint8_t, 16> mac2;
};

static_assert(sizeof(HandshakeResponse) == 92, "HandshakeResponse size mismatch");

struct CookieReplyMessage {
    union {
        struct {
            PacketType type; // 3
            std::array<uint8_t, 3> reserved;
        };
        uint32_t typeAndReserved;
    };
    uint32_t receiver;
    std::array<uint8_t, 24> nonce;
    std::array<uint8_t, 32> encryptedCookie;
};

static_assert(sizeof(CookieReplyMessage) == 64, "CookieReplyMessage size mismatch");

struct TransportDataMessageHeader {
    union {
        struct {
            PacketType type; // 4
            std::array<uint8_t, 3> reserved;
        };
        uint32_t typeAndReserved;
    };
    uint32_t receiver;
    uint64_t counter;
    // encrypted data follows
};

static_assert(sizeof(TransportDataMessageHeader) == 16, "TransportDataMessageHeader size mismatch");

}


#endif //UDPFWD_WIREGUARDPACKETSTRUCT_H
