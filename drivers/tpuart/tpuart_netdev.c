/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_tpuart
 *
 * @{
 * @file
 * @brief       Implementation of netdev interface for TPUART
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 * @}
 */

#include <string.h>
#include <errno.h>

#include "assert.h"
#include "net/netdev.h"
#include "net/netdev/knx.h"
#include "tpuart_netdev.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static void _cb(uint8_t event, void *data, void *arg)
{
    (void)arg;
    (void)data;

    netdev_tpuart_t *netdev_tpuart = (netdev_tpuart_t *)arg;

    switch (event) {
        case TPUART_EVENT_TELEGRAM:
            if (netdev_tpuart->netdev.event_callback != NULL) {
                netdev_tpuart->netdev.event_callback((netdev_t *)arg, NETDEV_EVENT_RX_COMPLETE);
            }
            break;
    }
}

static int _init(netdev_t *netdev)
{
    netdev_tpuart_t *netdev_tpuart = (netdev_tpuart_t *)netdev;
    tpuart_t *dev = &netdev_tpuart->tpuart;

    /* initialize tpuart driver */
    if (tpuart_init(dev, (const tpuart_params_t *)&dev->params) != TPUART_OK) {
        DEBUG("[tpuart_netdev] _init: unable to setup tpuart device\n");
        return -ENXIO;
    }

    /* reset device */
    if (tpuart_reset(dev) != TPUART_OK) {
        DEBUG("[tpuart_netdev] _init: unable to reset tpuart device\n");
        return -ENXIO;
    }

    /* set callback */
    dev->cb = _cb;
    dev->cb_arg = (void *)netdev;

    DEBUG("[tpuart_netdev] _init: initialization done\n");
    return 0;
}

static int _send(netdev_t *netdev, const iolist_t *iolist)
{
    netdev_tpuart_t *netdev_tpuart = (netdev_tpuart_t *)netdev;
    tpuart_t *dev = &((netdev_tpuart_t *)netdev)->tpuart;

    int result = tpuart_send(dev, iolist);

    if (netdev_tpuart->netdev.event_callback != NULL) {
        if (result >= 0) {
            netdev_tpuart->netdev.event_callback(netdev, NETDEV_EVENT_TX_COMPLETE);
        }
        else {
            netdev_tpuart->netdev.event_callback(netdev, NETDEV_EVENT_TX_TIMEOUT);
            DEBUG("[tpuart_netdev]: _send: failed with result %d", result);
        }
    }

    return result;
}

static int _recv(netdev_t *netdev, void *buf, size_t len, void *info)
{
    (void)info;

    tpuart_t *dev = &((netdev_tpuart_t *)netdev)->tpuart;
    size_t size = dev->telegram_size;

    /* return telegram size, and drop it if requested */
    if (buf == NULL) {
        if (len > 0) {
            dev->telegram_size = 0;
        }

        return size;
    }

    /* drop telegram if buffer is too small */
    if (size > len) {
        DEBUG("[tpuart_netdev]: _recv: buffer too small\n");
        dev->telegram_size = 0;

        return -ENOBUFS;
    }

    /* copy telegram into buffer */
    memcpy(buf, (void *)dev->buf, size);
    dev->telegram_size = 0;

    return size;
}

static void _isr(netdev_t *netdev)
{
    (void)netdev;
}

static int _get(netdev_t *netdev, netopt_t opt, void *value, size_t max_len)
{
    tpuart_t *dev = &((netdev_tpuart_t *)netdev)->tpuart;

    switch (opt) {
        case NETOPT_ADDRESS:
            assert(max_len >= sizeof(uint16_t));
            *((uint16_t *)value) = dev->address;
            return sizeof(uint16_t);
        case NETOPT_PHY_BUSY:
            *((netopt_enable_t *)value) = dev->busy ? NETOPT_ENABLE : NETOPT_DISABLE;
            return sizeof(netopt_enable_t);
        case NETOPT_CHECKSUM:
            *((netopt_enable_t *)value) = dev->crc ? NETOPT_ENABLE : NETOPT_DISABLE;
            return sizeof(netopt_enable_t);
        default:
            return netdev_knx_get(&((netdev_tpuart_t *)netdev)->netdev, opt, value, max_len);
    }
}

static int _set(netdev_t *netdev, netopt_t opt, const void *value, size_t value_len)
{
    tpuart_t *dev = &((netdev_tpuart_t *)netdev)->tpuart;

    switch (opt) {
        case NETOPT_ADDRESS:
            assert(value_len == sizeof(uint16_t));
            if (tpuart_set_address(dev, *((uint16_t *)value)) != TPUART_OK) {
                return -EIO;
            }
            return sizeof(uint16_t);
        case NETOPT_PHY_BUSY:
            if (((const bool *)value)[0]) {
                if (tpuart_activate_busy_mode(dev) != TPUART_OK) {
                    return -EIO;
                }
            }
            else {
                if (tpuart_reset_busy_mode(dev) != TPUART_OK) {
                    return -EIO;
                }
            }
            return sizeof(netopt_enable_t);
        case NETOPT_CHECKSUM:
            if (((const bool *)value)[0]) {
                if (tpuart_activate_crc(dev) != TPUART_OK) {
                    return -EIO;
                }

                return sizeof(netopt_enable_t);
            }
            else {
                /* checksum is an activate-only option */
                return -ENOTSUP;
            }
        default:
            return netdev_knx_set(&((netdev_tpuart_t *)netdev)->netdev, opt, value, value_len);
    }
}

const netdev_driver_t netdev_tpuart_driver = {
    .send = _send,
    .recv = _recv,
    .init = _init,
    .isr = _isr,
    .get = _get,
    .set = _set,
};

int netdev_tpuart_setup(netdev_tpuart_t *netdev_tpuart, const tpuart_params_t *params)
{
    netdev_tpuart->netdev.driver = &netdev_tpuart_driver;
    netdev_tpuart->tpuart.params = *params;

    return TPUART_OK;
}
