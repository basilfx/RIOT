/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stddef.h>

#include "byteorder.h"
#include "properties.h"
#include "memory.h"

/* this file must be user supplied/generated due to licensing */
#include "property_types.h"

/* this contains additional (non-compliant) properties */
#include "property_types_user.h"

#define VALUE_U8(x) { (uint8_t)(x & 0xFF) }
#define VALUE_U16(x) { (uint8_t)((x >> 8) & 0xFF), (uint8_t)(x & 0xFF) }
#define VALUE_U32(x) { (uint8_t)((x >> 24) & 0xFF), (uint8_t)((x >> 16) & 0xFF), (uint8_t)((x >> 8) & 0xFF), (uint8_t)(x & 0xFF) }

#define DEVICE_TABLE(field) ((void *)(((uint32_t)&device_table) + offsetof(knx_table_device_t, field)))
#define LOAD_STATE_TABLE(field) ((void *)(((uint32_t)&load_state_table) + offsetof(knx_table_load_state_t, field)))

/**
 * @brief   General-purpose dummy to use for properties that are not
 *          implemented, but need some place to store data
 */
static uint8_t dummy[16];

static knx_property_t properties_device[] = {
    {
        .id = PID_G_OBJECT_TYPE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(OT_DEVICE)
    },
    {
        .id = PID_G_SERIAL_NUMBER,
        .type = KNX_PROPERTY_TYPE_GENERIC6,
        .flags = KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = DEVICE_TABLE(serial)
    },
    {
        .id = PID_G_MANUFACTURER_ID,
        .type = KNX_PROPERTY_TYPE_GENERIC2,
        .flags = KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = DEVICE_TABLE(manufacturer_id)
    },
    {
        .id = PID_G_FIRMWARE_REVISION,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_CHAR,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(APP_VER)
    },
    {
        .id = PID_0_MAX_APDULENGTH,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(0x0f)
    },
    {
        .id = PID_0_PROGMODE,
        .type = KNX_PROPERTY_TYPE_BITSET8,
        .flags = KNX_PROPERTY_FLAG_WRITABLE | KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = DEVICE_TABLE(programming_mode)
    },
    {
        .id = PID_0_HARDWARE_TYPE,
        .type = KNX_PROPERTY_TYPE_GENERIC6,
        .flags = KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = DEVICE_TABLE(hardware_type)
    },
    {
        .id = PID_G_ORDER_INFO,
        .type = KNX_PROPERTY_TYPE_GENERIC10,
        .flags = KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = DEVICE_TABLE(order_info)
    },
    {
        .id = PID_G_DEVICE_CONTROL,
        .type = KNX_PROPERTY_TYPE_GENERIC1,
        .flags = KNX_PROPERTY_FLAG_POINTER | KNX_PROPERTY_FLAG_WRITABLE,
        .content.ptr = DEVICE_TABLE(device_control)
    }
};

static knx_property_t properties_1[] = {
    {
        .id = PID_G_OBJECT_TYPE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(OT_ADDRESS_TABLE)
    },
    {
        .id = PID_G_LOAD_STATE_CONTROL,
        .type = KNX_PROPERTY_TYPE_CONTROL,
        .flags = KNX_PROPERTY_FLAG_WRITABLE | KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = LOAD_STATE_TABLE(addr_table_load_state)
    },
    {
        .id = PID_G_TABLE_REFERENCE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(0x4000)
    }
};

static knx_property_t properties_2[] = {
    {
        .id = PID_G_OBJECT_TYPE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(OT_ASSOCIATION_TABLE)
    },
    {
        .id = PID_G_LOAD_STATE_CONTROL,
        .type = KNX_PROPERTY_TYPE_CONTROL,
        .flags = KNX_PROPERTY_FLAG_WRITABLE | KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = LOAD_STATE_TABLE(assoc_table_load_state)
    },
    {
        .id = PID_G_TABLE_REFERENCE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(0x5000)
    }
};

static knx_property_t properties_3[] = {
    {
        .id = PID_G_OBJECT_TYPE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(OT_APPLICATION_PROGRAM)
    },
    {
        .id = PID_G_LOAD_STATE_CONTROL,
        .type = KNX_PROPERTY_TYPE_CONTROL,
        .flags = KNX_PROPERTY_FLAG_WRITABLE | KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = LOAD_STATE_TABLE(com_object_table_load_state)
    },
    {
        .id = PID_G_RUN_STATE_CONTROL,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_CHAR,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U8(0x01)
    },
    {
        .id = PID_G_PROGRAM_VERSION,
        .type = KNX_PROPERTY_TYPE_GENERIC5,
        .flags = KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = &dummy
    },
    {
        .id = PID_G_TABLE_REFERENCE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(0x7000)
    }
};

static knx_property_t properties_4[] = {
    {
        .id = PID_G_OBJECT_TYPE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(OT_INTERACE_PROGRAM)
    },
    {
        .id = PID_G_LOAD_STATE_CONTROL,
        .type = KNX_PROPERTY_TYPE_CONTROL,
        .flags = KNX_PROPERTY_FLAG_WRITABLE | KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = &dummy
    },
    {
        .id = PID_G_RUN_STATE_CONTROL,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_CHAR,
        .flags = KNX_PROPERTY_FLAG_NONE,
    },
    {
        .id = PID_G_PROGRAM_VERSION,
        .type = KNX_PROPERTY_TYPE_GENERIC5,
        .flags = KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = &dummy
    },
    {
        .id = PID_G_TABLE_REFERENCE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(0x00)
    }
};

knx_property_t properties_ota[] = {
    {
        .id = PID_G_OBJECT_TYPE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(OT_USER_OTA)
    },
    {
        .id = PID_USER_OTA_SLOT,
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U8(0x00)
    },
    {
        .id = PID_USER_OTA_SIZE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_LONG,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U32(0x00)
    },
    {
        .id = PID_USER_OTA_FLASH,
        .type = KNX_PROPERTY_TYPE_CONTROL,
        .flags = KNX_PROPERTY_FLAG_WRITABLE,
    },
    {
        .id = PID_USER_OTA_STATE,
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U8(0x00)
    },
};

knx_property_t properties_shell[] = {
    {
        .id = PID_G_OBJECT_TYPE,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = VALUE_U16(OT_USER_SHELL)
    },
    {
        .id = PID_USER_SHELL_PIPE,
        .type = KNX_PROPERTY_TYPE_GENERIC9,
        .flags = KNX_PROPERTY_FLAG_WRITABLE | KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = &dummy
    }
};

knx_property_object_t objects[] = {
    {
        .properties = properties_device,
        .count = ARRAY_SIZE(properties_device)
    },
    {
        .properties = properties_1,
        .count = ARRAY_SIZE(properties_1)
    },
    {
        .properties = properties_2,
        .count = ARRAY_SIZE(properties_2)
    },
    {
        .properties = properties_3,
        .count = ARRAY_SIZE(properties_3)
    },
    {
        .properties = properties_4,
        .count = ARRAY_SIZE(properties_4)
    },
    {
        .properties = properties_ota,
        .count = ARRAY_SIZE(properties_ota)
    },
    {
        .properties = properties_shell,
        .count = ARRAY_SIZE(properties_shell)
    },
    KNX_PROPERTY_OBJECT_END
};
