/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_netdev_knx
 * @{
 *
 * @file
 * @brief       Common code for netdev KNX drivers
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <assert.h>
#include <errno.h>

#include "net/netdev.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

int netdev_knx_get(netdev_t *dev, netopt_t opt, void *value, size_t max_len)
{
    (void)dev;
    (void)max_len;

    int res = 0;

    switch (opt) {
        case NETOPT_DEVICE_TYPE:
        {
            uint16_t *tgt = (uint16_t *)value;
            *tgt = NETDEV_TYPE_KNX;
            res = 2;
            break;
        }
        case NETOPT_ADDR_LEN:
        case NETOPT_SRC_LEN:
        {
            assert(max_len == 2);
            uint16_t *tgt = (uint16_t *)value;
            *tgt = 2;
            res = sizeof(uint16_t);
            break;
        }
        case NETOPT_IS_WIRED:
        {
            res = 1;
            break;
        }
#ifdef MODULE_L2FILTER
        case NETOPT_L2FILTER:
        {
            assert(max_len >= sizeof(l2filter_t * *));
            *((l2filter_t **)value) = dev->filter;
            res = sizeof(l2filter_t * *);
            break;
        }
#endif
        default:
        {
            res = -ENOTSUP;
            break;
        }
    }

    return res;
}

int netdev_knx_set(netdev_t *dev, netopt_t opt, const void *value, size_t value_len)
{
#ifndef MODULE_L2FILTER
    (void)dev;
#endif
    (void)value;
    (void)value_len;

    int res = 0;

    switch (opt) {
#ifdef MODULE_L2FILTER
        case NETOPT_L2FILTER:
            res = l2filter_add(dev->filter, value, value_len);
            break;
        case NETOPT_L2FILTER_RM:
            res = l2filter_rm(dev->filter, value, value_len);
            break;
#endif
        default:
            res = -ENOTSUP;
            break;
    }

    return res;
}
