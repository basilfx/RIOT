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

#include <string.h>

#include "net/gnrc/ipv4/hdr.h"
#include "net/gnrc/nettype.h"
#include "net/gnrc/pktbuf.h"
#include "net/protnum.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/* For independent testing */
#ifdef MODULE_GNRC_NETTYPE_IPV4
#define HDR_NETTYPE (GNRC_NETTYPE_IPV4)
#else
#define HDR_NETTYPE (GNRC_NETTYPE_UNDEF)
#endif

gnrc_pktsnip_t *gnrc_ipv4_hdr_build(gnrc_pktsnip_t *payload,
                                   const ipv4_addr_t *src,
                                   const ipv4_addr_t *dst)
{
    gnrc_pktsnip_t *ipv4;
    ipv4_hdr_t *hdr;

    ipv4 = gnrc_pktbuf_add(payload, NULL, sizeof(ipv4_hdr_t), HDR_NETTYPE);

    if (ipv4 == NULL) {
        DEBUG("ipv4_hdr: no space left in packet buffer\n");
        return NULL;
    }

    hdr = (ipv4_hdr_t *)ipv4->data;
    memset(hdr, 0, sizeof(ipv4_hdr_t));

    if (src != NULL) {
        memcpy(&hdr->src, src, sizeof(ipv4_addr_t));
    }
    if (dst != NULL) {
        memcpy(&hdr->dst, dst, sizeof(ipv4_addr_t));
    }

    ipv4_hdr_set_version(hdr);
    ipv4_hdr_set_ihl(hdr, sizeof(ipv4_hdr_t));
    hdr->protocol = PROTNUM_RESERVED;

    return ipv4;
}

/** @} */
