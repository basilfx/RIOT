# gnrc_tcp listening queue defects

Working notes, not part of the RIOT documentation. Written 2026-09-10 while
debugging `examples/networking/misc/webserver` on branch `feature/sock_http`.
Everything below was reproduced on hardware (slstk3701a, IPv4 over DHCP,
port 8080), not derived from reading the code alone.

Delete this file before the branch is submitted upstream; the content belongs
in the commit messages and the pull request description.

## Reported symptom

Refreshing the served page in a browser too quickly killed the server. The
browser then reported `connection refused` for every further request and no
request was served again. The RIOT shell stayed responsive, `ifconfig` showed
link up with a valid DHCP lease, and only a reboot brought the server back.

## Root causes

Three distinct defects, in decreasing order of impact. The first is what the
user hit; the second and third were found while testing the fix for the first.

### 1. An unaccepted connection is never reclaimed, so the queue fills up permanently

A SYN only matches a TCB in `FSM_STATE_LISTEN` (see the TCB lookup in
`_receive()`, `sys/net/gnrc/transport_layer/tcp/gnrc_tcp_eventloop.c`). Once
every TCB of the listening queue has left that state, the stack answers every
new SYN with a reset, which a client reports as a refused connection.

Two things conspire to fill the queue and keep it full:

- `_transition_to()` in `gnrc_tcp_fsm.c` schedules
  `CONFIG_GNRC_TCP_CONNECTION_TIMEOUT_DURATION_MS` when a listening TCB enters
  `FSM_STATE_SYN_RCVD`, and *unschedules* it again on `FSM_STATE_ESTABLISHED`.
  A blocking user call schedules a connection timeout of its own, so a
  connection that the stack has completed but the application has not yet
  taken out of the queue with `sock_tcp_accept()` has **no timeout of any
  kind**. It occupies its TCB until the application accepts it, however long
  that takes.

- The example served connections from a single thread and read the request
  with `SOCK_NO_TIMEOUT`. One client that opens a connection and sends nothing
  therefore parked that thread forever, so it never got round to accepting the
  other queued connections either.

A browser triggers exactly this. It opens several connections in parallel for
one page and keeps spare ones open for reuse, and a spare connection sends
nothing. With `SOCK_QUEUE_LEN` at 4, four such connections are enough.

The TCB dump of a wedged board, taken over JLink (see *Diagnosing this again*
below), shows the picture unambiguously:

```
tcb[0] state=4 status=0x07 lport=8080 pport=59049 rcv_wnd=576 mbox=(nil)
tcb[1] state=4 status=0x07 lport=8080 pport=59048 rcv_wnd=576 mbox=(nil)
tcb[2] state=4 status=0x07 lport=8080 pport=59043 rcv_wnd=576 mbox=(nil)
tcb[3] state=4 status=0x0b lport=8080 pport=59042 rcv_wnd=576 mbox=0x20001778
```

`state=4` is `FSM_STATE_ESTABLISHED`, so **not one TCB is in `LISTEN`**.
`status` bit `0x08` is `STATUS_ACCEPTED`, so only `tcb[3]` was handed to the
application, and its non-NULL `mbox` shows the server thread parked in
`sock_tcp_read()` on it. The other three were completed by the stack and are
waiting for an `accept()` that never comes.

Measured before the fix: five silent sockets held the server refused for
**210 s and counting**, with no recovery, and it stayed refused even after the
client closed them. This is the "reboot is the only fix" the report describes.

### 2. A listening TCB re-armed itself while the application still held it

`_transition_to(FSM_STATE_CLOSED)` re-opened *any* TCB carrying
`STATUS_LISTENING` straight back into `FSM_STATE_LISTEN`, without looking at
`STATUS_ACCEPTED`.

So when a peer reset its connection between two `sock_tcp_read()` calls, the
TCB went back into the listening pool and the very next SYN was accepted into
it, while the application was still working on what it believed was the
previous connection. The same `sock_tcp_t` had silently become a **different
connection to a different peer**, and the application read one peer's bytes
concatenated with another's. There is no API by which an application could
notice this.

Traced on the board with temporary instrumentation in the example. Six
connections each sent the nine bytes `ZZZZZZZZZ` and were then reset; a
separate, well formed request followed:

