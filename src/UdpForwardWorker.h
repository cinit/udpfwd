//
// Created by sulfate on 10/20/23.
//

#ifndef UDPFWD_UDPFORWARDWORKER_H
#define UDPFWD_UDPFORWARDWORKER_H

#include "UdpForwardContext.h"

namespace udpfwd {

int SetupSockets(UdpForwardContext& context, std::vector<UdpForwardTargetInfo> targets);

int RunWorker(UdpForwardContext& context);

}


#endif //UDPFWD_UDPFORWARDWORKER_H
