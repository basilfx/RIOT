DEVELHELP := 1
include ../Makefile.net_common

export TAP ?= tap0

# use Ethernet as link-layer protocol
ifneq (,$(filter native native32 native64,$(BOARD)))
  PORT ?= $(TAP)
else
  ETHOS_BAUDRATE ?= 115200
  CFLAGS += -DETHOS_BAUDRATE=$(ETHOS_BAUDRATE)
  TERMDEPS += ethos
  TERMPROG ?= sudo $(RIOTTOOLS)/ethos/ethos
  TERMFLAGS ?= $(TAP) $(PORT) $(ETHOS_BAUDRATE)
endif
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv4 and the DHCPv4 client
USEMODULE += gnrc_ipv4
USEMODULE += gnrc_dhcpv4_client
# sys/shell/cmds/gnrc_netif.c's ifconfig "add"/"del" handling and
# gnrc_sock_udp.c unconditionally reference IPv6 address helpers whenever
# MODULE_GNRC_NETIF_IPV4 is defined, even in an IPv4-only build (see
# tests/net/gnrc_igmp/Makefile for the same workaround). Pull in ipv6_addr
# directly to satisfy that link-time dependency without pulling in gnrc_ipv6.
USEMODULE += ipv6_addr
USEMODULE += shell_cmd_gnrc_pktbuf
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

# Shorten the random pre-DISCOVER/restart delay so tests-as-root/01-run.py
# doesn't have to wait out the (production) default on every restart
CFLAGS += -DCONFIG_DHCPV4_CLIENT_INIT_DELAY_MS=300

# The test requires some setup and to be run as root
# So it cannot currently be run
TEST_ON_CI_BLACKLIST += all

.PHONY: ethos

ethos:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)/ethos

include $(RIOTBASE)/Makefile.include
