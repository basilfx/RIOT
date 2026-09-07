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

#include <errno.h>
#include <inttypes.h>
#include <string.h>

#include "net/gnrc.h"
#include "net/gnrc/icmpv4.h"
#include "net/gnrc/icmpv4/echo.h"
#include "net/gnrc/ipv4.h"
#include "unaligned.h"
#include "ztimer.h"

#define ENABLE_DEBUG 0
#include "debug.h"

gnrc_pktsnip_t *gnrc_icmpv4_echo_build(uint8_t type, uint16_t id, uint16_t seq,
                                      const void *data, size_t data_len)
{
    gnrc_pktsnip_t *pkt;
    icmpv4_echo_t *echo;

    if ((pkt = gnrc_icmpv4_build(NULL, type, 0, data_len + sizeof(icmpv4_echo_t))) == NULL) {
        return NULL;
    }

    DEBUG("icmpv4_echo: building echo message with type=%" PRIu8 " id=%" PRIu16
          ", seq=%" PRIu16 "\n", type, id, seq);
    echo = (icmpv4_echo_t *)pkt->data;
    echo->id = byteorder_htons(id);
    echo->seq = byteorder_htons(seq);

    if (data != NULL) {
        memcpy(echo + 1, data, data_len);
    }

    return pkt;
}

void gnrc_icmpv4_echo_req_handle(gnrc_netif_t *netif, ipv4_hdr_t *ipv4_hdr,
                                 icmpv4_echo_t *echo, uint16_t len)
{
    uint8_t *payload = ((uint8_t *)echo) + sizeof(icmpv4_echo_t);
    gnrc_pktsnip_t *hdr, *pkt;

    if ((echo == NULL) || (len < sizeof(icmpv4_echo_t))) {
        DEBUG("icmpv4_echo: echo was NULL or len (%" PRIu16
              ") was < sizeof(icmpv4_echo_t)\n", len);
        return;
    }

    pkt = gnrc_icmpv4_echo_build(ICMPV4_ECHO_REP, byteorder_ntohs(echo->id),
                                byteorder_ntohs(echo->seq), payload,
                                len - sizeof(icmpv4_echo_t));

    if (pkt == NULL) {
        DEBUG("icmpv4_echo: no space left in packet buffer\n");
        return;
    }

    if (ipv4_addr_is_multicast(&ipv4_hdr->dst) ||
        (ipv4_hdr->dst.u32.u32 == 0xffffffffU)) {
        /* replying from a multicast or (limited) broadcast destination
         * makes no sense: let gnrc_ipv4 pick one of our own addresses */
        hdr = gnrc_ipv4_hdr_build(pkt, NULL, &ipv4_hdr->src);
    }
    else {
        hdr = gnrc_ipv4_hdr_build(pkt, &ipv4_hdr->dst, &ipv4_hdr->src);
    }

    if (hdr == NULL) {
        DEBUG("icmpv4_echo: no space left in packet buffer\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    pkt = hdr;
    hdr = gnrc_netif_hdr_build(NULL, 0, NULL, 0);

    if (hdr == NULL) {
        DEBUG("icmpv4_echo: no space left in packet buffer\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    gnrc_netif_hdr_set_netif(hdr->data, netif);

    pkt = gnrc_pkt_prepend(pkt, hdr);

    if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_IPV4, GNRC_NETREG_DEMUX_CTX_ALL,
                                   pkt)) {
        DEBUG("icmpv4_echo: no receivers for IPv4 packets\n");
        gnrc_pktbuf_release(pkt);
    }
}

static void _fill_payload(uint8_t *buf, size_t len, uint32_t now)
{
    uint8_t i = 0;

    if (len >= sizeof(uint32_t)) {
        memcpy(buf, &now, sizeof(now));
        len -= sizeof(now);
        buf += sizeof(now);
    }

    while (len--) {
        *buf++ = i++;
    }
}

static void _check_payload(const void *buf, size_t len, uint32_t now,
                           uint32_t *triptime, int *corrupt)
{
    uint8_t i = 0;
    const uint8_t *data = buf;

    if (len >= sizeof(uint32_t)) {
        *triptime = now - unaligned_get_u32(buf);
        len  -= sizeof(uint32_t);
        data += sizeof(uint32_t);
    }

    while (len--) {
        if (*data++ != i++) {
            *corrupt = data - (uint8_t *)buf - 1;
            break;
        }
    }
}

int gnrc_icmpv4_echo_send(const ipv4_addr_t *addr, uint16_t id, uint16_t seq,
                          uint8_t ttl, size_t len)
{
    int res = 0;
    gnrc_pktsnip_t *pkt, *tmp;
    ipv4_hdr_t *ipv4;
    uint8_t *databuf;

    /* max IPv4 payload 65535 minus IPv4 and ICMPv4 header */
    if (len > (UINT16_MAX - sizeof(ipv4_hdr_t) - sizeof(icmpv4_hdr_t))) {
        DEBUG("error: wrong icmpv4 packet length\n");
        return -EINVAL;
    }

    pkt = gnrc_icmpv4_echo_build(ICMPV4_ECHO_REQ, id, seq, NULL, len);
    if (pkt == NULL) {
        DEBUG("error: packet buffer full\n");
        return -ENOMEM;
    }

    databuf = (uint8_t *)(pkt->data) + sizeof(icmpv4_echo_t);
    tmp = gnrc_ipv4_hdr_build(pkt, NULL, addr);
    if (tmp == NULL) {
        DEBUG("error: packet buffer full\n");
        res = -ENOMEM;
        goto error_exit;
    }
    pkt = tmp;
    ipv4 = pkt->data;
    /* if ttl is unset (i.e. 0) gnrc_ipv4 will select the interface default */
    ipv4->ttl = ttl;

    /* add TX timestamp & test data */
    _fill_payload(databuf, len, ztimer_now(ZTIMER_USEC));

    res = !gnrc_netapi_dispatch_send(GNRC_NETTYPE_IPV4,
                                     GNRC_NETREG_DEMUX_CTX_ALL,
                                     pkt);
    if (res) {
        DEBUG("error: unable to send ICMPv4 echo request\n");
        res = -EBADF;
    }

error_exit:
    if (res) {
        gnrc_pktbuf_release(pkt);
    }
    return res;
}

int gnrc_icmpv4_echo_rsp_handle(gnrc_pktsnip_t *pkt, size_t len,
                                gnrc_icmpv4_echo_rsp_handle_cb_t cb, void *ctx)
{
    gnrc_pktsnip_t *ipv4, *icmpv4;
    icmpv4_echo_t *icmpv4_hdr;
    uint32_t now = ztimer_now(ZTIMER_USEC);
    uint32_t triptime = 0;
    int corrupted = -1;

    ipv4 = gnrc_pktsnip_search_type(pkt, GNRC_NETTYPE_IPV4);
    icmpv4 = gnrc_pktsnip_search_type(pkt, GNRC_NETTYPE_ICMPV4);
    if ((ipv4 == NULL) || (icmpv4 == NULL)) {
        DEBUG("icmpv4_echo: no IPv4 or ICMPv4 header found in reply\n");
        return -EINVAL;
    }

    icmpv4_hdr = icmpv4->data;
    _check_payload(icmpv4_hdr + 1, len, now, &triptime, &corrupted);

    return cb(pkt, corrupted, triptime, ctx);
}

/** @} */
