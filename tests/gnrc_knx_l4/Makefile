include ../Makefile.tests_common

USEMODULE += knx
USEMODULE += gnrc_knx_l4
USEMODULE += embunit
USEMODULE += netdev_test

CFLAGS += -DTEST_SUITES

include $(RIOTBASE)/Makefile.include

test:
	tests/01-run.py