```
DBG accept sock=3
DBG sock=3 read=9  [ZZZZZZZZZ]                     <- the aborted connection
DBG sock=3 read=38 [GET /api/version HTTP/1.1...]  <- a different peer's connection
Request: ZZZZZZZZZGET /api/version
```

The effect was deterministic and easy to mistake for a parser bug: exactly one
of every two probes got a bogus `400 Bad Request`, and the status depended on
what the *aborted* connection had sent (`GET / HTT` produced 400,
`ZZZZZZZZZ` produced 404), which is what proved the bytes were crossing
connections rather than being mis-parsed.

### 3. A re-armed listening TCB kept the previous connection's unread bytes

A listening TCB owns its receive buffer for its whole lifetime;
`_gnrc_tcp_rcvbuf_get_buffer()` only calls `ringbuffer_init()` when
`rcv_buf_raw == NULL`, which is not the case on re-use. Neither the ring
buffer nor `rcv_wnd` was reset when a TCB re-entered `LISTEN`.

Two consequences. Bytes the user never read survived into the next connection
accepted on that TCB, and `rcv_wnd` stayed reduced by their number, because
`_fsm_call_recv()` only restores the window to full when
`ringbuffer_get_free() >= CONFIG_GNRC_TCP_MSS`, that is, only when the buffer
has been drained completely. A TCB caught mid-wedge showed this directly:

```
tcb[2] state=4 status=0x07 rcv_wnd=237 rcv_buf(avail=339 free=237)
```

For IPv4 `CONFIG_GNRC_TCP_MSS`, `CONFIG_GNRC_TCP_DEFAULT_WINDOW` and
`GNRC_TCP_RCV_BUF_SIZE` are all 576, so any residue at all keeps the window
below its full size until the buffer happens to empty.

Defect 2 masks defect 3 in most tests, since both surface as the next
connection reading foreign bytes. They are independent and both need fixing.

## Changes made

### Stack

`sys/net/gnrc/transport_layer/tcp/gnrc_tcp_fsm.c`

- `_transition_to()`, `FSM_STATE_CLOSED`: re-open as listening only when
  `STATUS_ACCEPTED` is clear. A TCB the user still holds stays `CLOSED` until
  the user closes it. This is safe because a SYN only matches a TCB in
  `LISTEN`, so such a TCB cannot pick up a new connection, while segments from
  the old peer still match on the four-tuple and correctly get a reset.
- `_transition_to()`, `FSM_STATE_LISTEN`: re-initialise `rcv_buf` and reset
  `rcv_wnd` to `CONFIG_GNRC_TCP_DEFAULT_WINDOW`.
- `_fsm_call_close()`: clear `STATUS_ACCEPTED` on entry, and add a branch for
  a TCB already in `CLOSED`, which re-opens it as listening or, if
  `STATUS_LISTENING` was cleared meanwhile (the `gnrc_tcp_stop_listen()`
  path), performs the deferred `CLOSED` transition so the TCB leaves the
  active list and releases its receive buffer.
- `_fsm_call_abort()`: clear `STATUS_ACCEPTED` before the transition to
  `CLOSED`, so the TCB is returned to its queue.

`sys/net/gnrc/transport_layer/tcp/gnrc_tcp.c`

- `_close()`: a TCB already in `CLOSED` used to return immediately. It now
  dispatches `FSM_EVENT_CALL_CLOSE` when `STATUS_ACCEPTED` is set, which is
  what returns a deferred TCB to its queue.
- `_abort()`: likewise dispatches `FSM_EVENT_CALL_ABORT` for an accepted TCB
  that is already `CLOSED`.

### Example

`examples/networking/misc/webserver/main.c`

- The request is read with `REQUEST_TIMEOUT_MS` (2 s) instead of
  `SOCK_NO_TIMEOUT`, so a client that never completes a request cannot park
  the server thread. This is what lets the application walk the queue and
  reclaim the stuck connections of defect 1.
- `_drain_and_close()` is bounded by `LINGER_MAX_READS`, so a client that
  keeps sending cannot hold the connection either.
