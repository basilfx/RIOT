/*
 * SPDX-FileCopyrightText: 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     net_gnrc_icmpv4
 * @{
 *
 * @file
 */

#include <assert.h>
#include <errno.h>

#include "byteorder.h"
#include "net/gnrc.h"
#include "net/gnrc/icmpv4.h"
#include "net/gnrc/icmpv4/echo.h"
#include "net/ipv4/hdr.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief   Calculates the ICMPv4 checksum.
 *
 * @note    Unlike ICMPv6 (RFC 4443, section 2.3), ICMPv4 does not fold an
 *          IPv4 pseudo-header into the checksum (RFC 792): the checksum
 *          covers only the ICMPv4 message itself.
 */
static inline uint16_t _calc_csum(gnrc_pktsnip_t *hdr, gnrc_pktsnip_t *payload)
{
    uint16_t csum = 0;
    uint16_t len = (uint16_t)hdr->size;

    while (payload && (payload != hdr)) {
        csum = inet_csum_slice(csum, payload->data, payload->size, len);
        len += (uint16_t)payload->size;
        payload = payload->next;
    }

    csum = inet_csum(csum, hdr->data, hdr->size);

    return ~csum;
}

void gnrc_icmpv4_demux(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    gnrc_pktsnip_t *icmpv4, *ipv4;
    icmpv4_hdr_t *hdr;

    icmpv4 = gnrc_pktsnip_search_type(pkt, GNRC_NETTYPE_ICMPV4);
    assert(icmpv4 != NULL);
    ipv4 = gnrc_pktsnip_search_type(icmpv4, GNRC_NETTYPE_IPV4);
    assert(ipv4 != NULL);
    /* only used below when MODULE_GNRC_ICMPV4_ECHO is built in; otherwise
     * it's read solely by the assert above */
    (void)ipv4;

    if (icmpv4->size < sizeof(icmpv4_hdr_t)) {
        DEBUG("icmpv4: packet too short\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    hdr = (icmpv4_hdr_t *)icmpv4->data;

    if (_calc_csum(icmpv4, pkt)) {
        DEBUG("icmpv4: wrong checksum\n");
        gnrc_pktbuf_release(pkt);
        return;
    }

    switch (hdr->type) {
#ifdef MODULE_GNRC_ICMPV4_ECHO
        case ICMPV4_ECHO_REQ:
            DEBUG("icmpv4: handle echo request\n");
            gnrc_icmpv4_echo_req_handle(netif, (ipv4_hdr_t *)ipv4->data,
                                        (icmpv4_echo_t *)hdr, icmpv4->size);
            break;
#endif
        default:
            DEBUG("icmpv4: unknown type field %u\n", hdr->type);
            (void)netif;
            break;
    }

    /* ICMPv4-all is dispatched in gnrc_ipv4.c, only subtypes here */
    if (!gnrc_netapi_dispatch_receive(GNRC_NETTYPE_ICMPV4, hdr->type, pkt)) {
        DEBUG("icmpv4: no one interested in type %d\n", hdr->type);
        gnrc_pktbuf_release(pkt);
    }
}

gnrc_pktsnip_t *gnrc_icmpv4_build(gnrc_pktsnip_t *next, uint8_t type,
                                 uint8_t code, size_t size)
{
    gnrc_pktsnip_t *pkt;
    icmpv4_hdr_t *icmpv4;

    if ((pkt = gnrc_pktbuf_add(next, NULL, size, GNRC_NETTYPE_ICMPV4)) == NULL) {
        DEBUG("icmpv4: no space left in packet buffer\n");
        return NULL;
    }

    DEBUG("icmpv4: building ICMPv4 message with type=%u, code=%u\n", type, code);
    icmpv4 = (icmpv4_hdr_t *)pkt->data;
    icmpv4->type = type;
    icmpv4->code = code;
    icmpv4->csum = byteorder_htons(0);

    return pkt;
}

int gnrc_icmpv4_calc_csum(gnrc_pktsnip_t *hdr, gnrc_pktsnip_t *pseudo_hdr)
{
    uint32_t csum = 0;

    /* kept for a uniform gnrc_netreg_calc_csum() dispatch signature; ICMPv4
     * does not use a pseudo-header (see _calc_csum() above) */
    (void)pseudo_hdr;

    if (hdr == NULL) {
        return -EFAULT;
    }
    if (hdr->type != GNRC_NETTYPE_ICMPV4) {
        return -EBADMSG;
    }

    csum = _calc_csum(hdr, hdr->next);

    if (csum == 0) {
        return -ENOENT;
    }

    ((icmpv4_hdr_t *)hdr->data)->csum = byteorder_htons(csum);

    return 0;
}

/** @} */
