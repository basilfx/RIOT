/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "com_objects.h"

#define LOC(object) ((uint32_t)&(object))

com_objects_t values;

knx_com_object_t com_objects[] = {
    [COM_OBJECT_SYSTEM_TIME] = {
        .type = KNX_COM_OBJECT_TYPE_DATA8,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.date_time)
    },

    [COM_OBJECT_CHANNEL0_SWITCH_A] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[0].switch_a)
    },
    [COM_OBJECT_CHANNEL0_SWITCH_B] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[0].switch_b)
    },
    [COM_OBJECT_CHANNEL0_SWITCH_C] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[0].switch_c)
    },
    [COM_OBJECT_CHANNEL0_DIMMING] = {
        .type = KNX_COM_OBJECT_TYPE_BIT4,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[0].dimming)
    },
    [COM_OBJECT_CHANNEL0_VALUE] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[0].value)
    },

    [COM_OBJECT_CHANNEL1_SWITCH_A] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[1].switch_a)
    },
    [COM_OBJECT_CHANNEL1_SWITCH_B] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[1].switch_b)
    },
    [COM_OBJECT_CHANNEL1_SWITCH_C] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[1].switch_c)
    },
    [COM_OBJECT_CHANNEL1_DIMMING] = {
        .type = KNX_COM_OBJECT_TYPE_BIT4,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[1].dimming)
    },
    [COM_OBJECT_CHANNEL1_VALUE] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[1].value)
    },

    [COM_OBJECT_CHANNEL2_SWITCH_A] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[2].switch_a)
    },
    [COM_OBJECT_CHANNEL2_SWITCH_B] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[2].switch_b)
    },
    [COM_OBJECT_CHANNEL2_SWITCH_C] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[2].switch_c)
    },
    [COM_OBJECT_CHANNEL2_DIMMING] = {
        .type = KNX_COM_OBJECT_TYPE_BIT4,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[2].dimming)
    },
    [COM_OBJECT_CHANNEL2_VALUE] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[2].value)
    },

    [COM_OBJECT_CHANNEL3_SWITCH_A] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[3].switch_a)
    },
    [COM_OBJECT_CHANNEL3_SWITCH_B] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[3].switch_b)
    },
    [COM_OBJECT_CHANNEL3_SWITCH_C] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[3].switch_c)
    },
    [COM_OBJECT_CHANNEL3_DIMMING] = {
        .type = KNX_COM_OBJECT_TYPE_BIT4,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[3].dimming)
    },
    [COM_OBJECT_CHANNEL3_VALUE] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[3].value)
    },

    [COM_OBJECT_CHANNEL4_SWITCH_A] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[4].switch_a)
    },
    [COM_OBJECT_CHANNEL4_SWITCH_B] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[4].switch_b)
    },
    [COM_OBJECT_CHANNEL4_SWITCH_C] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[4].switch_c)
    },
    [COM_OBJECT_CHANNEL4_DIMMING] = {
        .type = KNX_COM_OBJECT_TYPE_BIT4,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[4].dimming)
    },
    [COM_OBJECT_CHANNEL4_VALUE] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[4].value)
    },

    [COM_OBJECT_CHANNEL5_SWITCH_A] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[5].switch_a)
    },
    [COM_OBJECT_CHANNEL5_SWITCH_B] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[5].switch_b)
    },
    [COM_OBJECT_CHANNEL5_SWITCH_C] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[5].switch_c)
    },
    [COM_OBJECT_CHANNEL5_DIMMING] = {
        .type = KNX_COM_OBJECT_TYPE_BIT4,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[5].dimming)
    },
    [COM_OBJECT_CHANNEL5_VALUE] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.interface[5].value)
    },

    [COM_OBJECT_SENSOR0_VALUE] = {
        .type = KNX_COM_OBJECT_TYPE_BYTE2,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.sensor0_value)
    },
    [COM_OBJECT_SENSOR1_VALUE] = {
        .type = KNX_COM_OBJECT_TYPE_BYTE2,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.sensor1_value)
    },
    [COM_OBJECT_SENSOR2_VALUE] = {
        .type = KNX_COM_OBJECT_TYPE_BYTE2,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.sensor2_value)
    },
    [COM_OBJECT_SENSOR3_VALUE] = {
        .type = KNX_COM_OBJECT_TYPE_FLOAT,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.ptr = &(values.sensor3_value)
    },

    KNX_COM_OBJECT_END
};
