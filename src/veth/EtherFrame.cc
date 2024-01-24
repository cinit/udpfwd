//
// Created by sulfate on 2024-01-09.
//

#include "EtherFrame.h"

#include <fmt/format.h>

vether::EtherFrame vether::EtherFrame::Clone() const {
    auto another = BufferPool::ObtainBuffer();
    memcpy(another->data(), mData->data(), mSize);
    return EtherFrame(std::move(another), mSize);
}

std::string vether::EtherFrame::GetSourceMacString() const {
    const auto& mac = GetSourceMac();
    return fmt::format("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
                       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

std::string vether::EtherFrame::GetDestinationMacString() const {
    const auto& mac = GetDestinationMac();
    return fmt::format("{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
                       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
