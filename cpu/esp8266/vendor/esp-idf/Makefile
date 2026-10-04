MODULE=esp_idf

DIRS += esp8266/source
DIRS += nvs_flash/src
DIRS += spi_flash
DIRS += util/src
DIRS += wpa_supplicant

ifneq (, $(filter esp_idf_heap, $(USEMODULE)))
    DIRS += heap/src
endif

include $(RIOTBASE)/Makefile.base

# vendor code uses format specifiers that do not match the types of newer
# compiler versions, where uint32_t is unsigned long
CFLAGS += -Wno-format
