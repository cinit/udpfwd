//
// Created by sulfate on 10/24/23.
//

#ifndef UDPFWD_WGCFRELAYWORKER_H
#define UDPFWD_WGCFRELAYWORKER_H

#include "WgcfRelayContext.h"

namespace wgrelay {

int SetupSockets(WgcfRelayContext& context, net::INetSocketAddress listenAddress,
                 net::INetSocketAddress forwardAddress);

int RunWorker(WgcfRelayContext& context);

}

#endif //UDPFWD_WGCFRELAYWORKER_H
