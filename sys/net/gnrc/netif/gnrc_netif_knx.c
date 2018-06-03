/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_gnrc_netif
 * @{
 *
 * @file
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "assert.h"
#include "net/gnrc.h"
#include "net/knx/telegram.h"

#define ENABLE_DEBUG 0
#include "debug.h"

static gnrc_pktsnip_t *_recv(gnrc_netif_t *netif)
{
    netdev_t *dev = netif->dev;
    int pktlen;
    gnrc_pktsnip_t *payload;
    gnrc_pktsnip_t *hdr;
    gnrc_netif_hdr_t *netif_hdr;

    assert(dev);

    /* see how much data there is to process */
    pktlen = dev->driver->recv(dev, NULL, 0, NULL);

    if (pktlen == 0) {
        DEBUG("[gnrc_netif_knx] _recv: no data available to process\n");
        return NULL;
    }
    else if (pktlen < 0) {
        DEBUG("[gnrc_netif_knx] _recv: error while receiving\n");
        return NULL;
    }

    /* allocate space for the packet in the pktbuf */
#ifdef MODULE_GNRC_KNX_L3
    payload = gnrc_pktbuf_add(NULL, NULL, pktlen, GNRC_NETTYPE_KNX_L3);
#else
    payload = gnrc_pktbuf_add(NULL, NULL, pktlen, GNRC_NETTYPE_UNDEF);
#endif

    if (payload == NULL) {
        DEBUG("[gnrc_netif_knx] _recv: unable to allocate space in the pktbuf\n");
        dev->driver->recv(dev, NULL, pktlen, NULL);
        return NULL;
    }

    /* copy the complete telegram into the packet buffer */
    dev->driver->recv(dev, payload->data, pktlen, NULL);

    /* ensure telegram is supported */
    if (!knx_telegram_is(payload->data, payload->size)) {
        DEBUG("[gnrc_netif_knx] _receive: invalid or incomplete telegram\n");
        gnrc_pktbuf_release(payload);
        return NULL;
    }

    /* verify checksum of telegram */
    if (!knx_telegram_is_checksum_valid(payload->data, payload->size)) {
        DEBUG("[gnrc_netif_knx] _receive: telegram checksum mismatch\n");
        gnrc_pktbuf_release(payload);
        return NULL;
    }

    /* create a netif hdr from the obtained data */
    knx_addr_t src, dst;

    knx_telegram_get_src_addr(payload->data, &src);
    knx_telegram_get_dst_addr(payload->data, &dst);

#ifdef MODULE_L2FILTER
    if (!l2filter_pass(dev->filter, (uint8_t *)&src, KNX_ADDR_LEN)) {
        DEBUG("[gnrc_netif_knx] _receive: incoming packet filtered by l2filter\n");
        gnrc_pktbuf_release(payload);
        return NULL;
    }
#endif

    hdr = gnrc_netif_hdr_build((uint8_t *)&src, sizeof(src),
                               (uint8_t *)&dst, sizeof(dst));

    if (hdr == NULL) {
        DEBUG("[gnrc_netif_knx] _recv: unable to allocate netif header\n");
        gnrc_pktbuf_release(payload);
        return NULL;
    }

    netif_hdr = (gnrc_netif_hdr_t *)hdr->data;

    if (knx_telegram_is_group_addressed(payload->data)) {
        netif_hdr->flags |= GNRC_NETIF_HDR_FLAGS_MULTICAST;
    }

    gnrc_netif_hdr_set_netif(netif_hdr, netif);

    /* append the netif header */
    payload = gnrc_pkt_append(payload, hdr);

#ifdef MODULE_NETSTATS_L2
    /* update netstats */
    netif->stats.rx_count++;
    netif->stats.rx_bytes += pktlen;
#endif

    DEBUG("[gnrc_netif_knx] _recv: successfully parsed packet\n");

    return payload;
}

static int _send(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    gnrc_pktsnip_t *payload;
    netdev_t *dev = netif->dev;

    assert(dev);

    /* check for packet */
    if (pkt == NULL) {
        DEBUG("[gnrc_netif_knx] _send: pkt is NULL\n");
        return -EINVAL;
    }

    /* find generic netif header */
    if (pkt->type != GNRC_NETTYPE_NETIF) {
        DEBUG("[gnrc_netif_knx] _send: first header is not a generic netif header\n");
        return -EBADMSG;
    }

    /* drop telegrams that are not telegrams */
    payload = pkt->next;

    if (!knx_telegram_is(payload->data, payload->size)) {
        DEBUG("[gnrc_netif_knx] _send: invalid or incomplete telegram\n");
        gnrc_pktbuf_release(pkt);
        return -EBADMSG;
    }

#ifdef MODULE_NETSTATS_L2
    /* update netstats */
    if (knx_telegram_is_group_addressed(payload->data)) {
        netif->stats.tx_mcast_count++;
    }
    else {
        netif->stats.tx_unicast_count++;
    }
#endif

    /* calculate checksum */
    knx_telegram_update_checksum((uint8_t *)payload->data, payload->size);

    /* send telegram */
    iolist_t iolist = {
        .iol_next = NULL,
        .iol_base = payload->data,
        .iol_len = payload->size
    };

    int res = dev->driver->send(dev, &iolist);

    /* release packet */
    gnrc_pktbuf_release(pkt);

    DEBUG("[gnrc_netif_knx] _send: telegram send %d\n", res);

    return res;
}

static const gnrc_netif_ops_t gnrc_knx_netif_ops = {
    .init = gnrc_netif_default_init,
    .send = _send,
    .recv = _recv,
    .get = gnrc_netif_get_from_netdev,
    .set = gnrc_netif_set_from_netdev,
};

int gnrc_netif_knx_create(gnrc_netif_t *netif, char *stack, int stacksize,
                          char priority, char *name, netdev_t *dev)
{
    return gnrc_netif_create(netif, stack, stacksize, priority, name,
                             dev, &gnrc_knx_netif_ops);
}
