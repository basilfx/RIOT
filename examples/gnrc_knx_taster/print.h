/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef PRINT_H
#define PRINT_H

#include "knx_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Print hardware-level information
 */
void print_hardware_info(void);

/**
 * @brief   Print device-level information
 */
void print_device_info(knx_device_t *device);

/**
 * @brief   Print device settings information
 */
void print_settings(knx_device_t *device);

#ifdef __cplusplus
}
#endif

#endif /* PRINT_H */
/** @} */
