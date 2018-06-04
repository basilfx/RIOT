/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 * @ingroup     net_gnrc_knx_l4
 * @file
 * @brief       Implementation of the KNX Layer 4 stack
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 * @}
 */

#include "net/gnrc/knx_l4.h"
#include "net/knx.h"

#include "xtimer.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Allocate memory for stack
 */
static char _stack[GNRC_KNX_L4_STACK_SIZE + DEBUG_EXTRA_STACKSIZE];

/**
 * @brief   PID of the thread
 */
static kernel_pid_t gnrc_knx_l4_pid = KERNEL_PID_UNDEF;

/**
 * @brief   (Active) KNX Layer 4 connection
 *
 * Currently, only one active connection is supported.
 */
static knx_connection_t connection;

static void _send_control(gnrc_pktsnip_t *pkt, uint8_t control, uint8_t seq)
{
    /* allocate output telegram */
    gnrc_pktsnip_t *out = gnrc_pktbuf_add(NULL, NULL, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN, GNRC_NETTYPE_KNX_L4);

    if (out == NULL) {
        DEBUG("[gnrc_knx_l4] _send_control: unable to allocate telegram\n");
        return;
    }

    if (control & 0x40) {
        control &= ~0x3c;
        control |= (seq << 2);
    }

    ((uint8_t *)out->data)[6] = control;

    /* prepare data */
    knx_addr_t src, dest;

    knx_telegram_get_src_addr(pkt->data, &src);
    knx_telegram_get_dst_addr(pkt->data, &dest);

    knx_telegram_build(out->data, KNX_TELEGRAM_TYPE_STANDARD, &dest, &src, false);
    knx_telegram_set_priority(out->data, knx_telegram_get_priority(pkt->data));

    /* send telegram */
    if (gnrc_netapi_send(gnrc_knx_l4_pid, out) != 1) {
        DEBUG("[gnrc_knx_l4] _send_control: unable to send control\n");
        gnrc_pktbuf_release(out);
        return;
    }
}

static void _forward(gnrc_pktsnip_t *pkt)
{
#ifdef MODULE_GNRC_KNX_L7
    pkt->type = GNRC_NETTYPE_KNX_L7;
#else
    pkt->type = GNRC_NETTYPE_UNDEF;
#endif

    if (!gnrc_netapi_dispatch_receive(pkt->type, GNRC_NETREG_DEMUX_CTX_ALL, pkt)) {
        DEBUG("[gnrc_knx_l4] _forward: unable to forward packet as no one is interested in it\n");
        gnrc_pktbuf_release(pkt);
    }
}

static void _handle_ucd(gnrc_pktsnip_t *pkt)
{
    DEBUG("[gnrc_knx_l4] _handle_ucd: handling UCD telegram\n");

    knx_addr_t tmp;
    knx_tpci_ucd_t ucd = knx_tpci_ucd_get(pkt->data);

    switch (ucd) {
        case KNX_TPCI_UCD_CONNECT:
            if (!connection.connected) {
                connection.connected = true;
                connection.dest_seq = 0;
                connection.src_seq = 0;
                connection.timestamp = xtimer_now_usec64();
                knx_telegram_get_src_addr(pkt->data, &(connection.dest));

                DEBUG("[gnrc_knx_l4] _handle_ucd: connected\n");
            }
            else {
                DEBUG("[gnrc_knx_l4] _handle_ucd: connection request but already connected\n");
            }

            break;
        case KNX_TPCI_UCD_DISCONNECT:
            if (connection.connected) {
                knx_telegram_get_src_addr(pkt->data, &tmp);

                if (knx_addr_equal(&(connection.dest), &tmp)) {
                    connection.connected = false;

                    DEBUG("[gnrc_knx_l4] _handle_ucd: disconnected\n");
                }
                else {
                    DEBUG("[gnrc_knx_l4] _handle_ucd: disconnect request not from destination\n");
                }
            }
            else {
                DEBUG("[gnrc_knx_l4] _handle_ucd: disconnect request but not connected\n");
            }

            break;
        default:
            DEBUG("[gnrc_knx_l4] _handle_ucd: unexpected type: %02x\n", ucd);
    }
}

