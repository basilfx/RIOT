# name of your application
APPLICATION = webserver

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# default to using GNRC
LWIP ?= 0

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../../..

USEMODULE += sock_tcp
USEMODULE += netdev_default

USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps
USEMODULE += netutils
USEMODULE += schedstatistics

# ztimer defaults to backing ztimer_msec/ztimer_sec directly off periph_rtt
# when a board provides it, and forces RTT_FREQUENCY to the highest value the
# CPU claims to support (32768 Hz here) to get the best possible conversion
# precision for that backend. On this board that backfires: the RTCC counter
# does not actually hold 32 usable bits at that rate, it wraps roughly every
# 512 s, corrupting ztimer's own alarm bookkeeping. Falling back to a real
# hardware timer for ztimer sidesteps that entirely, restoring the board's
# own intended RTT_FREQUENCY of 1 Hz (well within any usable counter width),
# and gives the network stack's timers back proper hardware-timer precision
# instead of whatever the RTT backend could offer
USEMODULE += ztimer_no_periph_rtt

# main.c gets /api/uptime through sys/walltime rather than reading periph_rtt
# or periph_rtc directly, so it works the same way regardless of which timer
# backend a given board provides
USEMODULE += walltime_default

USEPKG += picohttpparser
USEPKG += mjson

# the RPC engine, base64 and pretty-printing support are all unused here,
# this app only ever emits flat JSON objects through mjson's print
# primitives
CFLAGS += -DMJSON_ENABLE_RPC=0
CFLAGS += -DMJSON_ENABLE_BASE64=0
CFLAGS += -DMJSON_ENABLE_PRETTY=0

# thread names are shown on the stats page served by this example
THREAD_NAMES ?= 1

# embed the served web assets as generated headers, see makefiles/blob.inc.mk
WWW_SRC := index.html assets/riot-logo.svg assets/github-logo.svg assets/theme-icon.svg

BLOBS += $(WWW_SRC)

# default to IPv4; set IPV6=1 to build an IPv6 node instead, for either stack
IPV6 ?= 0

ifeq (1, $(LWIP))
  USEMODULE += lwip_netdev

  ifeq (1, $(IPV6))
    USEMODULE += ipv6_addr
    USEMODULE += lwip_ipv6_autoconfig
  else
    USEMODULE += ipv4_addr
    USEMODULE += lwip_arp
    USEMODULE += lwip_ipv4
    USEMODULE += lwip_dhcp_auto
  endif
