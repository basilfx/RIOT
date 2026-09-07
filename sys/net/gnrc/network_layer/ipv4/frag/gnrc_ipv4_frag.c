/*
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
#include "evtimer_msg.h"
#include "net/gnrc.h"
#include "net/gnrc/icmpv4/error.h"
#include "net/gnrc/ipv4.h"
#include "net/gnrc/ipv4/frag.h"
#include "net/ipv4/addr.h"
#include "ztimer.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief   A half-open `[start, end)` byte interval of a fragment already
 *          received for some reassembly buffer entry
 */
typedef struct _frag_int {
    struct _frag_int *next;
    uint16_t start;
    uint16_t end;
} _frag_int_t;

typedef struct {
    ipv4_addr_t src;
    ipv4_addr_t dst;
    uint8_t protocol;
    bool used;
    bool last_seen;             /**< a fragment with the more-fragments flag
                                 *   unset has been received */
    uint16_t id;
    uint16_t datagram_len;      /**< total payload length; valid only if
                                 *   last_seen */
    uint32_t arrival;           /**< ztimer_now(ZTIMER_MSEC) of the last
                                 *   fragment received for this entry */
    _frag_int_t *ints;          /**< byte ranges received so far */
    gnrc_pktsnip_t *data;       /**< reassembled payload accumulator */
    /**
     * @brief   The offset-0 fragment's own, complete, as-received packet
     *          (held via gnrc_pktbuf_hold()), kept only in case this entry
     *          times out and needs to be quoted in an ICMPv4 time exceeded
     *          message. NULL if the offset-0 fragment has not arrived yet.
     */
    gnrc_pktsnip_t *frag0_orig;
} _frag_entry_t;

typedef enum {
    _FRAG_NEW,
    _FRAG_DUPLICATE,
    _FRAG_OVERLAP,
} _frag_check_t;

static _frag_entry_t _entries[CONFIG_GNRC_IPV4_FRAG_RBUF_SIZE];
static _frag_int_t _int_pool[CONFIG_GNRC_IPV4_FRAG_LIMITS_POOL_SIZE];

static evtimer_msg_t _evtimer;
static evtimer_msg_event_t _gc_event;

static void _release_entry(_frag_entry_t *entry)
{
    for (_frag_int_t *i = entry->ints; i != NULL; ) {
        _frag_int_t *next = i->next;

        i->start = 0;
        i->end = 0;     /* mark free; start >= end is never true for an
                         * interval actually in use, since zero-length
                         * fragments never get an interval (see
                         * _store_fragment()) */
        i = next;
    }
    entry->ints = NULL;
    if (entry->data != NULL) {
        gnrc_pktbuf_release(entry->data);
        entry->data = NULL;
    }
    if (entry->frag0_orig != NULL) {
        gnrc_pktbuf_release(entry->frag0_orig);
        entry->frag0_orig = NULL;
    }
    entry->used = false;
}

static void _reschedule_gc(void)
{
    evtimer_del(&_evtimer, &_gc_event.event);
    _gc_event.event.offset = CONFIG_GNRC_IPV4_FRAG_RBUF_TIMEOUT_MS;
    _gc_event.msg.type = GNRC_IPV4_FRAG_GC;
    evtimer_add_msg(&_evtimer, &_gc_event, gnrc_ipv4_pid);
}

void gnrc_ipv4_frag_init(void)
{
    evtimer_init_msg(&_evtimer);
}

unsigned gnrc_ipv4_frag_rbuf_used(void)
{
    unsigned used = 0;

    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_FRAG_RBUF_SIZE; i++) {
        if (_entries[i].used) {
            used++;
        }
    }
    return used;
}

void gnrc_ipv4_frag_reset(void)
{
    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_FRAG_RBUF_SIZE; i++) {
        if (_entries[i].used) {
            _release_entry(&_entries[i]);
        }
    }
    evtimer_del(&_evtimer, &_gc_event.event);
}