static void _handle_ncd(gnrc_pktsnip_t *pkt)
{
    DEBUG("[gnrc_knx_l4] _handle_ndp: handling NCD telegram\n");

    knx_addr_t tmp;
    knx_tpci_ncd_t ncd = knx_tpci_ncd_get(pkt->data);

    switch (ncd) {
        case KNX_TPCI_NCD_ACK:
            if (connection.connected) {
                knx_telegram_get_src_addr(pkt->data, &tmp);

                if (knx_addr_equal(&(connection.dest), &tmp)) {
                    uint8_t seq_number = knx_tpci_get_seq_number(pkt->data);

                    if (seq_number == connection.src_seq) {
                        connection.timestamp = xtimer_now_usec64();
                        connection.src_seq = (connection.src_seq + 1) % 16;

                        DEBUG("[gnrc_knx_l4] _handle_ncd: received ACK with sequence number %d\n", seq_number);
                    }
                    else {
                        DEBUG("[gnrc_knx_l4] _handle_ncd: received ACK for wrong sequence number (got %d, expected %d)\n", seq_number, connection.src_seq);
                    }
                }
                else {
                    DEBUG("[gnrc_knx_l4] _handle_ncd: ACK received from wrong destination\n");
                }
            }
            else {
                DEBUG("[gnrc_knx_l4] _handle_ncd: ACK received but not connected\n");
            }

            break;
        case KNX_TPCI_NCD_NACK:
            if (connection.connected) {
                knx_telegram_get_src_addr(pkt->data, &tmp);

                if (knx_addr_equal(&(connection.dest), &tmp)) {
                    DEBUG("[gnrc_knx_l4] _handle_ncd: NACK received, disconnecting\n");

                    connection.connected = false;
                    _send_control(pkt, 0x81, 0); // T_DISCONNECT_PDU
                }
                else {
                    DEBUG("[gnrc_knx_l4] _handle_ncd: NACK received from wrong destination\n");
                }
            }
            else {
                DEBUG("[gnrc_knx_l4] _handle_ncd: ACK received but not connected\n");
            }

            break;
        default:
            DEBUG("[gnrc_knx_l4] _handle_ncd: unexpected type: %02x\n", ncd);
    }
}

static void _handle_udp(gnrc_pktsnip_t *pkt)
{
    DEBUG("[gnrc_knx_l4] _handle_ndp: handling UDP telegram\n");

    /* no action to take on unnumbered telegrams, so forward it */
    _forward(pkt);
}

