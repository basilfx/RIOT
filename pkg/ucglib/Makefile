PKG_NAME=ucglib
PKG_URL=https://github.com/olikraus/ucglib
PKG_VERSION=230f15e3bcd3c84977780e84bd855ac89c1959ee
PKG_LICENSE=BSD-2-Clause

include $(RIOTBASE)/pkg/pkg.mk

.PHONY: ucglib_sdl

all: $(filter ucglib_sdl,$(USEMODULE))
	"$(MAKE)" -C $(PKG_BUILDDIR) -f $(CURDIR)/Makefile.ucglib
	"$(MAKE)" -C $(PKG_BUILDDIR)/csrc -f $(CURDIR)/Makefile.ucglib_csrc

ucglib_sdl:
	"$(MAKE)" -C $(PKG_BUILDDIR)/sys/sdl/dev -f $(CURDIR)/Makefile.ucglib_sdl
