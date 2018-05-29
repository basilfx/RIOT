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
 * @brief       Implementation of netdev interface for NCN5120
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 * @}
 */

#include <string.h>
#include <errno.h>

#include "assert.h"
#include "net/netdev.h"
#include "net/netdev/knx.h"
#include "ncn5120_netdev.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static void _cb(uint8_t event, void *data, void *arg)
{
    (void)arg;
    (void)data;

    netdev_ncn5120_t *netdev_ncn5120 = (netdev_ncn5120_t *)arg;

    switch (event) {
        case NCN5120_EVENT_TELEGRAM:
            if (netdev_ncn5120->netdev.event_callback != NULL) {
                netdev_ncn5120->netdev.event_callback((netdev_t *)arg, NETDEV_EVENT_RX_COMPLETE);
            }
            break;
    }
}

static int _init(netdev_t *netdev)
{
    netdev_ncn5120_t *netdev_ncn5120 = (netdev_ncn5120_t *)netdev;
    ncn5120_t *dev = &netdev_ncn5120->ncn5120;

    /* initialize ncn5120 driver */
    if (ncn5120_init(dev, (const ncn5120_params_t *)&dev->params) != NCN5120_OK) {
        DEBUG("[ncn5120_netdev] _init: unable to setup ncn5120 device\n");
        return -ENXIO;
    }

    /* reset device */
    if (ncn5120_reset(dev) != NCN5120_OK) {
        DEBUG("[ncn5120_netdev] _init: unable to reset ncn5120 device\n");
        return -ENXIO;
    }

    /* set callback */
    dev->cb = _cb;
    dev->cb_arg = (void *)netdev;

    DEBUG("[ncn5120_netdev] _init: initialization done\n");
    return 0;
}

static int _send(netdev_t *netdev, const iolist_t *iolist)
{
    netdev_ncn5120_t *netdev_ncn5120 = (netdev_ncn5120_t *)netdev;
    ncn5120_t *dev = &((netdev_ncn5120_t *)netdev)->ncn5120;

    int result = ncn5120_send(dev, iolist);

    if (netdev_ncn5120->netdev.event_callback != NULL) {
        if (result >= 0) {
            netdev_ncn5120->netdev.event_callback(netdev, NETDEV_EVENT_TX_COMPLETE);
        }
        else {
            netdev_ncn5120->netdev.event_callback(netdev, NETDEV_EVENT_TX_TIMEOUT);
            DEBUG("[ncn5120_netdev]: _send: failed with result %d", result);
        }
    }

    return result;
}

static int _recv(netdev_t *netdev, void *buf, size_t len, void *info)
{
    (void)info;

    ncn5120_t *dev = &((netdev_ncn5120_t *)netdev)->ncn5120;
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
        DEBUG("[ncn5120_netdev]: _recv: buffer too small\n");
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
    ncn5120_t *dev = &((netdev_ncn5120_t *)netdev)->ncn5120;

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
            return netdev_knx_get(&((netdev_ncn5120_t *)netdev)->netdev, opt, value, max_len);
    }
}

static int _set(netdev_t *netdev, netopt_t opt, const void *value, size_t value_len)
{
    ncn5120_t *dev = &((netdev_ncn5120_t *)netdev)->ncn5120;

    switch (opt) {
        case NETOPT_ADDRESS:
            assert(value_len == sizeof(uint16_t));
            if (ncn5120_set_address(dev, *((uint16_t *)value)) != 0) {
                return -EIO;
            }
            return sizeof(uint16_t);
        case NETOPT_PHY_BUSY:
            if (((const bool *)value)[0]) {
                if (ncn5120_set_busy_mode(dev) != NCN5120_OK) {
                    return -EIO;
                }
            }
            else {
                if (ncn5120_quit_busy(dev) != NCN5120_OK) {
                    return -EIO;
                }
            }
            return sizeof(netopt_enable_t);
        case NETOPT_CHECKSUM:
            if (((const bool *)value)[0]) {
                if (ncn5120_configure(dev, NCN5120_CONFIGURE_CRC_CTIT) != 0) {
                    return -EIO;
                }
            }
            else {
                if (ncn5120_configure(dev, NCN5120_CONFIGURE_NONE) != 0) {
                    return -EIO;
                }
            }
            return sizeof(netopt_enable_t);
        default:
            return netdev_knx_set(&((netdev_ncn5120_t *)netdev)->netdev, opt, value, value_len);
    }
}

const netdev_driver_t netdev_ncn5120_driver = {
    .send = _send,
    .recv = _recv,
    .init = _init,
    .isr = _isr,
    .get = _get,
    .set = _set,
};

int netdev_ncn5120_setup(netdev_ncn5120_t *netdev_ncn5120, const ncn5120_params_t *params)
{
    netdev_ncn5120->netdev.driver = &netdev_ncn5120_driver;
    netdev_ncn5120->ncn5120.params = *params;

    return NCN5120_OK;
}
