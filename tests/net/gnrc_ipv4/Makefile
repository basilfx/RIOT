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
# Specify the mandatory networking modules for IPv4
USEMODULE += gnrc_ipv4
USEMODULE += gnrc_ipv4_frag
USEMODULE += gnrc_icmpv4_echo
USEMODULE += gnrc_icmpv4_error
USEMODULE += shell_cmd_gnrc_icmpv4_echo
USEMODULE += gnrc_udp
USEMODULE += shell_cmd_gnrc_pktbuf
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

# Shorten the reassembly buffer timeout so the timed-out-fragment-set test
# doesn't have to wait out the (production) 60 second default
CFLAGS += -DCONFIG_GNRC_IPV4_FRAG_RBUF_TIMEOUT_MS=5000

# The test requires some setup and to be run as root
# So it cannot currently be run
TEST_ON_CI_BLACKLIST += all

.PHONY: ethos

ethos:
	$(Q)env -u CC -u CFLAGS $(MAKE) -C $(RIOTTOOLS)/ethos

include $(RIOTBASE)/Makefile.include