- `SOCK_TIMEOUT_MS()` works around the unit disagreement described below.

`examples/networking/misc/webserver/Makefile`

- `CONFIG_GNRC_TCP_CONNECTION_TIMEOUT_DURATION_MS=10000`. The two minute
  default is the upper bound on every blocking `gnrc_tcp` call, so a peer that
  vanishes without closing its connection, for instance during a write or
  during `gnrc_tcp_close()`, holds the only server thread for that long.

`examples/networking/misc/webserver/README.md`

- A `Limitations` section describing the single threaded design, why a few
  refused connections during a page load are normal, and why reclaiming a
  queue full of idle connections takes a few seconds per sock.

## Related API trap, not fixed here

`sys/include/net/sock/tcp.h` documents the timeouts of `sock_tcp_read()` and
`sock_tcp_accept()` **in microseconds**. The lwIP port honours that
(`lwip_sock_timeout_ms()`), but the GNRC port passes the value straight to
`gnrc_tcp_recv()`/`gnrc_tcp_accept()`, which expect **milliseconds**. A
portable application cannot express a timeout correctly.

The example works around it with a `SOCK_TIMEOUT_MS()` macro that scales the
value for lwIP only. Fixing the GNRC port to honour the documented unit would
be the correct repair, but it silently changes the timeout of every existing
GNRC `sock_tcp` user by a factor of a thousand, so it was left alone
deliberately.

## Verification

All on hardware, comparing the same firmware before and after the fixes.

| Test | Before | After |
| --- | --- | --- |
| Five silent sockets held open | refused for 210 s+, never recovered | recovers at t=39 s and stays healthy while the sockets are still held |
| Aborted-connection bursts, then a valid request | 20 of 40 answered with a wrong status | 40 of 40 correct |
| Mixed-abuse soak, full responses checked against `Content-Length` | not run | 22 correct, 0 wrong status, 0 truncated body |
| All routes | — | `/` 8089 B, `/riot-logo.svg` 6525 B, `/api/stats`, `/api/version`, `/api/uptime` 200, `/nope` 404 |

The soak also shows six momentary refusals. Those are back pressure, not a
failure: the load opens six parallel connections against a four deep queue, so
two are refused and a real client retries them.

Builds check out for GNRC and for `LWIP=1`, and `tests/net/gnrc_tcp` and
`tests/net/gnrc_sock_tcp` still compile.

### Not verified

- The `tests/net/gnrc_tcp` and `tests/net/gnrc_sock_tcp` suites were **not
  run**. They need root and tap devices on Linux, neither available on this
  machine. Run them before submitting, since the FSM change touches the shared
  stack.
- Only the IPv4 path was exercised on hardware. The changed code is not
  address family specific, but IPv6 was not tested.
- The client path is untouched by construction: every new branch is gated on
  `STATUS_ACCEPTED` or `STATUS_LISTENING`, and a client TCB sets neither.

### Behaviour change to be aware of

An application that accepted a sock and never called `sock_tcp_disconnect()`,
relying on the old behaviour of the TCB silently returning itself to the
listening pool, now leaks that TCB. That was always a misuse of the API, but
it used to be invisible.

## Diagnosing this again

Inspecting the TCBs on a wedged board is by far the fastest route to an
answer, and it does not disturb the wedge.

Start the GDB server, leaving the target running:

```sh
JLinkGDBServer -device EFM32GG11B820F2048GL192 -if SWD -speed 4000 -port 3333 -nogui
```

Then run a batch script against the application ELF. Connecting halts the
core; `monitor go` followed by `detach` resumes it, so the board keeps running
afterwards:

```
set pagination off
set confirm off
target extended-remote :3333
set $i = 0
while $i < 4
  set $t = &_sock_queue[$i]
  printf "tcb[%d] state=%u status=0x%02x lport=%u pport=%u rcv_wnd=%u rcv_buf(avail=%u) mbox=%p\n", \
    $i, $t->state, $t->status, $t->local_port, $t->peer_port, \
    $t->rcv_wnd, $t->rcv_buf.avail, $t->mbox
  set $i = $i + 1
end
monitor go
detach
```

Reading the output:

