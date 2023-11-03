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

It's probably not a good idea to use this project on your VPS, because almost all VPS use IP addresses that are intended
for data centers, and these IP addresses are typically considered as "bad/risky". Therefore, it's very likely that you
will encounter a lot of CAPTCHAs (Cloudflare Turnstile) when you access websites through your relay server.

Even the following command will trigger CAPTCHA:

```shell
# This is a public IP-echoing service.
curl ip.sb
```

It fails with a 403 response:

```html
<!DOCTYPE html>
<html lang="en-US">
<head><title>Just a moment...</title>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=Edge">
    <meta name="robots" content="noindex,nofollow">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <link href="/cdn-cgi/styles/challenges.css" rel="stylesheet">
</head>
<body class="no-js">
<div class="main-wrapper" role="main">
    <div class="main-content">
        <noscript>
            <div id="challenge-error-title">
                <div class="h2"><span class="icon-wrapper"><div class="heading-icon warning-icon"></div></span><span
                        id="challenge-error-text">Enable JavaScript and cookies to continue</span></div>
            </div>
        </noscript>
    </div>
</div>
<script>(function () {
    window._cf_chl_opt = {
        cvId: '2',
        cZone: "ip.sb",
        cType: 'managed',
        cNounce: 'REDACTED',
        cRay: 'REDACTED',
        cHash: 'REDACTED',
        // more...
        cUPMDTk: "\/?__cf_chl_tk=REDACTED"
    };
    var cpo = document.createElement('script');
    cpo.src = '/cdn-cgi/challenge-platform/h/b/orchestrate/chl_page/v1?ray=REDACTED';
    window._cf_chl_opt.cOgUHash = location.hash === '' && location.href.indexOf('#') !== -1 ? '#' : location.hash;
    window._cf_chl_opt.cOgUQuery = location.search === '' && location.href.slice(0, location.href.length - window._cf_chl_opt.cOgUHash.length).indexOf('?') !== -1 ? '?' : location.search;
    if (window.history && window.history.replaceState) {
        var ogU = location.pathname + window._cf_chl_opt.cOgUQuery + window._cf_chl_opt.cOgUHash;
        history.replaceState(null, null, "\/?__cf_chl_rt_tk=REDACTED" + window._cf_chl_opt.cOgUHash);
        cpo.onload = function () {
            history.replaceState(null, null, ogU);
        }
    }
    document.getElementsByTagName('head')[0].appendChild(cpo);
}());</script>
</body>
</html>
```

You don't want to solve CAPTCHA every time you access a website, do you?

## Disclaimer

This project is just a Proof of Concept. It's neither production-ready nor well-tested. Use it at your own risk.

This project is not affiliated with Cloudflare in any way.

The software is provided "AS IS" without any warranty, express or implied, including but not limited to the warranties
of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or Cloudflare,
Inc. be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising
from, out of or in connection with the software or the use or other dealings in the software.