void gnrc_ipv4_frag_gc(void)
{
    uint32_t now = ztimer_now(ZTIMER_MSEC);
    bool any_active = false;

    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_FRAG_RBUF_SIZE; i++) {
        _frag_entry_t *entry = &_entries[i];

        if (!entry->used) {
            continue;
        }
        if ((uint32_t)(now - entry->arrival) >= CONFIG_GNRC_IPV4_FRAG_RBUF_TIMEOUT_MS) {
            DEBUG("ipv4_frag: reassembly timed out\n");
            if (entry->frag0_orig != NULL) {
                gnrc_icmpv4_error_time_exc_send(ICMPV4_ERROR_TIME_EXC_FRAG,
                                                entry->frag0_orig);
            }
            _release_entry(entry);
        }
        else {
            any_active = true;
        }
    }
    if (any_active) {
        _reschedule_gc();
    }
}

static _frag_entry_t *_find(const ipv4_addr_t *src, const ipv4_addr_t *dst,
                            uint8_t protocol, uint16_t id)
{
    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_FRAG_RBUF_SIZE; i++) {
        _frag_entry_t *entry = &_entries[i];

        if (entry->used && (entry->protocol == protocol) && (entry->id == id) &&
            ipv4_addr_equal(&entry->src, src) && ipv4_addr_equal(&entry->dst, dst)) {
            return entry;
        }
    }
    return NULL;
}

static _frag_entry_t *_get_entry(const ipv4_addr_t *src, const ipv4_addr_t *dst,
                                 uint8_t protocol, uint16_t id)
{
    _frag_entry_t *entry = _find(src, dst, protocol, id);
    _frag_entry_t *free_slot = NULL;
    _frag_entry_t *oldest = NULL;
    uint32_t oldest_age = 0;
    uint32_t now;

    if (entry != NULL) {
        return entry;
    }

    now = ztimer_now(ZTIMER_MSEC);
    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_FRAG_RBUF_SIZE; i++) {
        _frag_entry_t *e = &_entries[i];
        uint32_t age;

        if (!e->used) {
            free_slot = e;
            break;
        }
        age = (uint32_t)(now - e->arrival);
        if ((oldest == NULL) || (age > oldest_age)) {
            oldest = e;
            oldest_age = age;
        }
    }
    if (free_slot == NULL) {
        if (oldest == NULL) {
            /* CONFIG_GNRC_IPV4_FRAG_RBUF_SIZE == 0 */
            return NULL;
        }
        DEBUG("ipv4_frag: reassembly buffer full, evicting oldest entry\n");
        _release_entry(oldest);
        free_slot = oldest;
    }

    memset(free_slot, 0, sizeof(*free_slot));
    free_slot->used = true;
    free_slot->src = *src;
    free_slot->dst = *dst;
    free_slot->protocol = protocol;
    free_slot->id = id;
    return free_slot;
}

static _frag_check_t _check_overlap(const _frag_entry_t *entry,
                                    uint16_t start, uint16_t end)
{
    for (const _frag_int_t *i = entry->ints; i != NULL; i = i->next) {
        /* half-open intervals [start, end) overlap iff each starts before
         * the other ends */
        if ((start < i->end) && (i->start < end)) {
            if ((start == i->start) && (end == i->end)) {
                return _FRAG_DUPLICATE;
            }
            return _FRAG_OVERLAP;
        }
    }
    return _FRAG_NEW;
}

static _frag_int_t *_int_pool_get_free(void)
{
    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_FRAG_LIMITS_POOL_SIZE; i++) {
        if (_int_pool[i].start >= _int_pool[i].end) {
            return &_int_pool[i];
        }
    }
    return NULL;
}

static bool _add_interval(_frag_entry_t *entry, uint16_t start, uint16_t end)
{
    _frag_int_t *node = _int_pool_get_free();

    if (node == NULL) {
        return false;
    }
    node->start = start;
    node->end = end;
    node->next = entry->ints;
    entry->ints = node;
    return true;
}

static bool _store_fragment(_frag_entry_t *entry, uint16_t start, uint16_t len,
                            const gnrc_pktsnip_t *pkt)
{
    uint16_t need;

    if (len == 0) {
        return true;
    }
    need = start + len;
    if (entry->data == NULL) {
        entry->data = gnrc_pktbuf_add(NULL, NULL, need, GNRC_NETTYPE_UNDEF);
        if (entry->data == NULL) {
            return false;
        }
    }
    else if (entry->data->size < need) {
        if (gnrc_pktbuf_realloc_data(entry->data, need) != 0) {
            return false;
        }
    }
    memcpy(((uint8_t *)entry->data->data) + start, pkt->data, len);
    return true;
}

