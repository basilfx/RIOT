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
# Specify the mandatory networking modules for IPv4 and IGMPv2
USEMODULE += gnrc_ipv4
USEMODULE += gnrc_igmp
# shell_cmd_gnrc_udp provides the "udp" shell command used to start a UDP
# server, and transitively pulls in gnrc_udp itself (see
# sys/shell/Makefile.dep). gnrc_pktdump is used to prove that a multicast
# datagram actually reaches the transport layer: it is auto-init'd and
# prints a distinctive line for every received packet, which
# tests-as-root/01-run.py's test_group_reception() looks for in the
# device's own shell output, instead of relying on an ICMPv4 error message
# (which gnrc_icmpv4_error_dst_unr_send() never generates for a multicast
# destination in the first place, see
# gnrc_icmpv4_error.c's _check_ipv4_hdr()).
USEMODULE += shell_cmd_gnrc_udp
USEMODULE += gnrc_pktdump
# shell_cmd_gnrc_udp's "udp" shell command implementation
# (sys/shell/cmds/gnrc_udp.c) also contains an unused "send" subcommand
# whose _send() unconditionally references gnrc_ipv6_hdr_build(), in the
# same translation unit as the "server" subcommand this test actually
# uses. gnrc_ipv6_hdr is the lightweight header-building module that
# symbol needs (no NDP/routing pulled in), required here purely to link,
# never exercised at runtime by this IPv4-only test.
USEMODULE += gnrc_ipv6_hdr
# sys/shell/cmds/gnrc_netif.c's ifconfig "add"/"del" handling shares its
# _get_prefix_len() helper between the IPv4 and IPv6 code paths, and that
# helper unconditionally calls ipv6_addr_split_int() whenever
# MODULE_GNRC_NETIF_IPV4 is defined -- even though this app never uses
# IPv6 itself. tests/net/gnrc_ipv4 only avoids needing this explicitly
# because it also happens to pull in gnrc_icmpv4_echo, whose netutils
# dependency drags in ipv6_addr as an unrelated side effect. Since this
# app has no need for gnrc_icmpv4_echo, pull in ipv6_addr directly instead.
USEMODULE += ipv6_addr
USEMODULE += shell_cmd_gnrc_pktbuf
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

# Shorten the unsolicited report interval so a group join's randomized
# repeated report retransmission (RFC 2236, section 7.10) settles well
# before the test suite's later scenarios start asserting that no further,
# spontaneous report arrives for a group. See ../tests-as-root/01-run.py
# for how this value is used to deterministically drain that retransmission
# between scenarios.
CFLAGS += -DCONFIG_GNRC_IGMP_UNSOLICITED_REPORT_INTERVAL_MS=500

# The default budget of 1 (sys/include/net/gnrc/netif/conf.h) is reserved
# for the permanent all-hosts group (224.0.0.1, see gnrc_netif.c's
# _init_from_device()) and leaves no room for any group this test suite
# joins itself. Scenario 3
# (test_group_specific_query_only_answers_that_group) needs 3 slots
# simultaneously (all-hosts + GROUP_A + GROUP_B); 4 gives comfortable
# margin over that.
CFLAGS += -DCONFIG_GNRC_NETIF_IPV4_GROUPS_NUMOF=4

# The test requires some setup and to be run as root
# So it cannot currently be run
TEST_ON_CI_BLACKLIST += all

.PHONY: ethos

ethos:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)/ethos

include $(RIOTBASE)/Makefile.include
