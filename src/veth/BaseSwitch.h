//
// Created by sulfate on 2024-01-11.
//

#ifndef UDPFWD_BASESWITCH_H
#define UDPFWD_BASESWITCH_H

#include <cstdint>
#include <vector>

#include "EndpointId.h"
#include "EtherFrame.h"

namespace vether {

class VEtherForwardContext;

class BaseSwitch {
public:
    BaseSwitch() = default;

    virtual ~BaseSwitch() = default;

    // neither copy nor move
    BaseSwitch(const BaseSwitch&) = delete;

    BaseSwitch& operator=(const BaseSwitch&) = delete;

    BaseSwitch(BaseSwitch&&) = delete;

    BaseSwitch& operator=(BaseSwitch&&) = delete;

    [[nodiscard]] virtual std::vector<EndpointId> GetEndpoints() const = 0;

    virtual void AddEndpoint(EndpointId endpointId) = 0;

    virtual bool RemoveEndpoint(EndpointId endpointId) = 0;

    virtual void ClearEndpoints() = 0;

    [[nodiscard]] virtual bool HasEndpoint(EndpointId endpointId) const = 0;

    virtual void HandleReceiveFrame(VEtherForwardContext& ctx, std::shared_ptr<EtherFrame> frame, EndpointId srcEp) = 0;

};

}

#endif //UDPFWD_BASESWITCH_H
