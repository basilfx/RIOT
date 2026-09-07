/*
 * SPDX-FileCopyrightText: 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include "byteorder.h"
#include "kernel_defines.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv4.h"
#include "net/gnrc/netif/internal.h"
#include "net/protnum.h"
#include "thread.h"

#define ENABLE_DEBUG 0
#include "debug.h"

static char _stack[GNRC_IPV4_STACK_SIZE + DEBUG_EXTRA_STACKSIZE];
static msg_t _msg_q[GNRC_IPV4_MSG_QUEUE_SIZE];

kernel_pid_t gnrc_ipv4_pid = KERNEL_PID_UNDEF;

static void _receive(gnrc_pktsnip_t *pkt);
static void _send(gnrc_pktsnip_t *pkt, bool prep_hdr);
static void *_event_loop(void *args);

kernel_pid_t gnrc_ipv4_init(void)
{
    if (gnrc_ipv4_pid == KERNEL_PID_UNDEF) {
        gnrc_ipv4_pid = thread_create(_stack, sizeof(_stack), GNRC_IPV4_PRIO,
                                      0, _event_loop, NULL, "ipv4");
    }
    return gnrc_ipv4_pid;
}

ipv4_hdr_t *gnrc_ipv4_get_header(gnrc_pktsnip_t *pkt)
{
    gnrc_pktsnip_t *tmp = gnrc_pktsnip_search_type(pkt, GNRC_NETTYPE_IPV4);

    if (tmp == NULL) {
        return NULL;
    }
    assert(tmp->data != NULL);
    assert(tmp->size >= sizeof(ipv4_hdr_t));
    return (ipv4_hdr_t *)tmp->data;
}

static void _demux(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt, uint8_t protocol)
{
    (void)netif;
    pkt->type = gnrc_nettype_from_protnum(protocol);
    if (gnrc_netapi_dispatch_receive(pkt->type, GNRC_NETREG_DEMUX_CTX_ALL,
                                     pkt) == 0) {
        DEBUG("ipv4: unable to forward packet as no one is interested in it\n");
        gnrc_pktbuf_release(pkt);
    }
}

/**
 * @brief   Checks whether @p dst is the limited broadcast address
 *          (255.255.255.255) or the directed broadcast address of one of
 *          @p netif's configured subnets.
 */
static bool _is_broadcast(gnrc_netif_t *netif, const ipv4_addr_t *dst)
{
    if (dst->u32.u32 == 0xffffffffU) {
        return true;
    }
    if (netif == NULL) {
        return false;
    }
    for (unsigned i = 0; i < CONFIG_GNRC_NETIF_IPV4_ADDRS_NUMOF; i++) {
        if (netif->ipv4.addrs_flags[i] == GNRC_NETIF_IPV4_ADDRS_FLAGS_STATE_UNUSED) {
            continue;
        }
        if (ipv4_addr_match_prefix(&netif->ipv4.addrs[i], dst,
                                   netif->ipv4.prefix_lens[i])) {
            ipv4_addr_t mask = ipv4_addr_netmask_from_prefix(netif->ipv4.prefix_lens[i]);

            if ((dst->u32.u32 & ~mask.u32.u32) == (uint32_t)(~mask.u32.u32)) {
                return true;
            }
        }
    }
    return false;
}