- `state` is the index into `_gnrc_tcp_fsm_state_t` in
  `include/gnrc_tcp_fsm.h`: 0 CLOSED, 1 LISTEN, 2 SYN_SENT, 3 SYN_RCVD,
  4 ESTABLISHED, 5 CLOSE_WAIT, 6 LAST_ACK, 7 FIN_WAIT_1, 8 FIN_WAIT_2,
  9 CLOSING, 10 TIME_WAIT.
- `status` is the `STATUS_*` bitmask from `include/gnrc_tcp_common.h`:
  `0x01` LISTENING, `0x02` ALLOW_ANY_ADDR, `0x04` NOTIFY_USER,
  `0x08` ACCEPTED, `0x10` LOCKED.
- A non-NULL `mbox` marks the TCB a user call is currently blocked on.
- **No TCB in state 1** is the "connection refused" picture. If the occupied
  TCBs also lack `0x08`, they are connections the stack completed that the
  application has not accepted, which is defect 1.
- `rcv_buf.avail` above zero on a TCB the application is not reading is
  leftover data, which is defect 3.

Two practical notes. GDB's `printf` rejects `%p` with a symbol argument such
as `_sock_queue`, so print scalars only. And `ps` on the RIOT shell is a
useful cross-check: `http_server` sitting in `bl mbox` with a switch count of
1 means it has been parked since boot and has never served anything.

### Reproducing the wedge

Deterministic, no browser needed. Open `SOCK_QUEUE_LEN` connections, send
nothing on them, hold them open, then try to fetch anything:

```python
import socket, time
HOST, PORT = '10.0.0.118', 8080
held = []
for _ in range(20):
    try:
        held.append(socket.create_connection((HOST, PORT), timeout=2))
    except Exception:
        pass
    if len(held) >= 6:
        break
    time.sleep(0.2)
# every request from here on is refused until the server reclaims the socks
```

### Reproducing the cross-connection corruption

Open several connections, send a partial or invalid request on each, reset
them all with `SO_LINGER` set to zero, then immediately send one valid
request. Before the fix the valid request was answered `400` when the aborted
connections had sent `GET / HTT`, and `404` when they had sent `ZZZZZZZZZ`.
That the status follows the aborted payload is the tell.

## Related

Earlier findings on the same branch, in
`~/.claude/projects/-Users-basilfx-Desktop-Projecten-RIOT-RIOT/memory/`:

- `gnrc-tcp-server-close-pitfalls.md`, on `sock_tcp_write()` being a partial
  write API, `gnrc_tcp_close()` blocking through TIME_WAIT, and the timeout
  unit mismatch.
- `gnrc-tcp-listening-tcb-recycling.md`, the condensed form of this document.
- `slstk3701a-board-test-setup.md`, on driving this board, and on why it
  periodically looks unreachable from this Mac although it is healthy.

---

# Reverse proxy failure: two further defects, neither in gnrc_tcp

Written 2026-09-11, debugging the report that the served page fails when it is
requested through an NGINX reverse proxy (`https://riot.apps.basilfx.net` ->
`proxy_pass http://10.0.0.118:8080`) while a direct request from this Mac
works. Reproduced on the same board and network as the notes above.

## Reported symptom

A request through the reverse proxy was never answered; NGINX gave up after
its 60 s `proxy_read_timeout` and returned `504 Gateway Time-out`. A direct
request from the Mac worked at the same moment. The RIOT shell stayed
responsive and a reboot restored service for a while.

## 4. A full ARP cache refused to resolve, so a new peer could not be answered

This is the reported failure. Both defects are in
`sys/net/gnrc/network_layer/ipv4/arp/gnrc_ipv4_arp.c`.

- `_alloc()` returned `NULL` once every one of the
  `CONFIG_GNRC_IPV4_ARP_CACHE_SIZE` (4) entries was taken, and
  `gnrc_ipv4_arp_request()` then dropped the packet. An entry is held for
  `CONFIG_GNRC_IPV4_ARP_CACHE_TIMEOUT_MS`, twenty minutes, so this host
  became unable to reach *any* address it had not already resolved, for that
  long. There was no eviction of any kind.

