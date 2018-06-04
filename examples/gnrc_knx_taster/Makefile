APPLICATION = gnrc_knx_taster

# If no BOARD is found in the environment, use this default:
BOARD ?= basilfx-taster

# Current version (for firmware updating):
APP_VER ?= 1

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Define additional application-specific modules:
EXTERNAL_MODULE_DIRS += $(CURDIR)/common

FEATURES_REQUIRED += periph_cpuid
FEATURES_REQUIRED += periph_i2c
FEATURES_REQUIRED += periph_rtc
FEATURES_REQUIRED += periph_gpio_irq

FEATURES_OPTIONAL += efm32_coretemp
FEATURES_OPTIONAL += riotboot

# Modules and packages:
USEMODULE += ncn5120
# USEMODULE += tpuart

USEMODULE += efm32_coretemp

USEMODULE += knx
USEMODULE += knx_device

USEMODULE += gnrc
USEMODULE += gnrc_knx_l3
USEMODULE += gnrc_knx_l4
USEMODULE += gnrc_knx_l7
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += netstats_l2
USEMODULE += l2filter
USEMODULE += hashes
USEMODULE += random

USEMODULE += mpu_stack_guard

USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

USEMODULE += vfs
USEMODULE += littlefs2

USEMODULE += riotboot_flashwrite
USEMODULE += riotboot_slot

USEMODULE += common_buttons
USEMODULE += common_sensors

USEPKG += u8g2

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

CFLAGS += -DLOG_LEVEL=LOG_DEBUG -DDEBUG_ASSERT_VERBOSE

CFLAGS += -DAPP_VER=$(APP_VER)

include $(RIOTBASE)/Makefile.include