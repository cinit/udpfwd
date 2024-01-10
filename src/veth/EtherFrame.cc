//
// Created by sulfate on 2024-01-09.
//

#include "EtherFrame.h"

vether::EtherFrame vether::EtherFrame::Clone() const {
    auto another = EtherPool::ObtainBuffer();
    memcpy(another->data(), mData->data(), mSize);
    return EtherFrame(std::move(another), mSize);
}
