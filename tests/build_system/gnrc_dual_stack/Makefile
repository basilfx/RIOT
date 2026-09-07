include ../Makefile.build_system_common

# No netdev/netif module on purpose: this is a compile and boot smoke test,
# not a networking test, and pulling in a real link layer would make the
# automated `test` target require a tap interface (and thus root) on
# native. gnrc_ipv4_init()/gnrc_ipv6_init() run from auto_init regardless of
# any actual interface, so both threads still start without one.

# IPv6 side
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_icmpv6_echo

# IPv4 side
USEMODULE += gnrc_ipv4
USEMODULE += gnrc_ipv4_frag
USEMODULE += gnrc_icmpv4_echo
USEMODULE += gnrc_icmpv4_error
USEMODULE += gnrc_igmp

# Upper layers, family-aware on both sides
USEMODULE += gnrc_udp
USEMODULE += gnrc_tcp
USEMODULE += gnrc_sock_udp
USEMODULE += gnrc_sock_ip
USEMODULE += gnrc_sock_tcp

USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += ps

include $(RIOTBASE)/Makefile.include