/**
 * @brief   Checks whether the intervals received so far contiguously cover
 *          `[0, entry->datagram_len)`, with no gaps
 */
static bool _no_gaps(const _frag_entry_t *entry)
{
    uint16_t covered = 0;
    bool progress = true;

    while (progress && (covered < entry->datagram_len)) {
        progress = false;
        for (const _frag_int_t *i = entry->ints; i != NULL; i = i->next) {
            if (i->start == covered) {
                covered = i->end;
                progress = true;
                break;
            }
        }
    }
    return covered == entry->datagram_len;
}

static gnrc_pktsnip_t *_finish(_frag_entry_t *entry)
{
    gnrc_pktsnip_t *frag0_ipv4 = gnrc_pktsnip_search_type(entry->frag0_orig,
                                                          GNRC_NETTYPE_IPV4);
    ipv4_hdr_t *frag0_hdr;
    uint16_t ihl;
    gnrc_pktsnip_t *hdr_snip;
    ipv4_hdr_t *hdr;
    gnrc_pktsnip_t *payload;
    gnrc_pktsnip_t *frag0_netif;

    assert(frag0_ipv4 != NULL);
    frag0_hdr = frag0_ipv4->data;
    ihl = ipv4_hdr_get_ihl(frag0_hdr);

    /* clone the offset-0 fragment's header bytes (incl. any options)
     * verbatim, then clear the fragmentation fields: this is now a whole,
     * unfragmented datagram */
    hdr_snip = gnrc_pktbuf_add(NULL, frag0_hdr, ihl, GNRC_NETTYPE_IPV4);
    if (hdr_snip == NULL) {
        DEBUG("ipv4_frag: out of memory finishing reassembly\n");
        _release_entry(entry);
        return NULL;
    }
    hdr = hdr_snip->data;
    ipv4_hdr_set_flags(hdr, ipv4_hdr_get_flags(hdr) & ~IPV4_HDR_FLAGS_MF);
    ipv4_hdr_set_fo(hdr, 0);
    hdr->tl = byteorder_htons(ihl + entry->datagram_len);
    hdr->csum = byteorder_htons(0);
    hdr->csum = ipv4_hdr_csum(hdr);

    payload = entry->data;
    if (payload == NULL) {
        /* datagram_len == 0: the offset-0 fragment carried no payload and
         * was also the last fragment */
        payload = gnrc_pktbuf_add(NULL, NULL, 0, GNRC_NETTYPE_UNDEF);
        if (payload == NULL) {
            DEBUG("ipv4_frag: out of memory finishing reassembly\n");
            gnrc_pktbuf_release(hdr_snip);
            _release_entry(entry);
            return NULL;
        }
    }
    entry->data = NULL;         /* ownership transferred to payload */
    payload->next = hdr_snip;
    payload->type = GNRC_NETTYPE_UNDEF;

    frag0_netif = gnrc_pktsnip_search_type(entry->frag0_orig, GNRC_NETTYPE_NETIF);
    if (frag0_netif != NULL) {
        gnrc_netif_t *netif = gnrc_netif_hdr_get_netif(frag0_netif->data);
        gnrc_pktsnip_t *netif_snip = gnrc_netif_hdr_build(NULL, 0, NULL, 0);

        if (netif_snip != NULL) {
            gnrc_netif_hdr_set_netif(netif_snip->data, netif);
            hdr_snip->next = netif_snip;
        }
        /* if this allocation fails, proceed without a netif snip rather
         * than failing the whole (already successful) reassembly */
    }

    _release_entry(entry);
    return payload;
}