else
  USEMODULE += auto_init_gnrc_netif
  USEMODULE += netstats_l2
  # every sock in the listening queue holds a receive buffer of its own, plus
  # a few more for the ones that are momentarily being read from or written
  # to. Sized for SOCK_QUEUE_LEN in main.c, see there for the reasoning
  CFLAGS += -DCONFIG_GNRC_TCP_RCV_BUFFERS=9
  # a single eventloop thread dispatches every packet and timer event for
  # every TCB, listening or active, through one fixed-size mailbox. Its
  # default of 8 slots was sized for a 4 deep accept queue; with 8 TCBs each
  # carrying their own timers it overflows, and gnrc_tcp silently drops
  # whatever does not fit ("gnrc_netapi: dropped message ... receiver queue
  # is full"), which was observed to turn an expected ~20 s queue-reclaim
  # into upwards of 90 s. Sized with headroom above SOCK_QUEUE_LEN, since a
  # burst of events across every held socket well overshoots the socket
  # count itself
  CFLAGS += -DCONFIG_GNRC_TCP_EVENTLOOP_MSG_QUEUE_SIZE_EXP=5
  # the static packet buffer is shared by every packet under construction or
  # awaiting processing across the whole stack: outgoing TCP segments,
  # address resolution, ICMP, and IP reassembly. The default of 6144 B was
  # sized for a 576 B MSS; the raised MSS below and a deeper accept queue
  # both need more packets in flight at once, so this grows along with them
  CFLAGS += -DCONFIG_GNRC_PKTBUF_SIZE=16384
  # more DMA descriptors absorb a burst of frames, for instance the SYNs of a
  # browser's parallel connections arriving together, without the driver
  # dropping any before the stack gets round to them
  CFLAGS += -DCONFIG_EFM32_ETH_RX_BUF_NUMOF=8
  CFLAGS += -DCONFIG_EFM32_ETH_TX_BUF_NUMOF=8
  # whenever the server is the side that closes a connection, its end point
  # enters TIME_WAIT and gnrc_tcp_close() blocks until that state is left.
  # With the RFC mandated maximum segment lifetime that keeps the only
  # listening sock busy for 60 seconds, so shorten it
  CFLAGS += -DCONFIG_GNRC_TCP_EXPERIMENTAL_DYN_MSL_EN=1
  # every blocking gnrc_tcp call gives up after this long. The default of two
  # minutes means that a peer which stops responding halfway through, for
  # instance one that vanished without closing its connection, keeps the only
  # server thread busy for that long. This bound applies per socket, and the
  # accept queue is reclaimed strictly serially by that one thread, so it is
  # also, worst case, what a client holding every slot of the queue idle
  # costs to recover from, multiplied by SOCK_QUEUE_LEN: measured at 75 s for
  # 6 held connections with the previous value of 10000. Two retransmissions
  # at CONFIG_GNRC_TCP_RTO_LOWER_BOUND_MS (1 s, 2 s) still fit well inside
  # this on the board's own Ethernet segment, which is the only kind of
  # delay a well behaved peer on this LAN should ever need it for
  CFLAGS += -DCONFIG_GNRC_TCP_CONNECTION_TIMEOUT_DURATION_MS=4000

  ifeq (1, $(IPV6))
    USEMODULE += gnrc_ipv6_default
    USEMODULE += gnrc_icmpv6_echo
    USEMODULE += gnrc_icmpv6_error
    # the IPv6 thread has the same fixed-size mailbox as the TCP eventloop
    # above, shared by every packet and timer event across every TCB, and is
    # just as prone to overflowing under an accept-queue burst, see
    # CONFIG_GNRC_TCP_EVENTLOOP_MSG_QUEUE_SIZE_EXP above for the mechanism
    CFLAGS += -DCONFIG_GNRC_IPV6_MSG_QUEUE_SIZE_EXP=5
    # the board's 1500 B link MTU minus a 40 B IPv6 header and a 20 B TCP
    # header, see CONFIG_GNRC_TCP_MSS below for why this is worth raising
    # from its safe-minimum default at all
    CFLAGS += -DCONFIG_GNRC_TCP_MSS=1440
  else
    USEMODULE += gnrc_ipv4
    USEMODULE += gnrc_dhcpv4_client
    USEMODULE += gnrc_ipv4_frag
    USEMODULE += gnrc_icmpv4_echo
    USEMODULE += gnrc_icmpv4_error
    # the single gnrc_ipv4 thread has the same shape of problem as the TCP
    # eventloop above: every packet for every TCB passes through its
    # mailbox, and its default of 8 slots was observed to overflow the same
    # way ("dropped message to 2", 2 being its pid) during a queue-reclaim
    # burst, well before that fix was enough on its own
    CFLAGS += -DCONFIG_GNRC_IPV4_MSG_QUEUE_SIZE_EXP=5
    # the IPv4 default is 576 B, the safe minimum, which turns every response
    # bigger than that, which is every route except /api/version and
    # /api/uptime, into a chain of small segments each waiting on the next
    # ACK. The board is on a switched Ethernet segment with a 1500 B MTU
    # throughout, so segmenting at the link MTU instead roughly halves the
    # segment count of a full page load without risking fragmentation
    CFLAGS += -DCONFIG_GNRC_TCP_MSS=1460
    # four entries is tight on a real Ethernet segment with more hosts on it
    # than that; see GNRC_TCP_FINDINGS.md for what running out of entries
    # does. More entries mean fewer evictions, and so fewer of the round
    # trips an eviction costs to re-resolve a peer
    CFLAGS += -DCONFIG_GNRC_IPV4_ARP_CACHE_SIZE=8
  endif
endif

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# As there is a .config we want to explicitly disable Kconfig by setting
# the variable to empty
SHOULD_RUN_KCONFIG ?=

include $(RIOTBASE)/Makefile.include
