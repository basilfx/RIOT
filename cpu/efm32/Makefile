MODULE = cpu

# add a list of subdirectories, that should also be build
DIRS = periph families/$(EFM32_FAMILY) $(RIOTCPU)/cortexm_common

# add EFM32 specific drivers, if enabled
ifneq (,$(filter efm32_coretemp,$(USEMODULE)))
  DIRS += drivers/coretemp
endif

include $(RIOTBASE)/Makefile.base