- `gnrc_ipv4_arp_handle_pkt()` allocated an entry for the sender of **every**
  ARP packet it saw, including the broadcast requests that every other host
  on the segment sends for peers this host never talks to. On a real
  Ethernet segment that fills all four entries within seconds.

Together the two turn a busy segment into a loss of connectivity. The TCB and
cache dump of the board in that state, taken over JLink, shows it exactly:

```
arp[0] state=2 ip=10.0.0.1   mac=0e:ea:14:1f:ca:ec   <- gateway
arp[1] state=2 ip=10.0.0.156 mac=00:50:b6:e2:84:40   <- this Mac
arp[2] state=2 ip=10.0.0.117 mac=ec:64:c9:c0:83:b4   <- unrelated host
arp[3] state=2 ip=10.0.0.168 mac=08:b6:1f:50:bf:80   <- unrelated host

tcb[3] state=3 status=0x13 lport=8080 pport=32792 peer=10.0.0.2
```

`state=3` is `FSM_STATE_SYN_RCVD`. The SYN of the proxy (10.0.0.2) arrived and
was accepted into a TCB, but the SYN+ACK could not be sent because 10.0.0.2
was not in the cache and no entry could be allocated for it. The TCB sat in
`SYN_RCVD` until the connection timeout returned it to `LISTEN`, and NGINX,
which had never completed its handshake, timed out. Nothing was wrong with
gnrc_tcp.

Why a direct request worked at the same time: 10.0.0.156 happened to hold one
of the four entries. Which peer is reachable is therefore decided by which
hosts ARPed first after boot, which is what made this look like a property of
the reverse proxy. It is not. Any peer that is not in the cache is
unreachable, and a reboot "fixes" it only until the cache fills again.

### Changes

- `_alloc()` reuses the least useful entry when the cache is full, via the new
  `_pick_victim()`. A `_STATE_REACHABLE` entry is preferred over a
  `_STATE_INCOMPLETE` one, which has a resolution in flight and a packet
  waiting on it, and among the candidates the one used longest ago is taken.
  The ordering comes from a new `last_used` field, stamped from a monotonic
  `_use_counter` in `_find()` and `_alloc()`.
- `gnrc_ipv4_arp_handle_pkt()` still refreshes an entry it already holds for
  any ARP packet, which is what keeps a gratuitous ARP working, but only
  *allocates* a new one when the packet's target protocol address is one of
  this host's. That covers the reply to a request of its own and a request
  from a peer that is about to talk to it, and it ignores the segment's
  unrelated broadcast traffic. This is what Linux does with `arp_accept=0`.

## 5. The example reset the RTT that ztimer owns, losing every armed timer

Found while testing the fix for defect 4, and worth its own entry because it
is a trap for any RIOT application, not just this one.

`examples/networking/misc/webserver/main.c` called `rtt_init()` in `main()` to
set up its `/api/uptime` reference. On this board `ZTIMER_MSEC` is a
`ztimer_convert_frac` over `ztimer_periph_rtt`, so ztimer already owns the
RTT and has initialised it during auto-init. On EFM32 `rtt_init()` performs
`RTCC_Reset()`, a full peripheral reset, which

- clears the compare channel, so the alarm ztimer had armed is gone, and
- zeroes the counter that `ztimer_init_extend()` extends.

Every ZTIMER_MSEC and ZTIMER_SEC timeout set before `main()` runs is therefore
lost, and the clock only comes back to life on the next `ztimer_set()`.

The DHCPv4 client arms exactly such a timer: `dhcpv4_client_start()` runs at
auto-init and schedules its `_init` event
`random_uint32_range(0, CONFIG_DHCPV4_CLIENT_INIT_DELAY_MS)` (0 to 2000 ms)
into the future, which is almost always still pending when `main()` reaches
`rtt_init()`. On a board in that state the client stays in `STATE_INIT` with
`xid == 0`, no DISCOVER is ever sent, `netstats_l2` shows `tx_bytes == 0`
while `rx_count` climbs, and the board never gets an address. The shell works
throughout, so it looks like a network problem.

