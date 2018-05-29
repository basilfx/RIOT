/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ncn5120
 *
 * @{
 * @file
 * @brief       Variables for the NCN5120 netdev interface
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NCN5120_NETDEV_H
#define NCN5120_NETDEV_H

#include "net/netdev.h"
#include "ncn5120.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Implementation of netdev_driver_t for NCN5120 device
 */
extern const netdev_driver_t netdev_ncn5120_driver;

/**
 * @brief NCN5120 netdev struct
 */
typedef struct {
    netdev_t netdev;            /**< Netdev instance */
    ncn5120_t ncn5120;          /**< NCN5120 instance */
} netdev_ncn5120_t;

/**
 * @brief Setup a NCN5120 Netdev instance.
 *
 * @param[out] netdev_ncn5120   NCN5120 netdev interface to configuare
 * @param[in] params            NCN5120 device parameters.
 *
 * @return                      NCN5120_OK on success
 */
int netdev_ncn5120_setup(netdev_ncn5120_t *netdev_ncn5120,
                         const ncn5120_params_t *params);

#ifdef __cplusplus
}
#endif

#endif /* NCN5120_NETDEV_H */
/** @} */
