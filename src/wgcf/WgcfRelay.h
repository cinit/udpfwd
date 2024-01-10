//
// Created by sulfate on 10/24/23.
//

#ifndef UDPFWD_WGCFRELAYMAIN_H
#define UDPFWD_WGCFRELAYMAIN_H

#include <string>

namespace wgrelay {

int RunWgcfRelay(std::string_view listenAddress, std::string_view forwardAddress);

}

#endif //UDPFWD_WGCFRELAYMAIN_H
