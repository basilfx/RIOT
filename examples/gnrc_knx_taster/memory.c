/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "memory.h"

knx_table_device_t device_table = {
    .programming_mode = 0x00,
    .manufacturer_id = { .u16 = 0x3A01 },
    .serial = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    .device_control = 0x00,
};

knx_table_addr_t addr_table;
knx_table_assoc_t assoc_table;
knx_table_com_objects_t com_object_table;

knx_table_load_state_t load_state_table = {
    .addr_table_load_state = 1,
    .assoc_table_load_state = 1,
    .com_object_table_load_state = 1,
    .pei_prog_load_state = 1
};

memory_settings_t settings;
uint8_t ota[2048];

knx_memory_segment_t segments[] = {
    [MEMORY_SEGMENT_DEVICE_TABLE] = {
        .start = 0x0060,
        .size = sizeof(device_table),
        .type = KNX_MEMORY_TYPE_RAM,
        .flags = KNX_MEMORY_FLAG_READABLE | KNX_MEMORY_FLAG_WRITABLE,
        .ptr = &device_table
    },
    [MEMORY_SEGMENT_ADDR_TABLE] = {
        .start = 0x4000,
        .size = sizeof(addr_table),
        .type = KNX_MEMORY_TYPE_EEPROM,
        .flags = KNX_MEMORY_FLAG_READABLE | KNX_MEMORY_FLAG_WRITABLE,
        .ptr = &addr_table
    },
    [MEMORY_SEGMENT_ASSOC_TABLE] = {
        .start = 0x5000,
        .size = sizeof(assoc_table),
        .type = KNX_MEMORY_TYPE_EEPROM,
        .flags = KNX_MEMORY_FLAG_READABLE | KNX_MEMORY_FLAG_WRITABLE,
        .ptr = &assoc_table
    },
    [MEMORY_SEGMENT_COM_OBJECT_TABLE] = {
        .start = 0x6000,
        .size = sizeof(com_object_table),
        .type = KNX_MEMORY_TYPE_EEPROM,
        .flags = KNX_MEMORY_FLAG_READABLE | KNX_MEMORY_FLAG_WRITABLE,
        .ptr = &com_object_table
    },
    [MEMORY_SEGMENT_SETTINGS] = {
        .start = 0x7000,
        .size = sizeof(settings),
        .type = KNX_MEMORY_TYPE_EEPROM,
        .flags = KNX_MEMORY_FLAG_READABLE | KNX_MEMORY_FLAG_WRITABLE,
        .ptr = &settings
    },
    [MEMORY_SEGMENT_OTA] = {
        .start = 0xa000,
        .size = sizeof(ota),
        .type = KNX_MEMORY_TYPE_RAM,
        .flags = KNX_MEMORY_FLAG_READABLE | KNX_MEMORY_FLAG_WRITABLE,
        .ptr = &ota
    },
    [MEMORY_SEGMENT_LOAD_STATE_TABLE] = {
        .start = 0xb6ea,
        .size = sizeof(load_state_table),
        .type = KNX_MEMORY_TYPE_RAM,
        .flags = KNX_MEMORY_FLAG_READABLE | KNX_MEMORY_FLAG_WRITABLE,
        .ptr = &load_state_table
    },
    KNX_MEMORY_SEGMENT_END
};
