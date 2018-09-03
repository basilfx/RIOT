/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_knx_device_system7
 *
 * @{
 *
 * @file
 * @brief       KNX device events and event types
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef KNX_DEVICE_SYSTEM7_EVENTS_H
#define KNX_DEVICE_SYSTEM7_EVENTS_H

#include <stdint.h>
#include <stdbool.h>

#include "net/knx.h"

#include "knx_device/system7/com_object.h"
#include "knx_device/system7/memory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    KNX_DEVICE_EVENT_AUTHORIZE,
    KNX_DEVICE_EVENT_COM_OBJECT_READ,
    KNX_DEVICE_EVENT_COM_OBJECT_WRITE,
    KNX_DEVICE_EVENT_MEM_READ,
    KNX_DEVICE_EVENT_MEM_WRITE,
    KNX_DEVICE_EVENT_PROP_READ,
    KNX_DEVICE_EVENT_PROP_WRITE,
    KNX_DEVICE_EVENT_RESTART,
    KNX_DEVICE_EVENT_SET_ADDRESS
} knx_device_event_t;

typedef struct {
    uint8_t level;
} knx_event_authorize_t;

typedef struct {
    knx_com_object_t *com_object;
} knx_event_com_object_write_t;

typedef struct {
    knx_memory_segment_t *segment;
} knx_event_mem_write_t;

typedef struct {
    knx_property_t *property;
    uint8_t object;
    uint8_t count;
    uint8_t start;
} knx_event_prop_read_t;

typedef struct {
    knx_property_t *property;
    uint8_t object;
    uint8_t count;
    uint8_t start;
    uint8_t *data;
} knx_event_prop_write_t;

typedef struct {
    knx_addr_t *address;
} knx_event_set_address_t;

#ifdef __cplusplus
}
#endif

#endif /* KNX_DEVICE_SYSTEM7_EVENTS_H */
/** @} */
