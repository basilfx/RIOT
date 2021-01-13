/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

/**
 * @defgroup    drivers_netdev_knx KNX drivers
 * @ingroup     drivers_netdev_api
 * @{
 *
 * @file
 * @brief       Definitions for netdev common KNX code
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NET_NETDEV_KNX_H
#define NET_NETDEV_KNX_H

#include <stdint.h>

#include "net/netdev.h"
#include "net/netopt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Fallback function for netdev KNX devices' _get function
 *
 * Supposed to be used by netdev drivers as default case.
 *
 * @warning Driver *MUST* implement NETOPT_ADDRESS case!
 *
 * @param[in]   dev     network device descriptor
 * @param[in]   opt     option type
 * @param[out]  value   pointer to store the option's value in
 * @param[in]   max_len maximal amount of byte that fit into @p value
 *
 * @return              number of bytes written to @p value
 * @return              <0 on error
 */
int netdev_knx_get(netdev_t *dev, netopt_t opt, void *value, size_t max_len);

/**
 * @brief   Fallback function for netdev KNX devices' _set function
 *
 * @param[in] dev       network device descriptor
 * @param[in] opt       option type
 * @param[in] value     value to set
 * @param[in] value_len the length of @p value
 *
 * @return              number of bytes used from @p value
 * @return              <0 on error
 */
int netdev_knx_set(netdev_t *dev, netopt_t opt, const void *value, size_t value_len);

#ifdef __cplusplus
}
#endif

#endif /* NET_NETDEV_KNX_H */
/** @} */