/* functions for receiving */
static void _receive_ipv4(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    gnrc_pktsnip_t *ipv4;
    ipv4_hdr_t *hdr;
    uint16_t ihl_bytes;
    uint16_t tot_len;
    uint16_t payload_len;
    uint8_t protocol;

    if ((pkt->data == NULL) || (pkt->size < sizeof(ipv4_hdr_t)) ||
        (ipv4_hdr_get_version((ipv4_hdr_t *)pkt->data) != 4)) {
        DEBUG("ipv4: received packet was not IPv4, dropping\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    ihl_bytes = ipv4_hdr_get_ihl((ipv4_hdr_t *)pkt->data);
    if ((ihl_bytes < sizeof(ipv4_hdr_t)) || (pkt->size < ihl_bytes)) {
        DEBUG("ipv4: invalid IHL, dropping\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    ipv4 = gnrc_pktbuf_start_write(pkt);
    if (ipv4 == NULL) {
        DEBUG("ipv4: unable to get write access to packet, dropping\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    pkt = ipv4;

    ipv4 = gnrc_pktbuf_mark(pkt, ihl_bytes, GNRC_NETTYPE_IPV4);
    pkt->type = GNRC_NETTYPE_UNDEF; /* snip is no longer the IPv4 header */

    if (ipv4 == NULL) {
        DEBUG("ipv4: error marking IPv4 header, dropping\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    hdr = ipv4->data;
    tot_len = byteorder_ntohs(hdr->tl);

    if (tot_len < ihl_bytes) {
        DEBUG("ipv4: total length shorter than header, dropping\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    payload_len = tot_len - ihl_bytes;

    /* if available, remove any padding that was added by lower layers to
     * fulfill their minimum size requirements (e.g. ethernet) */
    if ((ipv4 != pkt) && (payload_len < pkt->size)) {
        gnrc_pktbuf_realloc_data(pkt, payload_len);
    }
    else if (payload_len > (gnrc_pkt_len_upto(pkt, GNRC_NETTYPE_IPV4) - ihl_bytes)) {
        DEBUG("ipv4: invalid total length: %u, actual: %u, dropping packet\n",
              (unsigned)tot_len,
              (unsigned)(gnrc_pkt_len_upto(pkt, GNRC_NETTYPE_IPV4)));
        gnrc_pktbuf_release_error(pkt, EINVAL);
        return;
    }

    if (!(_is_broadcast(netif, &hdr->dst) ||
         (ipv4_addr_is_multicast(&hdr->dst) &&
          (netif != NULL) && (gnrc_netif_ipv4_group_idx(netif, &hdr->dst) >= 0)) ||
         (gnrc_netif_get_by_ipv4_addr(&hdr->dst) != NULL))) {
        /* GNRC IPv4 is host-only: packets not addressed to us are dropped,
         * never forwarded */
        DEBUG("ipv4: packet destination not this host, dropping\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    protocol = hdr->protocol;
    _demux(netif, pkt, protocol);
}

static void _receive(gnrc_pktsnip_t *pkt)
{
    gnrc_pktsnip_t *netif_snip;
    gnrc_netif_t *netif = NULL;

    assert(pkt != NULL);
    netif_snip = gnrc_pktsnip_search_type(pkt, GNRC_NETTYPE_NETIF);
    if (netif_snip != NULL) {
        netif = gnrc_netif_hdr_get_netif(netif_snip->data);
    }
    if (pkt->type == GNRC_NETTYPE_ARP) {
        if (netif == NULL) {
            DEBUG("ipv4: ARP packet without interface, dropping\n");
            gnrc_pktbuf_release(pkt);
            return;
        }
        gnrc_ipv4_arp_handle_pkt(netif, pkt);
        return;
    }
    _receive_ipv4(netif, pkt);
}

/* functions for sending */
static int _fill_ipv4_hdr(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    ipv4_hdr_t *hdr = pkt->data;
    uint16_t ihl_bytes = ipv4_hdr_get_ihl(hdr);
    gnrc_pktsnip_t *payload, *prev;
    int res;

    hdr->tl = byteorder_htons(ihl_bytes + gnrc_pkt_len(pkt->next));

    if (hdr->protocol == PROTNUM_RESERVED) {
        hdr->protocol = (pkt->next != NULL) ?
                       gnrc_nettype_to_protnum(pkt->next->type) : PROTNUM_RESERVED;
    }
    if (hdr->ttl == 0) {
        hdr->ttl = (netif != NULL) ? netif->cur_hl : CONFIG_GNRC_NETIF_DEFAULT_HL;
    }
    if (hdr->src.u32.u32 == 0) {
        gnrc_netif_t *src_netif = netif;

        if (src_netif == NULL) {
            src_netif = gnrc_netif_get_by_ipv4_prefix(&hdr->dst);
        }
        if (src_netif != NULL) {
            int idx = gnrc_netif_ipv4_addr_match(src_netif, &hdr->dst);

            if (idx < 0) {
                for (idx = 0; idx < CONFIG_GNRC_NETIF_IPV4_ADDRS_NUMOF; idx++) {
                    if (src_netif->ipv4.addrs_flags[idx] !=
                        GNRC_NETIF_IPV4_ADDRS_FLAGS_STATE_UNUSED) {
                        break;
                    }
                }
                if (idx >= CONFIG_GNRC_NETIF_IPV4_ADDRS_NUMOF) {
                    idx = -1;
                }
            }
            if (idx >= 0) {
                hdr->src = src_netif->ipv4.addrs[idx];
            }
        }
    }

    /* write protect up to payload to calculate checksum */
    payload = pkt;
    prev = pkt;
    while ((payload->type == GNRC_NETTYPE_IPV4) && (payload->next != NULL)) {
        if ((payload = gnrc_pktbuf_start_write(payload->next)) == NULL) {
            DEBUG("ipv4: unable to get write access to payload header\n");
            return -ENOMEM;
        }
        prev->next = payload;
        prev = payload;
    }
    res = gnrc_netreg_calc_csum(payload, pkt);
    if ((res < 0) && (res != -ENOENT)) {
        DEBUG("ipv4: checksum calculation failed\n");
        return res;
    }

    hdr->csum = byteorder_htons(0);
    hdr->csum = ipv4_hdr_csum(hdr);
    return 0;
}

static void _send_to_iface(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt,
                           const uint8_t *l2addr, uint8_t l2addr_len,
                           uint8_t extra_flags)
{
    gnrc_pktsnip_t *netif_hdr = gnrc_netif_hdr_build(NULL, 0, l2addr, l2addr_len);
    gnrc_netif_hdr_t *hdr;

    if (netif_hdr == NULL) {
        DEBUG("ipv4: unable to allocate interface header, dropping packet\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    hdr = netif_hdr->data;
    hdr->flags |= extra_flags;
    gnrc_netif_hdr_set_netif(hdr, netif);
    pkt = gnrc_pkt_prepend(pkt, netif_hdr);
    if (gnrc_netif_send(netif, pkt) < 1) {
        DEBUG("ipv4: unable to send packet\n");
        gnrc_pktbuf_release(pkt);
    }
}

static void _send_multicast(gnrc_pktsnip_t *pkt, gnrc_netif_t *netif,
                            uint8_t netif_hdr_flags)
{
    if (netif == NULL) {
        netif = gnrc_netif_iter(NULL);
        if (netif == NULL) {
            DEBUG("ipv4: no interface registered, dropping packet\n");
            gnrc_pktbuf_release(pkt);
            return;
        }
    }
    _send_to_iface(netif, pkt, NULL, 0,
                   netif_hdr_flags | GNRC_NETIF_HDR_FLAGS_MULTICAST);
}

static void _send_broadcast(gnrc_pktsnip_t *pkt, gnrc_netif_t *netif,
                            uint8_t netif_hdr_flags)
{
    if (netif == NULL) {
        netif = gnrc_netif_iter(NULL);
        if (netif == NULL) {
            DEBUG("ipv4: no interface registered, dropping packet\n");
            gnrc_pktbuf_release(pkt);
            return;
        }
    }
    _send_to_iface(netif, pkt, NULL, 0,
                   netif_hdr_flags | GNRC_NETIF_HDR_FLAGS_BROADCAST);
}

static void _send_to_self(gnrc_pktsnip_t *pkt)
{
    if (gnrc_pktbuf_merge(pkt) != 0) {
        DEBUG("ipv4: error looping packet to sender\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    if (gnrc_netapi_dispatch_receive(GNRC_NETTYPE_IPV4, GNRC_NETREG_DEMUX_CTX_ALL,
                                     pkt) == 0) {
        DEBUG("ipv4: unable to deliver looped back packet\n");
        gnrc_pktbuf_release(pkt);
    }
}

static void _send_unicast(gnrc_pktsnip_t *pkt, gnrc_netif_t *netif)
{
    ipv4_hdr_t *hdr = pkt->data;
    ipv4_addr_t next_hop;

    if (netif == NULL) {
        netif = gnrc_netif_get_by_ipv4_prefix(&hdr->dst);
    }
    if ((netif != NULL) && (gnrc_netif_ipv4_addr_match(netif, &hdr->dst) >= 0)) {
        /* destination is on a directly connected subnet: resolve it directly */
        next_hop = hdr->dst;
    }
    else {
        gnrc_ipv4_ft_t fte;

        if (gnrc_ipv4_ft_get(&hdr->dst, &fte) < 0) {
            DEBUG("ipv4: no route to host, dropping packet\n");
            gnrc_pktbuf_release_error(pkt, EHOSTUNREACH);
            return;
        }
        next_hop = fte.next_hop;
        netif = gnrc_netif_get_by_pid(fte.iface);
    }
    if (netif == NULL) {
        DEBUG("ipv4: no interface to send over, dropping packet\n");
        gnrc_pktbuf_release_error(pkt, ENETUNREACH);
        return;
    }
    if (gnrc_pkt_len(pkt) > netif->ipv4.mtu) {
        /* fragmentation is not yet implemented */
        DEBUG("ipv4: packet too big, dropping\n");
        gnrc_pktbuf_release_error(pkt, EMSGSIZE);
        return;
    }
    if (netif->l2addr_len == 0) {
        /* point-to-point link (e.g. SLIP): no link layer address to
         * resolve */
        _send_to_iface(netif, pkt, NULL, 0, 0);
        return;
    }
    gnrc_ipv4_arp_request(netif, &next_hop, pkt);
}

static void _send(gnrc_pktsnip_t *pkt, bool prep_hdr)
{
    gnrc_netif_t *netif = NULL;
    gnrc_pktsnip_t *tmp_pkt;
    ipv4_hdr_t *hdr;
    uint8_t netif_hdr_flags = 0U;

    if (pkt->type == GNRC_NETTYPE_NETIF) {
        const gnrc_netif_hdr_t *netif_hdr = pkt->data;

        netif = gnrc_netif_hdr_get_netif(pkt->data);
        netif_hdr_flags = netif_hdr->flags &
                         ~(GNRC_NETIF_HDR_FLAGS_BROADCAST |
                           GNRC_NETIF_HDR_FLAGS_MULTICAST);
        tmp_pkt = gnrc_pktbuf_start_write(pkt);
        if (tmp_pkt == NULL) {
            DEBUG("ipv4: unable to get write access to netif header, dropping packet\n");
            gnrc_pktbuf_release(pkt);
            return;
        }
        pkt = gnrc_pktbuf_remove_snip(tmp_pkt, tmp_pkt);
    }
    if (pkt->type != GNRC_NETTYPE_IPV4) {
        DEBUG("ipv4: unexpected packet type, dropping\n");
        gnrc_pktbuf_release_error(pkt, EINVAL);
        return;
    }

    tmp_pkt = gnrc_pktbuf_start_write(pkt);
    if (tmp_pkt == NULL) {
        DEBUG("ipv4: unable to get write access to IPv4 header, dropping packet\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    pkt = tmp_pkt;
    hdr = pkt->data;

    if (prep_hdr && (_fill_ipv4_hdr(netif, pkt) < 0)) {
        gnrc_pktbuf_release(pkt);
        return;
    }

    if (ipv4_addr_is_multicast(&hdr->dst)) {
        _send_multicast(pkt, netif, netif_hdr_flags);
    }
    else if (_is_broadcast(netif, &hdr->dst)) {
        _send_broadcast(pkt, netif, netif_hdr_flags);
    }
    else if (gnrc_netif_get_by_ipv4_addr(&hdr->dst) != NULL) {
        _send_to_self(pkt);
    }
    else {
        _send_unicast(pkt, netif);
    }
}

static void *_event_loop(void *args)
{
    msg_t msg, reply;
    gnrc_netreg_entry_t me_ipv4_reg = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                                 thread_getpid());
    gnrc_netreg_entry_t me_arp_reg = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                                thread_getpid());

    (void)args;
    msg_init_queue(_msg_q, GNRC_IPV4_MSG_QUEUE_SIZE);
    gnrc_ipv4_arp_init();

    gnrc_netreg_register(GNRC_NETTYPE_IPV4, &me_ipv4_reg);
    gnrc_netreg_register(GNRC_NETTYPE_ARP, &me_arp_reg);

    reply.type = GNRC_NETAPI_MSG_TYPE_ACK;

    while (1) {
        DEBUG("ipv4: waiting for incoming message\n");
        msg_receive(&msg);

        switch (msg.type) {
            case GNRC_NETAPI_MSG_TYPE_RCV:
                DEBUG("ipv4: GNRC_NETAPI_MSG_TYPE_RCV received\n");
                _receive(msg.content.ptr);
                break;
            case GNRC_NETAPI_MSG_TYPE_SND:
                DEBUG("ipv4: GNRC_NETAPI_MSG_TYPE_SND received\n");
                _send(msg.content.ptr, true);
                break;
            case GNRC_NETAPI_MSG_TYPE_GET:
            case GNRC_NETAPI_MSG_TYPE_SET:
                DEBUG("ipv4: reply to unsupported get/set\n");
                reply.content.value = -ENOTSUP;
                msg_reply(&msg, &reply);
                break;
            case GNRC_IPV4_ARP_TIMEOUT:
                gnrc_ipv4_arp_handle_timeout(msg.content.ptr);
                break;
            default:
                break;
        }
    }

    return NULL;
}

/** @} */
