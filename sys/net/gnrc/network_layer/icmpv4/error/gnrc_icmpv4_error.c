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

#include "macros/utils.h"
#include "net/gnrc/icmpv4.h"
#include "net/gnrc/icmpv4/error.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/pktbuf.h"
#include "net/ipv4/addr.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/* dst_unr and time_exc are both a 4-byte "unused" value after the ICMPv4
 * header; param_prob overlays a 1-byte pointer plus 3 bytes unused at the
 * same offset. All three share the same overall size. */
#define ICMPV4_ERROR_SZ (sizeof(icmpv4_error_dst_unr_t))
#define ICMPV4_ERROR_SET_VALUE(data, value) \
    ((icmpv4_error_dst_unr_t *)(data))->unused = byteorder_htonl(value)

/**
 * @brief   Get packet fit.
 *
 * Gets the minimum size for an ICMPv4 error message packet, based on the
 * invoking packet's size and the interface the invoking packet came over.
 *
 * @param[in] orig_pkt  The invoking packet
 *
 * @return  The supposed size of the ICMPv4 error message.
 */
static size_t _fit(const gnrc_pktsnip_t *orig_pkt)
{
    /* discarding const qualifier is safe here */
    gnrc_pktsnip_t *netif_hdr = gnrc_pktsnip_search_type(
            (gnrc_pktsnip_t *)orig_pkt, GNRC_NETTYPE_NETIF
        );
    /* orig_pkt already contains its own (marked) IPv4 header snip, so
     * unlike the outgoing datagram's MTU clamp below, this must not add
     * another sizeof(ipv4_hdr_t) on top */
    size_t pkt_len = gnrc_pkt_len(orig_pkt) + ICMPV4_ERROR_SZ;

    if (netif_hdr) {
        gnrc_netif_t *netif = gnrc_netif_hdr_get_netif(netif_hdr->data);
        assert(netif != NULL);

        pkt_len -= netif_hdr->size;
        DEBUG("gnrc_icmpv4_error: fitting to MTU of iface %u (%u)\n",
              netif->pid, netif->ipv4.mtu);
        return MIN(pkt_len, netif->ipv4.mtu - sizeof(ipv4_hdr_t));
    }
    else {
        /* packet does not have a netif header (most likely because it did not
         * came from remote) => just assume pkt_len as ideal */
        DEBUG("gnrc_icmpv4_error: copying whole packet\n");
        return pkt_len;
    }
}

static inline bool _in_orig_pkt(const gnrc_pktsnip_t *orig_pkt)
{
    return (orig_pkt != NULL) && (orig_pkt->type != GNRC_NETTYPE_NETIF);
}

static size_t _copy_rcv_snip(gnrc_pktsnip_t *pkt,
                             const gnrc_pktsnip_t *orig_snip)
{
    /* always skip ICMPv4 error header */
    size_t offset = ICMPV4_ERROR_SZ;
    const gnrc_pktsnip_t *ptr = orig_snip;

    while (_in_orig_pkt(ptr->next)) {
        offset += ptr->next->size;
        ptr = ptr->next;
    }

    if (offset < pkt->size) {
        uint8_t *data = pkt->data;

        memcpy(data + offset, orig_snip->data,
               MIN(pkt->size - offset, orig_snip->size));
    }
    return offset;
}

static inline bool _check_send_order(const gnrc_pktsnip_t *pkt)
{
    /* sent packets in IPv4 start either with netif header or
     * with IPv4 header (but then the NETIF header doesn't follow) */
    return (pkt->type == GNRC_NETTYPE_NETIF) ||
           ((pkt->type == GNRC_NETTYPE_IPV4) &&
            ((pkt->next == NULL) || (pkt->next->type != GNRC_NETTYPE_NETIF)));
}

