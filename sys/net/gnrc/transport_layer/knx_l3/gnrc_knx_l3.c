/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 * @ingroup     net_gnrc_knx_l3
 * @file
 * @brief       Implementation of the KNX Layer 3 stack
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 * @}
 */

#include "net/gnrc/knx_l3.h"
#include "net/knx.h"

#include "xtimer.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Allocate memory for stack
 */
static char _stack[GNRC_KNX_L3_STACK_SIZE + DEBUG_EXTRA_STACKSIZE];

/**
 * @brief   PID of the thread
 */
static kernel_pid_t gnrc_knx_l3_pid = KERNEL_PID_UNDEF;

gnrc_netif_t *_get_if_by_addr(const knx_addr_t *addr)
{
    gnrc_netif_t *netif = NULL;

    while ((netif = gnrc_netif_iter(netif))) {
        if (netif->device_type == NETDEV_TYPE_KNX) {
            knx_addr_t *addr_netif = (knx_addr_t *)netif->l2addr;

            if (knx_addr_equal(addr, addr_netif)) {
                return netif;
            }
        }
    }

    return NULL;
}

static void _receive(gnrc_pktsnip_t *pkt)
{
    knx_addr_t dst;

    /* drop telegrams of wrong type */
    if (pkt->type != GNRC_NETTYPE_KNX_L3) {
        DEBUG("[gnrc_knx_l3] _receive: telegram not a network layer telegram\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    /* drop telegrams that are not telegrams */
    if (!knx_telegram_is(pkt->data, pkt->size)) {
        DEBUG("[gnrc_knx_l3] _receive: invalid or incomplete telegram\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    /* drop telegrams not addressed */
    if (!knx_telegram_is_group_addressed(pkt->data)) {
        knx_telegram_get_dst_addr(pkt->data, &dst);

        if (!knx_addr_equal(&dst, &knx_addr_broadcast)) {
            gnrc_netif_t *netif = _get_if_by_addr(&dst);

            if (netif == NULL) {
                DEBUG("[gnrc_knx_l3] _receive: telegram not for me\n");
                gnrc_pktbuf_release(pkt);
                return;
            }
        }
    }

    /* pass telegram on to the transport layer */
#ifdef MODULE_GNRC_KNX_L4
    pkt->type = GNRC_NETTYPE_KNX_L4;
#else
    pkt->type = GNRC_NETTYPE_UNDEF;
#endif

    if (!gnrc_netapi_dispatch_receive(pkt->type, GNRC_NETREG_DEMUX_CTX_ALL, pkt)) {
        DEBUG("[gnrc_knx_l3]: _receive: unable to forward telegram to transport layer\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
}

static void _send(gnrc_pktsnip_t *pkt)
{
    knx_addr_t src;

    /* drop telegrams that are not telegrams */
    if (!knx_telegram_is(pkt->data, pkt->size)) {
        DEBUG("[gnrc_knx_l3] _receive: invalid or incomplete telegram\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    /* find the interface based on source address */
    knx_telegram_get_src_addr(pkt->data, &src);

    gnrc_netif_t *netif = _get_if_by_addr(&src);

    if (netif == NULL) {
        DEBUG("[gnrc_knx_l3] _send: unable to find iface\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    /* add netif header */
    gnrc_pktsnip_t *netif_hdr = gnrc_netif_hdr_build(NULL, 0, NULL, 0);

    if (netif_hdr == NULL) {
        DEBUG("[gnrc_knx_l3] _send: unable to allocate netif header\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    LL_PREPEND(pkt, netif_hdr);

    /* forward telegram to network interface */
    if (gnrc_netapi_send(netif->pid, pkt) < 1) {
        DEBUG("[gnrc_knx_l3] _send: unable to send telegram\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    DEBUG("[gnrc_knx_l3] _send: telegram sent\n");
}

static void *_event_loop(void *args)
{
    (void)args;

    msg_t msg, reply, msg_q[GNRC_KNX_L3_MSG_QUEUE_SIZE];
    gnrc_netreg_entry_t me_reg = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                            sched_active_pid);

    msg_init_queue(msg_q, GNRC_KNX_L3_MSG_QUEUE_SIZE);

    /* register interest in all KNX packets */
    gnrc_netreg_register(GNRC_NETTYPE_KNX_L3, &me_reg);

    /* preinitialize ACK */
    reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
    reply.content.value = (uint32_t)-ENOTSUP;

    /* start event loop */
    while (1) {
        // DEBUG("[gnrc_knx_l3]: _event_loop: waiting for incoming message\n");
        msg_receive(&msg);

        switch (msg.type) {
            case GNRC_NETAPI_MSG_TYPE_RCV:
                DEBUG("[gnrc_knx_l3] _event_loop: GNRC_NETAPI_MSG_TYPE_RCV received\n");
                _receive(msg.content.ptr);
                break;
            case GNRC_NETAPI_MSG_TYPE_SND:
                DEBUG("[gnrc_knx_l3] _event_loop: GNRC_NETAPI_MSG_TYPE_SND received\n");
                _send(msg.content.ptr);
                break;
            case GNRC_NETAPI_MSG_TYPE_GET:
            case GNRC_NETAPI_MSG_TYPE_SET:
                DEBUG("[gnrc_knx_l3] _event_loop: reply to unsupported get/set\n");
                msg_reply(&msg, &reply);
                break;
            default:
                DEBUG("[gnrc_knx_l3] _event_loop: received unidentified message\n");
                break;
        }
    }

    return NULL;
}

kernel_pid_t gnrc_knx_l3_init(void)
{
    if (gnrc_knx_l3_pid == KERNEL_PID_UNDEF) {
        gnrc_knx_l3_pid = thread_create(_stack,
                                        sizeof(_stack),
                                        GNRC_KNX_L3_PRIO,
                                        THREAD_CREATE_STACKTEST,
                                        _event_loop,
                                        NULL,
                                        "knx_l3");
    }

    return gnrc_knx_l3_pid;
}
