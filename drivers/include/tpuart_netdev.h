/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     drivers_tpuart
 *
 * @{
 * @file
 * @brief       Variables for the TPUART netdev interface
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef TPUART_NETDEV_H
#define TPUART_NETDEV_H

#include "net/netdev.h"
#include "tpuart.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Implementation of netdev_driver_t for TPUART device
 */
extern const netdev_driver_t netdev_tpuart_driver;

/**
 * @brief   TPUART netdev struct
 */
typedef struct {
    netdev_t netdev;        /**< netdev instance */
    tpuart_t tpuart;        /**< tpuart instance */
} netdev_tpuart_t;

/**
 * @brief Setup a TPUART Netdev instance.
 *
 * @param[out] netdev_tpuart    TPUART netdev interface to configuare
 * @param[in] params            TPUART device parameters.
 *
 * @return                      TPUART_OK on success
 */
int netdev_tpuart_setup(netdev_tpuart_t *netdev_tpuart,
                        const tpuart_params_t *params);

#ifdef __cplusplus
}
#endif

#endif /* TPUART_NETDEV_H */
/** @} */
