PKG_NAME=picotcp
PKG_URL=https://github.com/toonst/picotcp.git
PKG_VERSION=e196e5be7aea410b8d65d5e30635c2ecf7413595

.PHONY: all

# undefine variables that might already be defined by RIOT
undefine TCP
undefine UDP
undefine ETH
undefine IPV4
undefine IPV4FRAG
undefine IPV6FRAG
undefine NAT
undefine ICMP4
undefine MCAST
undefine DEVLOOP
undefine PING
undefine DHCP_CLIENT
undefine DHCP_SERVER
undefine DNS_CLIENT
undefine MDNS
undefine DNS_SD
undefine SNTP_CLIENT
undefine IPFILTER
undefine CRC
undefine OLSR
undefine SLAACV4
undefine TFTP
undefine AODV
undefine MEMORY_MANAGER
undefine MEMORY_MANAGER_PROFILING
undefine TUN
undefine TAP
undefine PCAP
undefine PPP
undefine IPC
undefine CYASSL
undefine WOLFSSL
undefine POLARSSL
undefine IPV6

#ifneq (,$(filter picotcp_core,$(USEMODULE)))
#    PICO_FLAGS += core
#endif
#
#ifneq (,$(filter picotcp_udp,$(USEMODULE)))
#    PICO_FLAGS += mod
#    PICO_FLAGS += UDP=1
#endif

# PREFIX=$(PKG_BUILDDIR)
unexport PREFIX

all: git-download
	@echo "builddir: $(PKG_BUILDDIR)"
	@echo "prefix: $(PREFIX)"
	@echo "pw: $(PWD)"
	"$(MAKE)" all -C $(PKG_BUILDDIR) CROSS_COMPILE=$(PREFIX)
	@echo "pwd: $(PWD)"

include $(RIOTBASE)/pkg/pkg.mk
