/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_gnrc_ipv4_frag  Fragmentation and reassembly
 * @ingroup     net_gnrc_ipv4
 * @brief       IPv4 fragmentation and reassembly
 *
 * Reassembly uses a fixed-size pool of in-progress datagrams, each tracking
 * the byte ranges received so far as a linked list of intervals drawn from a
 * second, separately bounded pool shared across all in-progress datagrams.
 * A fragment whose range partially (but not exactly) overlaps a range
 * already recorded for its datagram discards the *entire* in-progress
 * reassembly rather than attempting to reconcile the overlap, per the
 * well-established security posture for this class of bug (the same
 * discard-on-overlap rule 6LoWPAN and RFC 8200 IPv6 fragmentation use in
 * this tree). A datagram that does not complete within
 * @ref CONFIG_GNRC_IPV4_FRAG_RBUF_TIMEOUT_MS is dropped, and an ICMPv4 time
 * exceeded (fragment reassembly time exceeded) message is sent back to the
 * sender, but only if the offset-0 fragment (the one carrying the actual
 * datagram header) was among those received -- RFC 792 has nothing sendable
 * to quote otherwise.
 *
 * @{
 *
 * @file
 * @brief   Fragmentation and reassembly definitions
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <stdint.h>

#include "net/gnrc/netif.h"
#include "net/gnrc/pkt.h"
#include "net/ipv4/hdr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Maximum number of datagrams reassembled at the same time
 */
#ifndef CONFIG_GNRC_IPV4_FRAG_RBUF_SIZE
#define CONFIG_GNRC_IPV4_FRAG_RBUF_SIZE         (1U)
#endif

/**
 * @brief   Maximum number of not-yet-coalesced fragment intervals, shared
 *          across all in-progress reassemblies
 */
#ifndef CONFIG_GNRC_IPV4_FRAG_LIMITS_POOL_SIZE
#define CONFIG_GNRC_IPV4_FRAG_LIMITS_POOL_SIZE  (CONFIG_GNRC_IPV4_FRAG_RBUF_SIZE * 4U)
#endif

/**
 * @brief   Time a partial datagram is kept before it is discarded, in ms
 *
 * @see <a href="https://tools.ietf.org/html/rfc1122#section-3.3.2">
 *          RFC 1122, section 3.3.2
 *      </a>
 *      recommends between 60 and 120 seconds.
 */
#ifndef CONFIG_GNRC_IPV4_FRAG_RBUF_TIMEOUT_MS
#define CONFIG_GNRC_IPV4_FRAG_RBUF_TIMEOUT_MS   (60U * 1000U)
#endif

/**
 * @brief   Message type for the reassembly buffer's garbage collection timer
 */
#define GNRC_IPV4_FRAG_GC                       (0x4fe1U)

/**
 * @brief   Initializes the fragmentation and reassembly module
 *
 * @note    Only to be called by gnrc_ipv4_init(), from the gnrc_ipv4 thread.
 */
void gnrc_ipv4_frag_init(void);

/**
 * @brief   Handles a received fragment
 *
 * @pre     @p pkt contains a marked @ref GNRC_NETTYPE_IPV4 snip whose flags
 *          or fragment offset indicate it is a fragment (more fragments set,
 *          or a non-zero fragment offset).
 * @pre     @p pkt is in the same shape @ref gnrc_ipv4_get_header() and the
 *          IPv4 receive path use: a payload snip, followed by the marked
 *          @ref GNRC_NETTYPE_IPV4 header snip, optionally followed by a
 *          @ref GNRC_NETTYPE_NETIF snip.
 *
 * @param[in] pkt   The received fragment. Always consumed by this function.
 *
 * @return  The reassembled datagram, in the same shape @p pkt was in, if
 *          this fragment completed it. The caller resumes normal receive
 *          handling (protocol demultiplexing) with the returned packet.
 * @return  NULL if @p pkt was queued, a duplicate, discarded (out of
 *          resources, malformed, or overlapping an already-received
 *          fragment), or the datagram is not yet complete. The caller must
 *          not touch @p pkt again.
 */
gnrc_pktsnip_t *gnrc_ipv4_frag_reass(gnrc_pktsnip_t *pkt);

/**
 * @brief   Handles a @ref GNRC_IPV4_FRAG_GC event
 */
void gnrc_ipv4_frag_gc(void);

/**
 * @brief   Sends a packet exceeding @p netif's MTU as a series of fragments
 *
 * @pre     `gnrc_pkt_len(pkt) > netif->ipv4.mtu`
 * @pre     @p pkt is in send order: a filled-in (checksummed)
 *          @ref GNRC_NETTYPE_IPV4 header snip, followed by its payload.
 * @pre     The don't-fragment flag is not set in @p pkt's header (the
 *          caller is expected to have already handled that case, since
 *          host-only GNRC IPv4 never forwards a datagram it did not
 *          originate itself: there is no third party to notify of a
 *          fragmentation-needed condition, only the local caller of the
 *          failed send).
 *
 * @param[in] pkt           The packet to fragment and send. Always consumed.
 * @param[in] netif         The outgoing interface.
 * @param[in] l2addr        The already-resolved link-layer address to send
 *                          each fragment to.
 * @param[in] l2addr_len    Length of @p l2addr.
 * @param[in] netif_hdr_flags   Flags to set on every fragment's
 *                              @ref gnrc_netif_hdr_t (e.g. broadcast or
 *                              multicast).
 */
void gnrc_ipv4_frag_send(gnrc_pktsnip_t *pkt, gnrc_netif_t *netif,
                        const uint8_t *l2addr, uint8_t l2addr_len,
                        uint8_t netif_hdr_flags);

/**
 * @brief   Number of reassembly buffer entries currently in use
 *
 * @internal Exposed for unit tests only.
 */
unsigned gnrc_ipv4_frag_rbuf_used(void);

/**
 * @brief   Discards all in-progress reassemblies, resetting the module to
 *          its just-initialized state
 *
 * @internal Exposed for unit tests only.
 */
void gnrc_ipv4_frag_reset(void);

#ifdef __cplusplus
}
#endif

/** @} */
