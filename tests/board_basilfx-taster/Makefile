BOARD ?= basilfx-taster
include ../Makefile.tests_common

# This test application is for the BasilFX Taster only
BOARD_WHITELIST := basilfx-taster

# Required features
FEATURES_REQUIRED += periph_gpio_irq
FEATURES_REQUIRED += periph_rtc
FEATURES_REQUIRED += periph_spi

# Modules and packages
USEMODULE += ncn5120
USEMODULE += shell

include $(RIOTBASE)/Makefile.include