**Before the ARP fix this was invisible**, which is why it had never been
noticed: the old `gnrc_ipv4_arp_handle_pkt()` allocated an entry for the first
ARP broadcast to arrive, within milliseconds of boot, and the
`_sched_timeout()` that followed called `ztimer_set()` and re-armed the RTT
alarm, after which the pending DHCP timer fired. Fixing the ARP cache removed
that accidental kick and turned a latent defect into a hard one, which is a
useful reminder that this class of bug hides behind unrelated timer traffic.

### Change

`main()` only calls `rtt_init()` / `rtc_init()` when ztimer is not the owner
of that peripheral, guarded on `IS_USED(MODULE_ZTIMER_PERIPH_RTT)` and
`IS_USED(MODULE_ZTIMER_PERIPH_RTC)`. Reading the counter is harmless and stays
as it was.

Worth considering instead: drop the RTT and RTC handling from the example
altogether and take the uptime from `ztimer_now(ZTIMER_SEC)`, which is
portable, needs no board feature gating, and cannot conflict with ztimer over
a peripheral. That is a design change to the example rather than a repair, so
it was left out here.

## Verification

All on hardware, IPv4 over DHCP on the same segment as the NGINX host.

| Test | Before | After |
| --- | --- | --- |
| `GET /` through the reverse proxy | 504 after 60 s, three attempts in a row | 200, 7944 B, 30 to 48 ms, three attempts in a row |
| Every route through the reverse proxy | not reachable | `/` 7944 B, `/riot-logo.svg` 6525 B, `/api/stats` 891 B, `/api/version` 56 B, `/api/uptime` 17 B, `/nope` 404 |
| ARP cache contents after minutes of real traffic | four entries, two of them hosts the board never talks to, 10.0.0.2 absent | three entries, all peers that addressed the board, 10.0.0.2 present, one entry still free |
| Eviction path, `CONFIG_GNRC_IPV4_ARP_CACHE_SIZE=2` with three competing peers | queue wedges immediately | 24 of 24 requests answered 200 while every allocation evicts |
| DHCP lease after flashing | 5 s (ARP fix absent) / never (ARP fix alone) | 4 to 5 s |
| Soak, 240 s, every route alternating proxy and direct, one connection at a time | not run | 866 rounds, 8660 responses, all 200 or 404, 0 failures, 0 reboots, uptime monotonic to 258 s |
| Builds | — | GNRC and `LWIP=1` both build for slstk3701a |

### Not verified

- IPv6 was not exercised. `gnrc_ipv4_arp.c` is IPv4 only, and the `main.c`
  guard is address family agnostic.
- The RTT guard was only exercised on a board where ztimer owns the RTT. The
  `MODULE_PERIPH_RTC` branch was not run.
- `tests/net/gnrc_tcp` and `tests/net/gnrc_sock_tcp` still were not run, see
  the note above.

### Diagnosing this again

The ARP cache is a static array, so it dumps the same way as the TCBs:

```
set $i = 0
while $i < 4
  set $e = &'gnrc_ipv4_arp.c'::_cache[$i]
  printf "arp[%d] state=%u ip=%d.%d.%d.%d used=%u pending=%d\n", $i, $e->state, \
    $e->ipv4.u8[0], $e->ipv4.u8[1], $e->ipv4.u8[2], $e->ipv4.u8[3], \
    $e->last_used, ($e->pending != 0)
  set $i = $i + 1
end
```

`state` is `0` free, `1` incomplete, `2` reachable. A cache with no free entry
and no entry for the peer that is failing is defect 4.

`$n = gnrc_netif_iter(0)` then `print $n->stats` gives the `netstats_l2`
counters, and they separate the two defects at a glance: `tx_bytes == 0` with
`rx_count` climbing means nothing was ever transmitted, which is defect 5, not
a routing or ARP problem.

Two practical notes on the tooling. The serial console is often already held
by a `pyterm`, and only one reader gets the bytes; building with
`USEMODULE=stdio_rtt` and reading the output over JLink RTT
(`JLinkExe -AutoConnect 1 -RTTTelnetPort 19021`, then `nc localhost 19021`)
avoids fighting over it. And `stdio_rtt` blocks once its buffer fills with no
host attached, so turning `ENABLE_DEBUG` on in a module that logs per packet
stops the board at boot; attach the reader before resetting, or keep the debug
output to one module.
