/*
 * Copyright (C) 2016 Toon Stegen
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Toon Stegen <toon.stegen@altran.com>
 */

#include <assert.h>
#include <sys/uio.h>
#include <inttypes.h>

#include "net/ieee802154.h"
#include "net/netdev2.h"
#include "net/netopt.h"
#include "utlist.h"
#include "thread.h"
#include "kernel_defines.h"

#define ENABLE_DEBUG                (0)
#include "debug.h"

#define PICOTCP_NETDEV2_NAME           "picotcp_netdev2_mux"
#define PICOTCP_NETDEV2_PRIO           (THREAD_PRIORITY_MAIN - 4)
#define PICOTCP_NETDEV2_STACKSIZE      (THREAD_STACKSIZE_DEFAULT)
#define PICOTCP_NETDEV2_QUEUE_LEN      (8)
#define PICOTCP_NETDEV2_MSG_TYPE_EVENT 0x1235

static kernel_pid_t _pid = KERNEL_PID_UNDEF;
static char _stack[PICOTCP_NETDEV2_STACKSIZE];
static msg_t _queue[PICOTCP_NETDEV2_QUEUE_LEN];
static char _tmp_buf[PICOTCP_NETDEV2_BUFLEN];

static void _event_cb(netdev2_t *dev, netdev2_event_t event, void *arg);
static void *_event_loop(void *arg);

static int _link_state;

static netdev2_t *_get_netdev(struct pico_device *pico_dev)
{
    return container_of(pico_dev, netdev2_t, isr_arg);
}

/* Send function. Return 0 if busy */
static int _netdev2_send(struct pico_device *pico_dev, void *buf, int len)
{
    netdev2_t *dev = _get_netdev(pico_dev);

    return 0;
}

static int _netdev2_dsr(struct pico_device *pico_dev, int loop_score)
{
    netdev2_t *dev = _get_netdev(pico_dev);
    (void) loop_score;
    return 0;
}

static int _netdev2_link_state(struct pico_device *pico_dev)
{
    netdev2_t *dev = _get_netdev(pico_dev);
    return _link_state;
}

static void _netdev2_destroy(struct pico_device *pico_dev)
{
    netdev2_t *dev = _get_netdev(pico_dev);
}

static void _event_cb(netdev2_t *dev, netdev2_event_t event, void *arg)
{
    int len;
    struct pico_device * pico_dev = (struct pico_device *) arg;

    if (event == NETDEV2_EVENT_ISR) {
        /* driver needs it's ISR handled */
        msg_t msg;

        msg.type = PICOTCP_NETDEV2_MSG_TYPE_EVENT;
        msg.content.ptr = (char *)dev;

        if (msg_send(&msg, _pid) <= 0) {
            DEBUG("picotcp_netdev2: possibly lost interrupt.\n");
        }
    }
    switch(event) {
        case NETDEV2_EVENT_RX_STARTED:
            /* started to receive a packet */
        break;
        case NETDEV2_EVENT_RX_COMPLETE:
            /* finished receiving a packet */
            len = dev->driver->recv(dev, _tmp_buf, sizeof(_tmp_buf), NULL);
            pico_stack_recv(&pico_dev, _tmp_buf, len);
        break;
        case NETDEV2_EVENT_TX_STARTED:
            /* started to transfer a packet */
        break;
        case NETDEV2_EVENT_TX_COMPLETE:
            /* finished transferring packet */
        break;
        case NETDEV2_EVENT_TX_NOACK:
            /* ACK requested but not received */
        break;
        case NETDEV2_EVENT_TX_MEDIUM_BUSY:
            /* couldn't transfer packet */
        break;
        case NETDEV2_EVENT_LINK_UP:
            /* link established */
            _link_state = 1;
        break;
        case NETDEV2_EVENT_LINK_DOWN:
            /* link gone */
            _link_state = 0;
        break;
        default:
        break;
    }
}

static void *_event_loop(void *arg)
{
    (void)arg;
    msg_init_queue(_queue, PICOTCP_NETDEV2_QUEUE_LEN);
    while (1) {
        msg_t msg;
        msg_receive(&msg);
        if (msg.type == PICOTCP_NETDEV2_MSG_TYPE_EVENT) {
            netdev2_t *dev = (netdev2_t *)msg.content.ptr;
            dev->driver->isr(dev);
        }
    }
    return NULL;
}

int picotcp_netdev2_init(struct pico_device *pico_dev)
{
    netdev2_t *netdev;
    uint16_t dev_type;
    uint8_t *mac;

    /* start multiplexing thread (only one needed) */
    if (_pid <= KERNEL_PID_UNDEF) {
        _pid = thread_create(_stack, PICOTCP_NETDEV2_STACKSIZE, PICOTCP_NETDEV2_PRIO,
                             THREAD_CREATE_STACKTEST, _event_loop, NULL,
                             PICOTCP_NETDEV2_NAME);
        if (_pid <= 0) {
            return -1;
        }
    }

    /* initialize netdev */
    netdev->driver->init(netdev);
    netdev->isr_arg = pico_dev;
    netdev->event_callback = _event_cb;

    if (netdev->driver->get(netdev, NETOPT_DEVICE_TYPE, &dev_type,
                            sizeof(dev_type)) < 0) {
        return -1;
    }

    switch (dev_type) {
#ifdef MODULE_NETDEV2_ETH
        case NETDEV2_TYPE_ETHERNET:
            /* */
            break;
#endif
        case NETDEV2_TYPE_UNKNOWN:
        case NETDEV2_TYPE_RAW:
        case NETDEV2_TYPE_IEEE802154:
        case NETDEV2_TYPE_CC110X:
        default:
            /* device type not supported yet */
            return -1;
    }

    /* *retreive mac address */
    if (netdev->driver->get(netdev, NETOPT_ADDRESS , &mac,
                            sizeof(dev_type)) < 0) {
        return -1;
    }

    /* initialize pico_device */

    if( 0 != pico_device_init(pico_dev, "pico_device", mac)) {
        dbg("Device init failed.\n");
        PICO_FREE(eth_dev);
        return NULL;
    }

    return 0;
}

/** @} */
