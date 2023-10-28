# WARP-WireGuard Relay

## Introduction

[Cloudflare WARP](https://1.1.1.1/) adopts ECMP (Equal-Cost Multi-Path routing) as a load-balancing strategy to
achieve high availability and high throughput. However, the ECMP strategy is not suitable for all scenarios. For
example, in the case of a single TCP connection, the ECMP strategy will cause the connection to be interrupted when the
network path changes, resulting in a poor user experience when you switch from Wi-Fi to a cellular network and vice
versa.

Although [this blog](https://blog.cloudflare.com/warp-technical-challenges/) says that Cloudflare has implemented a
strategy to avoid this problem, where servers in Cloudflare's data centers will act as a relay to ensure that the
correct WireGuard packets will be forwarded to the correct server which owns the WireGuard session if a client id is
correctly set in the reserved field of the WireGuard packet. However, as far as I have tested, this strategy does not
work. Therefore, I wrote this project to implement a WireGuard relay server to make my WARP connection more stable.

These ECMP routers in Cloudflare data center use a combination of (Source IP, Source Port, Destination IP, and
Destination Port) to match a packet to a server. Since the destination IP and port for Cloudflare's WARP are always the
same, the relay server only needs to keep the source IP and source port stable to ensure that the packets from the same
client will be forwarded to the same server. In the case of a VPS, the source IP is stable, so the relay server only
needs to keep the source port stable for each client/session.

## Implementation

This project works as a WireGuard relay server. It receives WireGuard packets from clients and forwards them to the
server. When receiving a packet from the client, the relay server will check the client ID (aka, the reserved field of
the WireGuard packet) to determine which session the packet belongs to. If the client ID is not set, the relay server
just drops the packet, because both the sender and receiver field in a WireGuard packet are random and if an ephemeral
key expires, the handshake procedure starts again and the sender and receiver fields are reset to random values.
Therefore, I don't think I can use the sender and receiver fields to determine which session the packet belongs to.
If the client ID is set, the relay server will check if the client ID is in the session table. If the client ID is in
the session table, the relay server updates the ip:port mapping in the session table and forwards the packet to the
server. If the client ID is not in the session table, the relay server will create a new session and pick a random port
to forward the packet to the server. When receiving a packet from the server, the relay server will check the
destination port to determine which session the packet belongs to. If the destination port is in the session table, the
relay server will forward the packet to the client. If a session is inactive for more than 10 minutes, the relay server
will delete the session.

The relay server will neither try to decrypt the WireGuard packets nor try to modify the WireGuard packets. It just
forwards the packets as they are. Therefore, the relay server does not need to know the public/private key pair of the
client or the server, nor does it need to know the pre-shared key. It's simply a UDP relay server.

## Usage

### Build

This project uses CMake as the build system. To build this project, you just need to run the following commands:

```shell
mkdir build && cd build
cmake ..
make
```

### Run

Throw the artifact udpfwd binary to your VPS, and run it with the following command:

```shell
./udpfwd -r [::]:2408 162.159.192.1:2408
```

It doesn't need any privilege to run. Run it without any argument to see the usage.

Don't forget to add a firewall rule to allow UDP traffic to the port you specified.

## Note

This project is only compatible with WireGuard with a none-zero reserved field. If you want to forward other UDP
traffic,
see the usage for more details.

## Disclaimer

This project is just a Proof of Concept. It's neither production-ready nor well-tested. Use it at your own risk.

This project is not affiliated with Cloudflare in any way.

The software is provided "AS IS" without any warranty, express or implied, including but not limited to the warranties
of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or Cloudflare,
Inc. be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising
from, out of or in connection with the software or the use or other dealings in the software.
