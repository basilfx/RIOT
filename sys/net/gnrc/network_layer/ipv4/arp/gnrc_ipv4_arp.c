/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 */

#include <errno.h>
#include <string.h>

#include "byteorder.h"
#include "evtimer_msg.h"
#include "net/arp.h"
#include "net/ethernet/hdr.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv4.h"
#include "net/gnrc/ipv4/arp.h"
#include "net/gnrc/netif/internal.h"

#define ENABLE_DEBUG 0
#include "debug.h"

enum {
    _STATE_FREE = 0,
    _STATE_INCOMPLETE,
    _STATE_REACHABLE,
};

typedef struct {
    ipv4_addr_t ipv4;
    uint8_t l2addr[GNRC_NETIF_L2ADDR_MAXLEN];
    uint8_t l2addr_len;
    uint8_t state;
    uint8_t probes_sent;
    kernel_pid_t iface;
    evtimer_msg_event_t timeout_event;
    gnrc_pktsnip_t *pending;
    uint32_t last_used;             /**< _use_counter at the last use */
} _arp_entry_t;

static evtimer_msg_t _evtimer;
static _arp_entry_t _cache[CONFIG_GNRC_IPV4_ARP_CACHE_SIZE];

/**
 * @brief   Counter that orders the cache entries by their last use.
 *
 * Incremented on every use of an entry, so a smaller value marks an entry
 * that was used longer ago. A wrap around would only ever pick a suboptimal
 * entry for reuse, and at one increment per packet it is far out of reach.
 */
static uint32_t _use_counter;

static void _sched_timeout(_arp_entry_t *entry, uint32_t ms)
{
    evtimer_del(&_evtimer, &entry->timeout_event.event);
    entry->timeout_event.event.offset = ms;
    entry->timeout_event.msg.type = GNRC_IPV4_ARP_TIMEOUT;
    entry->timeout_event.msg.content.ptr = entry;
    evtimer_add_msg(&_evtimer, &entry->timeout_event, gnrc_ipv4_pid);
}

/**
 * @brief   Looks up the entry for @p addr on @p iface, and marks it as used.
 *
 * Every caller looks the entry up in order to act on it, either to send to
 * that address or to refresh it from a packet received from it, so the
 * lookup itself is what orders the entries for @ref _pick_victim().
 */
static _arp_entry_t *_find(kernel_pid_t iface, const ipv4_addr_t *addr)
{
    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_ARP_CACHE_SIZE; i++) {
        if ((_cache[i].state != _STATE_FREE) && (_cache[i].iface == iface) &&
            ipv4_addr_equal(&_cache[i].ipv4, addr)) {
            _cache[i].last_used = ++_use_counter;
            return &_cache[i];
        }
    }
    return NULL;
}

static void _free(_arp_entry_t *entry)
{
    evtimer_del(&_evtimer, &entry->timeout_event.event);
    if (entry->pending != NULL) {
        gnrc_pktbuf_release(entry->pending);
        entry->pending = NULL;
    }
    entry->state = _STATE_FREE;
}

/**
 * @brief   Picks the entry to reuse when every entry of a full cache is
 *          taken.
 *
 * A reachable entry is preferred over an incomplete one, as the latter has a
 * resolution in flight and a packet waiting on it. Among the candidates the
 * one used longest ago is picked.
 */
static _arp_entry_t *_pick_victim(void)
{
    _arp_entry_t *victim = NULL;

    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_ARP_CACHE_SIZE; i++) {
        if (_cache[i].state != _STATE_REACHABLE) {
            continue;
        }
        if ((victim == NULL) || (_cache[i].last_used < victim->last_used)) {
            victim = &_cache[i];
        }
    }
    if (victim != NULL) {
        return victim;
    }
    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_ARP_CACHE_SIZE; i++) {
        if ((victim == NULL) || (_cache[i].last_used < victim->last_used)) {
            victim = &_cache[i];
        }
    }
    return victim;
}

static _arp_entry_t *_alloc(kernel_pid_t iface, const ipv4_addr_t *addr)
{
    _arp_entry_t *entry = NULL;

    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_ARP_CACHE_SIZE; i++) {
        if (_cache[i].state == _STATE_FREE) {
            entry = &_cache[i];
            break;
        }
    }
    /* A full cache reuses its least useful entry rather than refusing to
     * resolve. Refusing leaves this host unable to reach any address it has
     * not resolved yet, and since an entry is kept for
     * CONFIG_GNRC_IPV4_ARP_CACHE_TIMEOUT_MS, on a segment holding more hosts
     * than the cache has entries that is indistinguishable from a loss of
     * connectivity: a peer whose SYN arrives cannot even be answered */
    if (entry == NULL) {
        entry = _pick_victim();
        if (entry == NULL) {
            return NULL;
        }
        _free(entry);
    }
    memset(entry, 0, sizeof(*entry));
    entry->ipv4 = *addr;
    entry->iface = iface;
    entry->last_used = ++_use_counter;
    return entry;
}

