APPLICATION = gnrc_knx_device

# If no BOARD is found in the environment, use this default:
BOARD ?= slstk3401a

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Modules and packages:
USEMODULE += ncn5120
# USEMODULE += tpuart

USEMODULE += knx
USEMODULE += knx_device

USEMODULE += gnrc
USEMODULE += gnrc_knx_l3
USEMODULE += gnrc_knx_l4
USEMODULE += gnrc_knx_l7
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif

USEMODULE += shell
USEMODULE += shell_commands

FEATURES_REQUIRED += periph_gpio_irq

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

include $(RIOTBASE)/Makefile.include
