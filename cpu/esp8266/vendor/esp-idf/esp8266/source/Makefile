MODULE=esp_idf_esp8266

include $(RIOTBASE)/Makefile.base

INCLUDES += -I$(ESP8266_RTOS_SDK_DIR)/components/esp8266/source
INCLUDES += -I$(ESP8266_RTOS_SDK_DIR)/components/util/include

# vendor code triggers warnings with newer compiler versions
CFLAGS += -Wno-format
CFLAGS += -Wno-calloc-transposed-args

# rtc_sys_info is defined by the linker script, which hides its real size
CFLAGS += -Wno-array-bounds
CFLAGS += -Wno-stringop-overflow
