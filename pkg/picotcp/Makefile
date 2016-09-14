PKG_NAME=picotcp
PKG_URL=https://github.com/toonst/picotcp.git
PKG_VERSION=e196e5be7aea410b8d65d5e30635c2ecf7413595

.PHONY: all

all: git-download
	"$(MAKE)" all -C $(PKG_BUILDDIR) CROSS_COMPILE=$(PREFIX) PREFIX=$(PKG_BUILDDIR)/build PLATFORM_CFLAGS="$(CFLAGS_CPU) -m32 $(INCLUDES)" ARCH=riotos

include $(RIOTBASE)/pkg/pkg.mk