gnrc_pktsnip_t *gnrc_ipv4_frag_reass(gnrc_pktsnip_t *pkt)
{
    gnrc_pktsnip_t *ipv4_snip = gnrc_pktsnip_search_type(pkt, GNRC_NETTYPE_IPV4);
    ipv4_hdr_t *hdr;
    uint16_t ihl;
    uint16_t frag_start;
    uint16_t frag_end;
    uint16_t payload_len;
    bool more_frags;
    _frag_entry_t *entry;
    gnrc_pktsnip_t *completed = NULL;

    assert(ipv4_snip != NULL);
    hdr = ipv4_snip->data;
    ihl = ipv4_hdr_get_ihl(hdr);
    frag_start = ipv4_hdr_get_fo(hdr) * 8U;
    more_frags = (ipv4_hdr_get_flags(hdr) & IPV4_HDR_FLAGS_MF) != 0;
    payload_len = pkt->size;

    if (more_frags && (payload_len & 0x7U)) {
        DEBUG("ipv4_frag: non-last fragment length not a multiple of 8, "
              "dropping\n");
        gnrc_pktbuf_release(pkt);
        return NULL;
    }
    if (((uint32_t)frag_start + payload_len) > (uint32_t)(UINT16_MAX - ihl)) {
        DEBUG("ipv4_frag: fragment offset + length exceeds maximum "
              "datagram size, dropping\n");
        gnrc_pktbuf_release(pkt);
        return NULL;
    }
    frag_end = frag_start + payload_len;

    entry = _get_entry(&hdr->src, &hdr->dst, hdr->protocol,
                       byteorder_ntohs(hdr->id));
    if (entry == NULL) {
        DEBUG("ipv4_frag: no space in reassembly buffer, dropping "
              "fragment\n");
        gnrc_pktbuf_release(pkt);
        return NULL;
    }

    if (entry->last_seen && (frag_end > entry->datagram_len)) {
        DEBUG("ipv4_frag: fragment extends past known datagram length, "
              "discarding reassembly\n");
        _release_entry(entry);
        gnrc_pktbuf_release(pkt);
        return NULL;
    }
    if (!more_frags && entry->last_seen && (entry->datagram_len != frag_end)) {
        DEBUG("ipv4_frag: conflicting datagram length from last fragment, "
              "discarding reassembly\n");
        _release_entry(entry);
        gnrc_pktbuf_release(pkt);
        return NULL;
    }

    switch (_check_overlap(entry, frag_start, frag_end)) {
        case _FRAG_OVERLAP:
            DEBUG("ipv4_frag: overlapping, non-identical fragment, "
                  "discarding reassembly\n");
            _release_entry(entry);
            gnrc_pktbuf_release(pkt);
            return NULL;
        case _FRAG_DUPLICATE:
            DEBUG("ipv4_frag: duplicate fragment, ignoring\n");
            entry->arrival = ztimer_now(ZTIMER_MSEC);
            _reschedule_gc();
            gnrc_pktbuf_release(pkt);
            return NULL;
        default:
            break;
    }

    if (!_store_fragment(entry, frag_start, payload_len, pkt)) {
        DEBUG("ipv4_frag: out of memory, discarding reassembly\n");
        _release_entry(entry);
        gnrc_pktbuf_release(pkt);
        return NULL;
    }
    if ((payload_len > 0) && !_add_interval(entry, frag_start, frag_end)) {
        DEBUG("ipv4_frag: no space left for fragment bookkeeping, "
              "discarding reassembly\n");
        _release_entry(entry);
        gnrc_pktbuf_release(pkt);
        return NULL;
    }

    if (!more_frags) {
        entry->last_seen = true;
        entry->datagram_len = frag_end;
    }
    if ((frag_start == 0) && (entry->frag0_orig == NULL)) {
        entry->frag0_orig = pkt;
        gnrc_pktbuf_hold(pkt, 1);
    }

    entry->arrival = ztimer_now(ZTIMER_MSEC);

    if (entry->last_seen && _no_gaps(entry)) {
        completed = _finish(entry);
    }
    else {
        _reschedule_gc();
    }

    gnrc_pktbuf_release(pkt);
    return completed;
}

static void _copy_payload(const gnrc_pktsnip_t *payload, size_t offset,
                          void *dst, size_t len)
{
    size_t pos = 0;

    while (len > 0) {
        size_t snip_end;

        assert(payload != NULL);
        snip_end = pos + payload->size;
        if (offset < snip_end) {
            size_t snip_off = offset - pos;
            size_t avail = payload->size - snip_off;
            size_t chunk = (avail < len) ? avail : len;

            memcpy(dst, ((const uint8_t *)payload->data) + snip_off, chunk);
            dst = ((uint8_t *)dst) + chunk;
            offset += chunk;
            len -= chunk;
        }
        pos = snip_end;
        payload = payload->next;
    }
}

