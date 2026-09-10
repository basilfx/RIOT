# Webserver

## Introduction

This example implements a minimal HTTP/1.1 server on top of the SOCK TCP
API. It uses [picohttpparser](https://github.com/h2o/picohttpparser) to parse
the requests.

This example is on the large side (~ 128 KiB of flash, ~ 128 KiB of memory,
depening on board). The HTML could be minified and simplified, the assets could
be embedded or omitted, and more. However, the goal of this application is to
demonstrate what is be possible. That means serving resources over multiple
routes, including a JSON API to update the dashboard real-time.

## Limitations

A single thread accepts and serves the connections, one after another, so
a client is only attended to once the preceding one has been dealt with.
The stack meanwhile completes the handshake of further clients on the
remaining socks of the listening queue, of which there are
`SOCK_QUEUE_LEN`, and answers every connection attempt beyond that with a
reset, which a client reports as a refused connection. A browser opens
several connections in parallel for a single page, so it is normal to see
a few of them refused and retried while the page loads.

Such a queued connection is only reclaimed once this thread accepts it,
as nothing else times it out. A client that connects without sending a
request, as a browser does with the spare connections it keeps for reuse,
would therefore occupy its sock forever. To prevent that, a client that
does not complete its request within `REQUEST_TIMEOUT_MS` has its
connection closed. Reclaiming a queue full of such connections takes a
few seconds per sock, during which the server keeps refusing new ones.

## Usage

After flashing or starting the native instance, configure an address on
the network interface (see `ifconfig help`), then the server
automatically starts listening on port 8080. Use a browser to open the
dashboard page.

It can make use of both the GNRC and the LWIP network stack. The default
is GNRC, to choose LWIP set `LWIP=1` when compiling the example.