static void _handle_ndp(gnrc_pktsnip_t *pkt)
{
    DEBUG("[gnrc_knx_l4] _handle_ndp: handling NDP telegram\n");

    knx_addr_t src;

    /* check if connected */
    if (!connection.connected) {
        DEBUG("[gnrc_knx_l4] _handle_ndp: not connected\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    /* check if destination matches */
    knx_telegram_get_src_addr(pkt->data, &src);

    if (!knx_addr_equal(&src, &(connection.dest))) {
        DEBUG("[gnrc_knx_l4] _handle_ndp: telegram received from unconnected destination\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    /* ensure sequence number is expected */
    uint8_t seq_number = knx_tpci_get_seq_number(pkt->data);

    if (connection.dest_seq != seq_number) {
        DEBUG("[gnrc_knx_l4] _handle_ndp: unexpected sequence number (got %d, expected %d)\n", seq_number, connection.dest_seq);

        _send_control(pkt, 0x81, 0); // T_DISCONNECT_PDU
        connection.connected = false;

        gnrc_pktbuf_release(pkt);
        return;
    }

    /* send payload to receivers */
    _forward(pkt);

    /* send ack */
    connection.timestamp = xtimer_now_usec64();
    connection.dest_seq = (seq_number + 1) % 16;

    DEBUG("[gnrc_knx_l4] _handle_ndp: next sequence number is %d\n", connection.dest_seq);

    _send_control(pkt, 0xc2, seq_number);
}

static void _receive(gnrc_pktsnip_t *pkt)
{
    /* drop telegrams of wrong type */
    if (pkt->type != GNRC_NETTYPE_KNX_L4) {
        DEBUG("[gnrc_knx_l4] _receive: telegram not a transport layer telegram\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    /* drop telegrams that are not telegrams */
    if (!knx_telegram_is(pkt->data, pkt->size)) {
        DEBUG("[gnrc_knx_l4] _receive: invalid or incomplete telegram\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    /* start parsing telegram */
    uint8_t tpci = knx_tpci_get(pkt->data);

    switch (tpci) {
        case KNX_TPCI_UCD:
            _handle_ucd(pkt);
            gnrc_pktbuf_release(pkt);
            break;
        case KNX_TPCI_NCD:
            _handle_ncd(pkt);
            gnrc_pktbuf_release(pkt);
            break;
        case KNX_TPCI_UDP:
            _handle_udp(pkt);
            break;
        case KNX_TPCI_NDP:
            _handle_ndp(pkt);
            break;
        default:
            DEBUG("[gnrc_knx_l4] _receive: unexpected type: %02x\n", tpci);
    }
}

static void _send(gnrc_pktsnip_t *pkt)
{
    knx_addr_t dst;

#ifdef MODULE_GNRC_KNX_L3
    pkt->type = GNRC_NETTYPE_KNX_L3;
#else
    pkt->type = GNRC_NETTYPE_UNDEF;
#endif

    /* if telegram is of type NDP, check if connected and add sequence number */
    uint8_t tpci = knx_tpci_get(pkt->data);

    if (tpci == KNX_TPCI_NDP) {
        /* ensure connected */
        if (!connection.connected) {
            DEBUG("[gnrc_knx_l4] _send: not connected\n");
            gnrc_pktbuf_release(pkt);
            return;
        }

        /* ensure destination is connected */
        knx_telegram_get_dst_addr(pkt->data, &dst);

        if (!knx_addr_equal(&dst, &(connection.dest))) {
            DEBUG("[gnrc_knx_l4] _send: unable to send telegram to unconnected destination\n");
            gnrc_pktbuf_release(pkt);
            return;
        }

        /* set the sequence number */
        knx_tpci_set_seq_number(pkt->data, connection.src_seq);
    }

    /* forward telegram to the network layer */
    if (!gnrc_netapi_dispatch_send(pkt->type, GNRC_NETREG_DEMUX_CTX_ALL, pkt)) {
        DEBUG("[gnrc_knx_l4] _send: cannot send due to missing network layer\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
}

static void *_event_loop(void *args)
{
    (void)args;

    msg_t msg, reply, msg_q[GNRC_KNX_L4_MSG_QUEUE_SIZE];
    gnrc_netreg_entry_t me_reg = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                            sched_active_pid);

    msg_init_queue(msg_q, GNRC_KNX_L4_MSG_QUEUE_SIZE);

    /* register interest in all KNX packets */
    gnrc_netreg_register(GNRC_NETTYPE_KNX_L4, &me_reg);

    /* preinitialize ACK */
    reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
    reply.content.value = (uint32_t)-ENOTSUP;

    /* start event loop */
    while (1) {
        // DEBUG("[gnrc_knx_l4] _event_loop: waiting for incoming message\n");
        msg_receive(&msg);

        switch (msg.type) {
            case GNRC_NETAPI_MSG_TYPE_RCV:
                DEBUG("[gnrc_knx_l4] _event_loop: GNRC_NETAPI_MSG_TYPE_RCV received\n");
                _receive(msg.content.ptr);
                break;
            case GNRC_NETAPI_MSG_TYPE_SND:
                DEBUG("[gnrc_knx_l4] _event_loop: GNRC_NETAPI_MSG_TYPE_SND received\n");
                _send(msg.content.ptr);
                break;
            case GNRC_NETAPI_MSG_TYPE_GET:
            case GNRC_NETAPI_MSG_TYPE_SET:
                DEBUG("[gnrc_knx_l4] _event_loop: reply to unsupported get/set\n");
                msg_reply(&msg, &reply);
                break;
            default:
                DEBUG("[gnrc_knx_l4] _event_loop: received unidentified message\n");
                break;
        }
    }

    return NULL;
}

kernel_pid_t gnrc_knx_l4_init(void)
{
    if (gnrc_knx_l4_pid == KERNEL_PID_UNDEF) {
        gnrc_knx_l4_pid = thread_create(_stack, sizeof(_stack), GNRC_KNX_L4_PRIO,
                                        THREAD_CREATE_STACKTEST,
                                        _event_loop, NULL, "knx_l4");
    }

    return gnrc_knx_l4_pid;
}

knx_connection_t *gnrc_knx_l4_get_connection(void)
{
    return &connection;
}
