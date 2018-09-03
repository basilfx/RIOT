/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_knx_device_system7    KNX Device System 7
 * @ingroup     sys
 * @brief       KNX device types and helper functions
 *
 * # About
 * KNX System 7 device model supports up to 255 communication objects, and has
 * a memory space of 65535 bytes.
 *
 * # System model
 * The model recognizes several modules.
 *
 * - Communication objects (also known as group objects)
 * - Memory
 * - Properties
 *
 * Communication objects are the group objects that are are broadcasted over
 * the bus when sensors or actors change values.
 *
 * Memory is emulated. Several memory ranges are used while programming
 * devices.
 *
 * Devices can be configured using memory or properties. Properties have the
 * benefit of being memory-address independent, and it is therefore not
 * required to know the memory layout of a device. Properties are typically
 * mapped onto memory. Mask versions 0705 makes extensive use of properties.
 *
 * @{
 *
 * @file
 * @brief       KNX device interface
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef KNX_DEVICE_SYSTEM7_DEVICE_H
#define KNX_DEVICE_SYSTEM7_DEVICE_H

#include <stdint.h>
#include <stdbool.h>

#include "net/netif.h"
#include "net/knx.h"

#include "knx_device/system7/assoc.h"
#include "knx_device/system7/com_object.h"
#include "knx_device/system7/events.h"
#include "knx_device/system7/memory.h"
#include "knx_device/system7/property.h"
#include "knx_device/system7/tables.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   KNX device mask version
 * 
 * A mask version indicates the implemented system version and its
 * capabilities.
 */
typedef enum {
    KNX_DEVICE_MASK_0001 = 0x0001,      /**< Mask version 0001 (Dummy) */
    KNX_DEVICE_MASK_0010 = 0x0010,      /**< Mask version 0010 */
    KNX_DEVICE_MASK_0011 = 0x0011,      /**< Mask version 0011 */
    KNX_DEVICE_MASK_0012 = 0x0012,      /**< Mask version 0012 (System 1) */
    KNX_DEVICE_MASK_0013 = 0x0013,      /**< Mask version 0013 */
    KNX_DEVICE_MASK_0020 = 0x0020,      /**< Mask version 0020 */
    KNX_DEVICE_MASK_0021 = 0x0021,      /**< Mask version 0021 (System 2) */
    KNX_DEVICE_MASK_0025 = 0x0025,      /**< Mask version 0025 (System 2) */
    KNX_DEVICE_MASK_0300 = 0x0300,      /**< Mask version 0300 */
    KNX_DEVICE_MASK_0310 = 0x0310,      /**< Mask version 0310 */
    KNX_DEVICE_MASK_0700 = 0x0700,      /**< Mask version 0700 */
    KNX_DEVICE_MASK_0701 = 0x0701,      /**< Mask version 0701 (System 7, TP) */
    KNX_DEVICE_MASK_0705 = 0x0705,      /**< Mask version 0705 (System 7, TP) */
    KNX_DEVICE_MASK_07B0 = 0x07B0,      /**< Mask version 07B0 (System B, TP) */
    KNX_DEVICE_MASK_0900 = 0x0900,      /**< Mask version 0900 */
    KNX_DEVICE_MASK_091A = 0x091A,      /**< Mask version 091A */
    KNX_DEVICE_MASK_1011 = 0x1011,      /**< Mask version 1011 */
    KNX_DEVICE_MASK_1012 = 0x1012,      /**< Mask version 1012 */
    KNX_DEVICE_MASK_1013 = 0x1013,      /**< Mask version 1013 */
    KNX_DEVICE_MASK_1310 = 0x1310,      /**< Mask version 1310 */
    KNX_DEVICE_MASK_17B0 = 0x17B0,      /**< Mask version 17B0 */
    KNX_DEVICE_MASK_1900 = 0x1900,      /**< Mask version 1900 */
    KNX_DEVICE_MASK_5705 = 0x5705,      /**< Mask version 5705 (Sytem 7, KNXnet/IP) */
} knx_device_mask_t;

/**
 * @brief   Forward declaration for KNX device struct
 */
typedef struct knx_device knx_device_t;

/**
 * @brief   KNX device event callback definition
 *
 * @param[in] dev           The KNX device
 * @param[in] events        Device event
 * @param[in] args          Optional pointer to arguments
 */
typedef void (*knx_device_event_cb_t)(knx_device_t *dev, knx_device_event_t event, void *args);

/**
 * @brief   KNX device structure
 */
struct knx_device {
    knx_device_event_cb_t event_callback;       /**< callback for device events */
    netif_t *iface;                             /**< device interface */

    knx_addr_physical_t address;
    knx_device_mask_t mask_version;

    knx_table_device_t *info;

    knx_memory_segment_t *segments;
    knx_property_object_t *objects;
    knx_com_object_t *com_objects;
    knx_assoc_t *associations;
};

#ifdef __cplusplus
}
#endif

#endif /* KNX_DEVICE_SYSTEM7_DEVICE_H */
/** @} */
