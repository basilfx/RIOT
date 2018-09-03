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
 * @brief       KNX device definitions for the in-memory tables
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef KNX_DEVICE_SYSTEM7_TABLES_H
#define KNX_DEVICE_SYSTEM7_TABLES_H

#include "knx_device/system7/property.h"

#include "byteorder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   KNX device info RAM table
 */
typedef struct __attribute__((packed)) {
    uint8_t programming_mode;
    uint8_t serial[6];
    be_uint16_t manufacturer_id;
    uint8_t hardware_type[6];
    uint8_t order_info[10];
    uint8_t device_control;
} knx_table_device_t;

/**
 * @brief   KNX communication objects table
 */
typedef struct __attribute__((packed)) {
    uint8_t dummy[3];
    struct {
        uint8_t dummy[2];
        uint8_t flags;
        uint8_t type;
    } objects[255];
} knx_table_com_objects_t;

/**
 * @brief   KNX group address table
 */
typedef struct __attribute__((packed)) {
    uint8_t count;
    uint8_t dummy[2];
    network_uint16_t addrs[255];
} knx_table_addr_t;

/**
 * @brief   KNX association table
 */
typedef struct __attribute__((packed)) {
    uint8_t count;
    struct {
        uint8_t addr_index;
        uint8_t com_object_index;
    } associations[255];
} knx_table_assoc_t;

/**
 * @brief   KNX load state table.
 */
typedef struct __attribute__((packed)) {
    uint8_t addr_table_load_state;
    uint8_t assoc_table_load_state;
    uint8_t com_object_table_load_state;
    uint8_t pei_prog_load_state;
} knx_table_load_state_t;

#ifdef __cplusplus
}
#endif

#endif /* KNX_DEVICE_SYSTEM7_TABLES_H */
/** @} */