void gnrc_ipv4_frag_send(gnrc_pktsnip_t *pkt, gnrc_netif_t *netif,
                        const uint8_t *l2addr, uint8_t l2addr_len,
                        uint8_t netif_hdr_flags)
{
    ipv4_hdr_t *orig_hdr = pkt->data;
    ipv4_addr_t src = orig_hdr->src;
    ipv4_addr_t dst = orig_hdr->dst;
    uint8_t protocol = orig_hdr->protocol;
    uint8_t ttl = orig_hdr->ttl;
    uint8_t tos = orig_hdr->ts;
    network_uint16_t id = orig_hdr->id;
    gnrc_pktsnip_t *payload = pkt->next;
    size_t payload_len = gnrc_pkt_len(payload);
    /* every fragment but the last must carry a payload length that is a
     * multiple of 8 bytes (the fragment offset field's unit) */
    size_t max_frag_payload = (netif->ipv4.mtu - sizeof(ipv4_hdr_t)) & ~((size_t)0x7U);
    size_t offset = 0;

    assert(!(ipv4_hdr_get_flags(orig_hdr) & IPV4_HDR_FLAGS_DF));

    if (max_frag_payload == 0) {
        DEBUG("ipv4_frag: MTU too small to fragment, dropping\n");
        gnrc_pktbuf_release_error(pkt, EMSGSIZE);
        return;
    }

    while (offset < payload_len) {
        size_t chunk = payload_len - offset;
        bool last = true;
        gnrc_pktsnip_t *data;
        gnrc_pktsnip_t *hdr_snip;
        ipv4_hdr_t *hdr;
        gnrc_pktsnip_t *netif_hdr;
        gnrc_netif_hdr_t *nh;
        gnrc_pktsnip_t *frame;

        if (chunk > max_frag_payload) {
            chunk = max_frag_payload;
            last = false;
        }

        data = gnrc_pktbuf_add(NULL, NULL, chunk, GNRC_NETTYPE_UNDEF);
        if (data == NULL) {
            DEBUG("ipv4_frag: out of memory building fragment, aborting\n");
            break;
        }
        _copy_payload(payload, offset, data->data, chunk);

        hdr_snip = gnrc_pktbuf_add(data, NULL, sizeof(ipv4_hdr_t),
                                   GNRC_NETTYPE_IPV4);
        if (hdr_snip == NULL) {
            DEBUG("ipv4_frag: out of memory building fragment header, "
                  "aborting\n");
            gnrc_pktbuf_release(data);
            break;
        }
        hdr = hdr_snip->data;
        memset(hdr, 0, sizeof(ipv4_hdr_t));
        ipv4_hdr_set_version(hdr);
        ipv4_hdr_set_ihl(hdr, sizeof(ipv4_hdr_t));
        hdr->ts = tos;
        hdr->id = id;
        hdr->ttl = ttl;
        hdr->protocol = protocol;
        hdr->src = src;
        hdr->dst = dst;
        ipv4_hdr_set_flags(hdr, last ? 0 : IPV4_HDR_FLAGS_MF);
        ipv4_hdr_set_fo(hdr, (uint16_t)(offset / 8U));
        hdr->tl = byteorder_htons(sizeof(ipv4_hdr_t) + chunk);
        hdr->csum = byteorder_htons(0);
        hdr->csum = ipv4_hdr_csum(hdr);

        netif_hdr = gnrc_netif_hdr_build(NULL, 0, l2addr, l2addr_len);
        if (netif_hdr == NULL) {
            DEBUG("ipv4_frag: out of memory building netif header, "
                  "aborting\n");
            gnrc_pktbuf_release(hdr_snip);
            break;
        }
        nh = netif_hdr->data;
        nh->flags |= netif_hdr_flags;
        gnrc_netif_hdr_set_netif(nh, netif);

        frame = gnrc_pkt_prepend(hdr_snip, netif_hdr);
        if (gnrc_netif_send(netif, frame) < 1) {
            DEBUG("ipv4_frag: unable to send fragment\n");
            gnrc_pktbuf_release(frame);
        }

        offset += chunk;
    }

    gnrc_pktbuf_release(pkt);
}

/** @} */