/* Build a generic error message */
static gnrc_pktsnip_t *_icmpv4_error_build(uint8_t type, uint8_t code,
                                           const gnrc_pktsnip_t *orig_pkt,
                                           uint32_t value)
{
    gnrc_pktsnip_t *pkt = gnrc_icmpv4_build(NULL, type, code, _fit(orig_pkt));

    /* copy as much of the originating packet into error message as fits the
     * message's size */
    if (pkt != NULL) {
        ICMPV4_ERROR_SET_VALUE(pkt->data, value);
        if (_check_send_order(orig_pkt)) {
            const gnrc_pktsnip_t *ptr = (orig_pkt->type == GNRC_NETTYPE_NETIF)
                                      ? orig_pkt->next
                                      : orig_pkt;
            size_t offset = ICMPV4_ERROR_SZ;

            while ((ptr != NULL) && (offset < pkt->size)) {
                uint8_t *data = pkt->data;

                memcpy(data + offset, ptr->data, MIN(pkt->size - offset,
                                                     ptr->size));
                offset += ptr->size;
                ptr = ptr->next;
            }
        }
        else {
            while (_in_orig_pkt(orig_pkt)) {
                _copy_rcv_snip(pkt, orig_pkt);
                orig_pkt = orig_pkt->next;
            }
        }
    }

    return pkt;
}

static inline gnrc_pktsnip_t *_dst_unr_build(uint8_t code,
                                             const gnrc_pktsnip_t *orig_pkt)
{
    return _icmpv4_error_build(ICMPV4_DST_UNR, code, orig_pkt, 0);
}

static inline gnrc_pktsnip_t *_time_exc_build(uint8_t code,
                                              const gnrc_pktsnip_t *orig_pkt)
{
    return _icmpv4_error_build(ICMPV4_TIME_EXC, code, orig_pkt, 0);
}

static inline bool _in_range(uint8_t *ptr, uint8_t *start, size_t sz)
{
    return (ptr >= start) && (ptr < (start + sz));
}

static gnrc_pktsnip_t *_param_prob_build(void *ptr,
                                         const gnrc_pktsnip_t *orig_pkt)
{
    gnrc_pktsnip_t *pkt = gnrc_icmpv4_build(NULL, ICMPV4_PARAM_PROB,
                                            ICMPV4_ERROR_PARAM_PROB_PTR,
                                            _fit(orig_pkt));

    /* copy as much of the originating packet into error message and
     * determine relative *ptr* offset */
    if (pkt != NULL) {
        icmpv4_error_param_prob_t *hdr = pkt->data;
        uint32_t ptr_offset = 0U;

        while (_in_orig_pkt(orig_pkt)) {
            /* copy as long as it fits into packet; parameter problem can only
             * come from received packets */
            size_t offset = _copy_rcv_snip(pkt, orig_pkt);

            if (_in_range(ptr, orig_pkt->data, orig_pkt->size)) {
                ptr_offset = (uint32_t)(((uint8_t *)ptr) -
                                        ((uint8_t *)orig_pkt->data) +
                                        (offset - ICMPV4_ERROR_SZ));
            }
            orig_pkt = orig_pkt->next;
        }

        /* set "pointer" field to relative pointer offset; unlike ICMPv6's
         * 32-bit pointer, RFC 792 only reserves a single byte for it, which
         * is sufficient since an IPv4 header is at most 60 bytes long */
        hdr->ptr = (uint8_t)ptr_offset;
    }

    return pkt;
}

