MODULE=esp_idf_nvs_flash

include $(RIOTBASE)/Makefile.base

CFLAGS += -DESP_PLATFORM -DNVS_CRC_HEADER_FILE=\"crc.h\"
INCLUDES += -I$(ESP8266_RTOS_SDK_DIR)/components/nvs_flash/include
INCLUDES += -I$(ESP8266_RTOS_SDK_DIR)/components/util/include

# vendor code triggers warnings with newer compiler versions
CXXFLAGS += -Wno-type-limits
CXXFLAGS += -Wno-stringop-truncation
CXXFLAGS += -Wno-format
