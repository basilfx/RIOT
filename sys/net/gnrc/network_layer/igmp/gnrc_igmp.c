/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     net_gnrc_igmp
 * @{
 *
 * @file
 */

#include <assert.h>
#include <string.h>

#include "byteorder.h"
#include "evtimer_msg.h"
#include "mutex.h"
#include "net/gnrc.h"
#include "net/gnrc/igmp.h"
#include "net/gnrc/ipv4.h"
#include "net/gnrc/ipv4/hdr.h"
#include "net/igmp.h"
#include "net/inet_csum.h"
#include "net/protnum.h"
#include "random.h"
#include "ztimer.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/** @brief   State of a cache entry that is not currently awaiting a report */
#define _STATE_IDLE      (0U)
/** @brief   State of a cache entry with a report timer armed */
#define _STATE_DELAYING  (1U)

typedef struct {
    ipv4_addr_t group;          /**< unspecified (0.0.0.0) marks a free slot */
    kernel_pid_t iface;
    uint8_t state;
    bool last_reporter;
    uint32_t deadline_ms;       /**< ztimer_now(ZTIMER_MSEC) this entry's
                                 *   timer fires at; valid only while
                                 *   state == _STATE_DELAYING */
    evtimer_msg_event_t timeout_event;
} _igmp_group_t;

static mutex_t _lock = MUTEX_INIT;
static evtimer_msg_t _evtimer;
static _igmp_group_t _cache[CONFIG_GNRC_IGMP_GROUP_CACHE_SIZE];

/* caller holds _lock */
static _igmp_group_t *_find(kernel_pid_t iface, const ipv4_addr_t *group)
{
    for (unsigned i = 0; i < CONFIG_GNRC_IGMP_GROUP_CACHE_SIZE; i++) {
        /* a free slot's group is always the unspecified address; never
         * match it, even if the caller looks up 0.0.0.0 itself (e.g. an
         * attacker-controlled query's general-query group address) */
        if (ipv4_addr_is_unspecified(&_cache[i].group)) {
            continue;
        }
        if ((_cache[i].iface == iface) &&
            ipv4_addr_equal(&_cache[i].group, group)) {
            return &_cache[i];
        }
    }
    return NULL;
}

/* caller holds _lock */
static _igmp_group_t *_alloc(kernel_pid_t iface, const ipv4_addr_t *group)
{
    _igmp_group_t *entry = _find(iface, group);

    if (entry != NULL) {
        return entry;
    }
    for (unsigned i = 0; i < CONFIG_GNRC_IGMP_GROUP_CACHE_SIZE; i++) {
        if (ipv4_addr_is_unspecified(&_cache[i].group)) {
            _cache[i].iface = iface;
            _cache[i].group = *group;
            _cache[i].state = _STATE_IDLE;
            _cache[i].last_reporter = false;
            return &_cache[i];
        }
    }
    return NULL;
}

/* caller holds _lock */
static void _schedule_report(_igmp_group_t *entry, uint32_t max_delay_ms)
{
    uint32_t delay_ms = (max_delay_ms == 0) ? 0
                                            : random_uint32_range(0, max_delay_ms);
    uint32_t now = ztimer_now(ZTIMER_MSEC);

    if (entry->state == _STATE_DELAYING) {
        int32_t remaining = (int32_t)(entry->deadline_ms - now);

        /* RFC 2236, section 3: only reset the timer to the new value if it
         * is sooner than the one already running. A deadline that has
         * already passed (remaining <= 0) is never "sooner", so always
         * re-arm in that case rather than letting the unsigned subtraction
         * wrap around. */
        if ((remaining > 0) && (delay_ms >= (uint32_t)remaining)) {
            return;
        }
    }
    /* Unconditionally delete first, regardless of entry->state: the event
    * may have already fired and been unlinked by the evtimer handler while
    * its message is still sitting unprocessed in the gnrc_ipv4 thread's
    * queue, in which case entry->state still reads _STATE_DELAYING even
    * though the event is no longer in the evtimer list. evtimer_del() on
    * an event that is not linked is a harmless no-op, so calling it here
    * unconditionally is always safe -- unlike gating it on entry->state,
    * which can otherwise cause evtimer_add_msg() to splice an
    * already-linked node into the list a second time and self-loop it. */
    evtimer_del(&_evtimer, &entry->timeout_event.event);
    entry->state = _STATE_DELAYING;
    entry->deadline_ms = now + delay_ms;
    entry->timeout_event.event.offset = delay_ms;
    entry->timeout_event.msg.type = GNRC_IPV4_IGMP_TIMEOUT;
    entry->timeout_event.msg.content.ptr = entry;
    evtimer_add_msg(&_evtimer, &entry->timeout_event, gnrc_ipv4_pid);
}

static void _send_igmp(gnrc_netif_t *netif, uint8_t type,
                       const ipv4_addr_t *group, const ipv4_addr_t *dst)
{
    gnrc_pktsnip_t *igmp, *pkt, *netif_hdr;
    gnrc_netif_hdr_t *hdr;
    ipv4_hdr_t *ipv4_hdr;

    igmp = gnrc_igmp_build(type, 0, group);
    if (igmp == NULL) {
        DEBUG("igmp: no space left in packet buffer\n");
        return;
    }
    /* gnrc_ipv4_hdr_build() does not release its payload argument on
     * failure, so the IGMP snip built above must be kept in a variable of
     * its own to be released here, rather than losing the only reference
     * to it by overwriting the same variable with NULL */
    pkt = gnrc_ipv4_hdr_build(igmp, NULL, dst);
    if (pkt == NULL) {
        DEBUG("igmp: unable to allocate IPv4 header\n");
        gnrc_pktbuf_release(igmp);
        return;
    }
    ipv4_hdr = pkt->data;
    ipv4_hdr->protocol = PROTNUM_IGMP;
    ipv4_hdr->ttl = 1;

    netif_hdr = gnrc_netif_hdr_build(NULL, 0, NULL, 0);
    if (netif_hdr == NULL) {
        DEBUG("igmp: unable to allocate interface header\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    hdr = netif_hdr->data;
    gnrc_netif_hdr_set_netif(hdr, netif);
    pkt = gnrc_pkt_prepend(pkt, netif_hdr);

    if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_IPV4, GNRC_NETREG_DEMUX_CTX_ALL,
                                   pkt)) {
        DEBUG("igmp: unable to dispatch to ipv4\n");
        gnrc_pktbuf_release(pkt);
    }
}

static void _handle_query(gnrc_netif_t *netif, const igmp_hdr_t *hdr)
{
    uint32_t max_delay_ms = (uint32_t)hdr->max_resp_time * 100U;

    if (max_delay_ms == 0) {
        /* IGMPv1 query compatibility: RFC 2236 requires switching to v1
         * report behaviour and tracking a v1-router-present timer; GNRC
         * IPv4 host-mode approximates this by only widening the response
         * window to the default unsolicited interval, still answering as
         * a v2 host */
        max_delay_ms = CONFIG_GNRC_IGMP_UNSOLICITED_REPORT_INTERVAL_MS;
    }

    mutex_lock(&_lock);
    for (unsigned i = 0; i < CONFIG_GNRC_IGMP_GROUP_CACHE_SIZE; i++) {
        _igmp_group_t *entry = &_cache[i];

        if ((entry->iface != netif->pid) ||
            ipv4_addr_is_unspecified(&entry->group)) {
            continue;
        }
        if (ipv4_addr_is_unspecified(&hdr->group_addr) ||
            ipv4_addr_equal(&hdr->group_addr, &entry->group)) {
            _schedule_report(entry, max_delay_ms);
        }
    }
    mutex_unlock(&_lock);
}

static void _handle_report(gnrc_netif_t *netif, const igmp_hdr_t *hdr)
{
    mutex_lock(&_lock);
    _igmp_group_t *entry = _find(netif->pid, &hdr->group_addr);

    if (entry != NULL) {
        if (entry->state == _STATE_DELAYING) {
            evtimer_del(&_evtimer, &entry->timeout_event.event);
            entry->state = _STATE_IDLE;
        }
        entry->last_reporter = false;
    }
    mutex_unlock(&_lock);
}

/* When building a message, the checksum field is still zero at the time
 * this runs, so the returned value is exactly what belongs in that field.
 * When verifying a received message, the checksum field already holds the
 * sender's value, so a correct message makes this return 0 (the standard
 * one's-complement checksum self-check). Both call sites rely on this dual
 * behaviour; do not "fix" either one in isolation. */
static uint16_t _calc_csum(gnrc_pktsnip_t *igmp)
{
    uint16_t csum = inet_csum(0, igmp->data, igmp->size);

    return ~csum;
}

void gnrc_igmp_init(void)
{
    evtimer_init_msg(&_evtimer);
}

void gnrc_igmp_demux(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt)
{
    igmp_hdr_t *hdr;

    assert((netif != NULL) && (pkt != NULL));

    if (pkt->size < sizeof(igmp_hdr_t)) {
        DEBUG("igmp: packet too short\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    hdr = (igmp_hdr_t *)pkt->data;
    if (_calc_csum(pkt)) {
        DEBUG("igmp: wrong checksum\n");
        gnrc_pktbuf_release(pkt);
        return;
    }
    switch (hdr->type) {
    case IGMP_MEMBERSHIP_QUERY:
        DEBUG("igmp: handle membership query\n");
        _handle_query(netif, hdr);
        break;
    case IGMP_V1_MEMBERSHIP_REPORT:
    case IGMP_V2_MEMBERSHIP_REPORT:
        DEBUG("igmp: handle membership report\n");
        _handle_report(netif, hdr);
        break;
    default:
        DEBUG("igmp: unknown type field %u\n", hdr->type);
        break;
    }
    gnrc_pktbuf_release(pkt);
}

void gnrc_igmp_handle_timeout(void *ctx)
{
    _igmp_group_t *entry = ctx;
    gnrc_netif_t *netif = NULL;
    ipv4_addr_t addr = { .u32 = { 0 } };
    bool active;

    mutex_lock(&_lock);
    /* the message that triggered this call may be stale: the timer for
     * this entry could have already been cancelled or rescheduled between
     * the event firing and this handler running (see _schedule_report()),
     * or the slot could have been freed and reused for a different group
     * in the meantime. Only act on it if the entry is still armed for the
     * exact report this message was sent for. */
    active = !ipv4_addr_is_unspecified(&entry->group) &&
             (entry->state == _STATE_DELAYING);
    if (active) {
        entry->state = _STATE_IDLE;
        entry->last_reporter = true;
        netif = gnrc_netif_get_by_pid(entry->iface);
        addr = entry->group;
    }
    mutex_unlock(&_lock);
    if (active && (netif != NULL)) {
        _send_igmp(netif, IGMP_V2_MEMBERSHIP_REPORT, &addr, &addr);
    }
}

void gnrc_igmp_group_joined(gnrc_netif_t *netif, const ipv4_addr_t *addr)
{
    _igmp_group_t *entry;

    if (ipv4_addr_equal(addr, &ipv4_addr_all_hosts_group)) {
        /* RFC 2236, section 6: never report or leave the all-hosts group --
         * membership in it is permanent and administrative, not announced */
        return;
    }

    mutex_lock(&_lock);
    entry = _alloc(netif->pid, addr);
    if (entry != NULL) {
        entry->last_reporter = true;
        _schedule_report(entry, CONFIG_GNRC_IGMP_UNSOLICITED_REPORT_INTERVAL_MS);
    }
    mutex_unlock(&_lock);
    if (entry == NULL) {
        DEBUG("igmp: no space left in group cache\n");
        return;
    }
    _send_igmp(netif, IGMP_V2_MEMBERSHIP_REPORT, addr, addr);
}

void gnrc_igmp_group_left(gnrc_netif_t *netif, const ipv4_addr_t *addr)
{
    bool last_reporter;

    if (ipv4_addr_equal(addr, &ipv4_addr_all_hosts_group)) {
        /* RFC 2236, section 6: never report or leave the all-hosts group --
         * membership in it is permanent and administrative, not announced */
        return;
    }

    mutex_lock(&_lock);
    _igmp_group_t *entry = _find(netif->pid, addr);

    last_reporter = (entry != NULL) && entry->last_reporter;
    if (entry != NULL) {
        evtimer_del(&_evtimer, &entry->timeout_event.event);
        memset(entry, 0, sizeof(*entry));
    }
    mutex_unlock(&_lock);
    if (last_reporter) {
        _send_igmp(netif, IGMP_V2_LEAVE_GROUP, addr, &ipv4_addr_all_routers_group);
    }
}

gnrc_pktsnip_t *gnrc_igmp_build(uint8_t type, uint8_t max_resp_time,
                                const ipv4_addr_t *group)
{
    gnrc_pktsnip_t *pkt;
    igmp_hdr_t *igmp;

    pkt = gnrc_pktbuf_add(NULL, NULL, sizeof(igmp_hdr_t), GNRC_NETTYPE_UNDEF);
    if (pkt == NULL) {
        DEBUG("igmp: no space left in packet buffer\n");
        return NULL;
    }
    igmp = pkt->data;
    igmp->type = type;
    igmp->max_resp_time = max_resp_time;
    igmp->csum = byteorder_htons(0);
    igmp->group_addr = *group;
    igmp->csum = byteorder_htons(_calc_csum(pkt));

    return pkt;
}

/** @} */
