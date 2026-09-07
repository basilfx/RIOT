# name of your application
APPLICATION = gnrc_ipv4

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../../..

# Include packages that pull up and auto-init the link layer.
USEMODULE += netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for an IPv4 node
USEMODULE += gnrc_ipv4
USEMODULE += gnrc_ipv4_frag
# Activate ICMPv4 echo and error messages
USEMODULE += gnrc_icmpv4_echo
USEMODULE += gnrc_icmpv4_error
# Add multicast group membership reporting
USEMODULE += gnrc_igmp
# Additional networking modules that can be dropped if not needed
USEMODULE += gnrc_udp
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_cmds_default
USEMODULE += shell_cmd_udp_server
USEMODULE += ps
USEMODULE += netstats_l2

# Address configuration: pick exactly one of the following, or leave both
# commented out and configure the address by hand with `ifconfig` from the
# shell once the node is running.

# Option 1: a static IPv4 address, prefix length and default router.
# USEMODULE += gnrc_ipv4_static_addr
# CFLAGS += -DCONFIG_GNRC_IPV4_STATIC_ADDR=\"192.168.0.100/24\"
# CFLAGS += -DCONFIG_GNRC_IPV4_STATIC_DEFAULT_ROUTER=\"192.168.0.1\"

# Option 2: obtain an address, default router and subnet mask via DHCPv4.
# USEMODULE += gnrc_dhcpv4_client

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