/**
 * @brief   Picks the address to use as sender protocol address for an ARP
 *          request, i.e. the first configured address on @p netif.
 */
static const ipv4_addr_t *_netif_primary_addr(gnrc_netif_t *netif)
{
    for (unsigned i = 0; i < CONFIG_GNRC_NETIF_IPV4_ADDRS_NUMOF; i++) {
        if (netif->ipv4.addrs_flags[i] != GNRC_NETIF_IPV4_ADDRS_FLAGS_STATE_UNUSED) {
            return &netif->ipv4.addrs[i];
        }
    }
    return NULL;
}

static gnrc_pktsnip_t *_build_arp_pkt(gnrc_netif_t *netif, uint16_t op,
                                     const uint8_t *tha, const ipv4_addr_t *spa,
                                     const ipv4_addr_t *tpa)
{
    gnrc_pktsnip_t *pkt = gnrc_pktbuf_add(NULL, NULL, sizeof(arp_ipv4_hdr_t),
                                         GNRC_NETTYPE_ARP);
    arp_ipv4_hdr_t *hdr;
    static const ipv4_addr_t unspecified = { .u8 = { 0, 0, 0, 0 } };

    if (pkt == NULL) {
        DEBUG("gnrc_ipv4_arp: no space left in packet buffer\n");
        return NULL;
    }
    hdr = pkt->data;
    hdr->hdr.hrd = byteorder_htons(ARP_HWTYPE_ETHERNET);
    hdr->hdr.pro = byteorder_htons(ARP_PROTOTYPE_IPV4);
    hdr->hdr.hln = ETHERNET_ADDR_LEN;
    hdr->hdr.pln = sizeof(ipv4_addr_t);
    hdr->hdr.op = byteorder_htons(op);
    memcpy(hdr->sha, netif->l2addr, ETHERNET_ADDR_LEN);
    hdr->spa = (spa != NULL) ? *spa : unspecified;
    if (tha != NULL) {
        memcpy(hdr->tha, tha, ETHERNET_ADDR_LEN);
    }
    else {
        memset(hdr->tha, 0, ETHERNET_ADDR_LEN);
    }
    hdr->tpa = *tpa;
    return pkt;
}

