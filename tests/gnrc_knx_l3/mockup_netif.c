/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "msg.h"
#include "net/gnrc.h"
#include "net/gnrc/netif/knx.h"
#include "net/netdev_test.h"
#include "sched.h"
#include "thread.h"

#define _MSG_QUEUE_SIZE  (2)

gnrc_netif_t *_mock_netif = NULL;

static netdev_test_t _mock_netdev;
static char _mock_netif_stack[THREAD_STACKSIZE_DEFAULT];
static gnrc_netreg_entry_t dumper;
static msg_t _main_msg_queue[_MSG_QUEUE_SIZE];

int _get_device_type(netdev_t *dev, void *value, size_t max_len)
{
    (void)dev;
    assert(max_len == sizeof(uint16_t));
    *((uint16_t *)value) = NETDEV_TYPE_KNX;
    return sizeof(uint16_t);
}

int _get_max_packet_size(netdev_t *dev, void *value, size_t max_len)
{
    (void)dev;
    assert(max_len == sizeof(uint16_t));
    *((uint16_t *)value) = 64;
    return sizeof(uint16_t);
}

int _get_address_long(netdev_t *dev, void *value, size_t max_len)
{
    static const uint8_t addr[] = { 0x00, 0xff };

    (void)dev;
    assert(max_len >= sizeof(addr));
    memcpy(value, addr, sizeof(addr));
    return sizeof(addr);
}

void _tests_init(void)
{
    msg_init_queue(_main_msg_queue, _MSG_QUEUE_SIZE);
    netdev_test_setup(&_mock_netdev, 0);
    netdev_test_set_get_cb(&_mock_netdev, NETOPT_DEVICE_TYPE,
                           _get_device_type);
    netdev_test_set_get_cb(&_mock_netdev, NETOPT_MAX_PACKET_SIZE,
                           _get_max_packet_size);
    netdev_test_set_get_cb(&_mock_netdev, NETOPT_ADDRESS,
                           _get_address_long);
    _mock_netif = gnrc_netif_knx_create(
        _mock_netif_stack, THREAD_STACKSIZE_DEFAULT, GNRC_NETIF_PRIO,
        "mockup_knx", &_mock_netdev.netdev);
    assert(_mock_netif != NULL);
    gnrc_netreg_entry_init_pid(&dumper, GNRC_NETREG_DEMUX_CTX_ALL,
                               sched_active_pid);
    gnrc_netreg_register(GNRC_NETTYPE_UNDEF, &dumper);
}