static void _send(gnrc_pktsnip_t *pkt, const gnrc_pktsnip_t *orig_pkt,
                  gnrc_pktsnip_t *ipv4)
{
    if (pkt != NULL) {
        /* discarding const qualifier is safe here */
        gnrc_pktsnip_t *netif = gnrc_pktsnip_search_type((gnrc_pktsnip_t *)orig_pkt,
                                                         GNRC_NETTYPE_NETIF);
        assert(ipv4 != NULL);
        ipv4_hdr_t *ipv4_hdr = ipv4->data;
        /* overwrite ipv4 parameter pointer ... we don't need it after this */
        ipv4 = gnrc_ipv4_hdr_build(pkt, NULL, &ipv4_hdr->src);
        if (ipv4 == NULL) {
            DEBUG("gnrc_icmpv4_error: No space in packet buffer left\n");
            gnrc_pktbuf_release(pkt);
            return;
        }
        pkt = ipv4;
        if (netif) {
            /* copy interface from original netif header to assure packet
             * goes out where it came from */
            gnrc_netif_t *iface = gnrc_netif_hdr_get_netif(netif->data);

            netif = gnrc_netif_hdr_build(NULL, 0, NULL, 0);
            if (netif == NULL) {
                DEBUG("gnrc_icmpv4_error: No space in packet buffer left\n");
                gnrc_pktbuf_release(pkt);
                return;
            }
            gnrc_netif_hdr_set_netif(netif->data, iface);
            pkt = gnrc_pkt_prepend(pkt, netif);
        }
        if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_IPV4,
                                       GNRC_NETREG_DEMUX_CTX_ALL,
                                       pkt)) {
            DEBUG("gnrc_icmpv4_error: No send handler found.\n");
            gnrc_pktbuf_release(pkt);
        }
    }
    else {
        DEBUG("gnrc_icmpv4_error: No space in packet buffer left\n");
    }
}

static gnrc_pktsnip_t *_check_ipv4_hdr(const gnrc_pktsnip_t *orig_pkt)
{
    /* discarding const qualifier is safe here */
    gnrc_pktsnip_t *ipv4 = gnrc_pktsnip_search_type((gnrc_pktsnip_t *)orig_pkt,
                                                    GNRC_NETTYPE_IPV4);
    assert(ipv4 != NULL);
    const ipv4_hdr_t *ipv4_hdr = ipv4->data;

    /* RFC 1122, section 3.2.2: an ICMP error message must not be sent as
     * the result of receiving a datagram whose source address does not
     * define a single host (i.e. unspecified, loopback, broadcast or
     * multicast), nor as the result of receiving a datagram destined to a
     * broadcast or multicast address */
    if (ipv4_addr_is_unspecified(&ipv4_hdr->src) ||
        ipv4_addr_is_multicast(&ipv4_hdr->src) ||
        ipv4_addr_equal(&ipv4_hdr->src, &ipv4_addr_broadcast) ||
        ipv4_addr_is_multicast(&ipv4_hdr->dst) ||
        ipv4_addr_equal(&ipv4_hdr->dst, &ipv4_addr_broadcast)) {
        ipv4 = NULL;
    }
    return ipv4;
}

void gnrc_icmpv4_error_dst_unr_send(uint8_t code, const gnrc_pktsnip_t *orig_pkt)
{
    gnrc_pktsnip_t *ipv4 = _check_ipv4_hdr(orig_pkt);

    if (ipv4 != NULL) {
        gnrc_pktsnip_t *pkt = _dst_unr_build(code, orig_pkt);

        DEBUG("gnrc_icmpv4_error: trying to send destination unreachable error\n");
        _send(pkt, orig_pkt, ipv4);
    }
}

void gnrc_icmpv4_error_time_exc_send(uint8_t code,
                                     const gnrc_pktsnip_t *orig_pkt)
{
    gnrc_pktsnip_t *ipv4 = _check_ipv4_hdr(orig_pkt);

    if (ipv4 != NULL) {
        gnrc_pktsnip_t *pkt = _time_exc_build(code, orig_pkt);

        DEBUG("gnrc_icmpv4_error: trying to send time exceeded error\n");
        _send(pkt, orig_pkt, ipv4);
    }
}

void gnrc_icmpv4_error_param_prob_send(void *ptr,
                                       const gnrc_pktsnip_t *orig_pkt)
{
    gnrc_pktsnip_t *ipv4 = _check_ipv4_hdr(orig_pkt);

    if (ipv4 != NULL) {
        gnrc_pktsnip_t *pkt = _param_prob_build(ptr, orig_pkt);

        DEBUG("gnrc_icmpv4_error: trying to send parameter problem error\n");
        _send(pkt, orig_pkt, ipv4);
    }
}

/** @} */