static void _send(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt,
                  const uint8_t *dst, bool broadcast)
{
    gnrc_pktsnip_t *netif_hdr = gnrc_netif_hdr_build(NULL, 0,
                                                     broadcast ? NULL : dst,
                                                     broadcast ? 0 : ETHERNET_ADDR_LEN);
    gnrc_netif_hdr_t *hdr;

    if (netif_hdr == NULL) {
        DEBUG("gnrc_ipv4_arp: unable to allocate netif header\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    hdr = netif_hdr->data;
    if (broadcast) {
        hdr->flags |= GNRC_NETIF_HDR_FLAGS_BROADCAST;
    }
    gnrc_netif_hdr_set_netif(hdr, netif);
    pkt = gnrc_pkt_prepend(pkt, netif_hdr);
    if (gnrc_netif_send(netif, pkt) < 1) {
        DEBUG("gnrc_ipv4_arp: unable to send packet\n");
        gnrc_pktbuf_release(pkt);
    }
}

static void _send_request(gnrc_netif_t *netif, const ipv4_addr_t *dst)
{
    gnrc_pktsnip_t *pkt = _build_arp_pkt(netif, ARP_OPCODE_REQUEST, NULL,
                                        _netif_primary_addr(netif), dst);

    if (pkt != NULL) {
        _send(netif, pkt, NULL, true);
    }
}

static void _send_reply(gnrc_netif_t *netif, const uint8_t *tha,
                        const ipv4_addr_t *spa, const ipv4_addr_t *tpa)
{
    gnrc_pktsnip_t *pkt = _build_arp_pkt(netif, ARP_OPCODE_REPLY, tha, spa, tpa);

    if (pkt != NULL) {
        _send(netif, pkt, tha, false);
    }
}

static void _send_pending(_arp_entry_t *entry, gnrc_netif_t *netif)
{
    gnrc_pktsnip_t *pkt = entry->pending;

    entry->pending = NULL;
    if (pkt != NULL) {
        gnrc_ipv4_send_to_iface(netif, pkt, entry->l2addr, entry->l2addr_len, 0);
    }
}

void gnrc_ipv4_arp_init(void)
{
    evtimer_init_msg(&_evtimer);
}

void gnrc_ipv4_arp_request(gnrc_netif_t *netif, const ipv4_addr_t *dst,
                          gnrc_pktsnip_t *pkt)
{
    _arp_entry_t *entry;

    assert((netif != NULL) && (dst != NULL) && (pkt != NULL));
    assert(!ipv4_addr_is_multicast(dst));
    entry = _find(netif->pid, dst);
    if ((entry != NULL) && (entry->state == _STATE_REACHABLE)) {
        gnrc_ipv4_send_to_iface(netif, pkt, entry->l2addr, entry->l2addr_len, 0);
        return;
    }
    if (entry == NULL) {
        entry = _alloc(netif->pid, dst);
        if (entry == NULL) {
            DEBUG("gnrc_ipv4_arp: cache full, dropping packet\n");
            gnrc_pktbuf_release(pkt);
            return;
        }
    }
    else if (entry->pending != NULL) {
        /* a newer packet supersedes whatever was still waiting on this
         * unresolved address */
        gnrc_pktbuf_release(entry->pending);
    }
    entry->pending = pkt;
    if (entry->state != _STATE_INCOMPLETE) {
        entry->state = _STATE_INCOMPLETE;
        entry->probes_sent = 1;
        _send_request(netif, dst);
        _sched_timeout(entry, CONFIG_GNRC_IPV4_ARP_REQUEST_TIMEOUT_MS);
    }
}

void gnrc_ipv4_arp_handle_pkt(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    arp_ipv4_hdr_t hdr;
    const ipv4_addr_t *ours = NULL;
    bool is_reply;

    assert((netif != NULL) && (pkt != NULL));
    if ((pkt->data == NULL) || (pkt->size < sizeof(arp_ipv4_hdr_t))) {
        DEBUG("gnrc_ipv4_arp: packet too short\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    /* copy out: pkt->data may not be aligned for arp_ipv4_hdr_t access */
    memcpy(&hdr, pkt->data, sizeof(hdr));
    if ((byteorder_ntohs(hdr.hdr.hrd) != ARP_HWTYPE_ETHERNET) ||
        (byteorder_ntohs(hdr.hdr.pro) != ARP_PROTOTYPE_IPV4) ||
        (hdr.hdr.hln != ETHERNET_ADDR_LEN) ||
        (hdr.hdr.pln != sizeof(ipv4_addr_t))) {
        DEBUG("gnrc_ipv4_arp: unsupported ARP packet format\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    is_reply = (byteorder_ntohs(hdr.hdr.op) == ARP_OPCODE_REPLY);

    /* Determine whether the packet asks after one of this host's addresses,
     * which is the case both for a request from a peer that is about to talk
     * to this host and for the reply to a request of this host's own */
    for (unsigned i = 0; i < CONFIG_GNRC_NETIF_IPV4_ADDRS_NUMOF; i++) {
        if ((netif->ipv4.addrs_flags[i] != GNRC_NETIF_IPV4_ADDRS_FLAGS_STATE_UNUSED) &&
            ipv4_addr_equal(&netif->ipv4.addrs[i], &hdr.tpa)) {
            ours = &netif->ipv4.addrs[i];
            break;
        }
    }

    /* Learn or refresh the sender's mapping. An entry that is already held
     * is always refreshed, which is what keeps the cache correct when a peer
     * announces a new link layer address by gratuitous ARP. A new entry is
     * only added for a packet addressed to this host, as every other host on
     * the segment keeps broadcasting requests for peers this host never
     * talks to, and holding those for
     * CONFIG_GNRC_IPV4_ARP_CACHE_TIMEOUT_MS would evict the peers that do
     * matter. ARP probes (RFC 5227) carry no sender address and are skipped */
    if (hdr.spa.u32.u32 != 0) {
        _arp_entry_t *entry = _find(netif->pid, &hdr.spa);
        bool was_incomplete;

        if ((entry == NULL) && (ours != NULL)) {
            entry = _alloc(netif->pid, &hdr.spa);
        }
        if (entry != NULL) {
            was_incomplete = (entry->state == _STATE_INCOMPLETE);
            memcpy(entry->l2addr, hdr.sha, ETHERNET_ADDR_LEN);
            entry->l2addr_len = ETHERNET_ADDR_LEN;
            entry->state = _STATE_REACHABLE;
            entry->probes_sent = 0;
            _sched_timeout(entry, CONFIG_GNRC_IPV4_ARP_CACHE_TIMEOUT_MS);
            if (was_incomplete) {
                _send_pending(entry, netif);
            }
        }
    }
    if (!is_reply && (ours != NULL)) {
        _send_reply(netif, hdr.sha, ours, &hdr.spa);
    }
    gnrc_pktbuf_release(pkt);
}

void gnrc_ipv4_arp_handle_timeout(void *ctx)
{
    _arp_entry_t *entry = ctx;
    gnrc_netif_t *netif = gnrc_netif_get_by_pid(entry->iface);

    if (netif == NULL) {
        _free(entry);
        return;
    }
    switch (entry->state) {
    case _STATE_INCOMPLETE:
        if (entry->probes_sent >= CONFIG_GNRC_IPV4_ARP_MAX_RETRIES) {
            DEBUG("gnrc_ipv4_arp: giving up on address resolution\n");
            _free(entry);
            return;
        }
        entry->probes_sent++;
        _send_request(netif, &entry->ipv4);
        _sched_timeout(entry, CONFIG_GNRC_IPV4_ARP_REQUEST_TIMEOUT_MS);
        break;
    case _STATE_REACHABLE:
        /* force re-resolution on next send */
        entry->state = _STATE_FREE;
        break;
    default:
        break;
    }
}

/** @} */
