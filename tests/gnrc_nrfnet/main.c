/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Tests extension header handling of gnrc stack.
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Takuo Yonezawa <Yonezawa-T2@mail.dnp.co.jp>
 *
 * @}
 */

#include <stdio.h>

#include "shell.h"

#include "msg.h"
#include "net/ipv6/addr.h"
#include "net/gnrc/ipv6/netif.h"
#include "net/gnrc/pkt.h"
#include "net/gnrc/pktbuf.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/netapi.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/gnrc/pktdump.h"

/*static void _init_interface(void)
{
    kernel_pid_t ifs[GNRC_NETIF_NUMOF];
    nrfnet_addr_t addr = NRFNET_ADDR_UNSPECIFIED;

    gnrc_netif_get(ifs);

    nrfnet_addr_from_str(&addr, "01");

    gnrc_nrfnet_netif_add_addr(ifs[0], &addr, 64, GNRC_IPV6_NETIF_ADDR_FLAGS_UNICAST);
}*/

static void _send_packet(void)
{
    kernel_pid_t ifs[GNRC_NETIF_NUMOF];

    gnrc_netif_get(ifs);

    struct {
        gnrc_netif_hdr_t netif_hdr;
        uint8_t src[2];
        uint8_t dst[2];
    } netif_hdr = {
        .src = { 0x00, 0x00 },
        .dst = { 0x00, 0x01 },
    };

    gnrc_netif_hdr_init(&(netif_hdr.netif_hdr), 2, 2);

    netif_hdr.netif_hdr.if_pid = ifs[0];

    uint8_t data1[] = {
        0x00, 0x00, 0x10, 0x00,     /* from (12 bits) + to (12 bits) */
        0xaa, 0x55,                 /* id */
        0xf0,                       /* length */
        0x34,                       /* offset + next */
        0x00,                       /* reserved */
        0x01, 0x02, 0x03, 0x04,     /* payload */
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a
    };

    gnrc_netreg_entry_t dump_nrfnet = { NULL, GNRC_NETREG_DEMUX_CTX_ALL, gnrc_pktdump_pid };

    gnrc_netreg_register(GNRC_NETTYPE_NRFNET, &dump_nrfnet);

    gnrc_pktsnip_t *netif1 = gnrc_pktbuf_add(NULL,
                                            &netif_hdr,
                                            sizeof(netif_hdr),
                                            GNRC_NETTYPE_NETIF);
    gnrc_pktsnip_t *pkt1 = gnrc_pktbuf_add(netif1,
                                           data1,
                                           sizeof(data1),
                                           GNRC_NETTYPE_NRFNET);

    gnrc_netapi_dispatch_receive(GNRC_NETTYPE_NRFNET, GNRC_NETREG_DEMUX_CTX_ALL, pkt1);
}

int main(void)
{
    //_init_interface();
    _send_packet();

    return 0;
}
