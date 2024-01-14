//
// Created by sulfate on 2024-01-09.
//

#ifndef UDPFWD_ETHERFRAME_H
#define UDPFWD_ETHERFRAME_H

#include <cstdint>
#include <array>
#include <string>
#include <memory>
#include <cstring>
#include <algorithm>

#include "EtherPool.h"
#include "EtherPacketStruct.h"

namespace vether {

class EtherFrame {
public:
    // no copy to avoid unintended copy
    EtherFrame(const EtherFrame&) = delete;

    EtherFrame& operator=(const EtherFrame&) = delete;

    // allow move
    EtherFrame(EtherFrame&&) = default;

    EtherFrame& operator=(EtherFrame&&) = default;

private:
    std::unique_ptr<std::array<uint8_t, kMaxFrameSize>> mData = EtherPool::ObtainBuffer();
    size_t mSize = 0;

public:
    EtherFrame() = default;

    explicit EtherFrame(size_t size) noexcept: mSize(size) {}

    explicit EtherFrame(std::unique_ptr<std::array<uint8_t, kMaxFrameSize>> data, size_t size) noexcept
            : mData(std::move(data)), mSize(size) {}

    inline ~EtherFrame() noexcept {
        EtherPool::ReturnBuffer(std::move(mData));
    }

    [[nodiscard]] inline auto data() const noexcept {
        return mData->data();
    }

    [[nodiscard]] inline auto data() noexcept {
        return mData->data();
    }

    [[nodiscard]] inline auto size() const noexcept {
        return mSize;
    }

    inline void SetSize(size_t size) {
        mSize = std::min(uint16_t(size), kMaxFrameSize);
    }

    [[nodiscard]] inline uint16_t GetVlanTag() const {
        return reinterpret_cast<const EtherFrameHeader*>(data())->GetVlanTag();
    }

    [[nodiscard]] inline uint16_t GetVlanId() const {
        return reinterpret_cast<const EtherFrameHeader*>(data())->GetVlanId();
    }

    [[nodiscard]] inline uint64_t GetDestinationIdentifier() const {
        return reinterpret_cast<const EtherFrameHeader*>(data())->GetDestinationIdentifier();
    }

    [[nodiscard]] inline uint64_t GetSourceIdentifier() const {
        return reinterpret_cast<const EtherFrameHeader*>(data())->GetSourceIdentifier();
    }

    [[nodiscard]] inline std::array<uint8_t, 6> GetDestinationMac() const {
        return reinterpret_cast<const EtherFrameHeader*>(data())->destination;
    }

    [[nodiscard]] inline std::array<uint8_t, 6> GetSourceMac() const {
        return reinterpret_cast<const EtherFrameHeader*>(data())->source;
    }

    [[nodiscard]] EtherFrame Clone() const;

    [[nodiscard]] std::string GetSourceMacString() const;

    [[nodiscard]] std::string GetDestinationMacString() const;

};

}

#endif //UDPFWD_ETHERFRAME_H
