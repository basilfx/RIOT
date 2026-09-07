/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_gnrc_ipv4_arp   ARP
 * @ingroup     net_gnrc_ipv4
 * @brief       Address resolution protocol (ARP) cache and state machine
 *
 * The cache is a flat table shared by all interfaces, with every entry
 * tagged by the interface it belongs to. Unlike the IPv6 NIB neighbor cache,
 * there is no reachability confirmation via upper-layer hints (no NUD): an
 * entry is either unresolved (a request is outstanding) or resolved, and it
 * is re-resolved from scratch once it expires. Each unresolved entry holds
 * at most one pending packet; a new send to the same unresolved destination
 * replaces (and drops) whatever was queued before it.
 * @{
 *
 * @file
 * @brief   ARP cache and state machine definitions
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include "net/gnrc/netif.h"
#include "net/ipv4/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Maximum number of entries in the ARP cache
 */
#ifndef CONFIG_GNRC_IPV4_ARP_CACHE_SIZE
#define CONFIG_GNRC_IPV4_ARP_CACHE_SIZE             (4U)
#endif

/**
 * @brief   Time between two retransmissions of an ARP request, in ms
 */
#ifndef CONFIG_GNRC_IPV4_ARP_REQUEST_TIMEOUT_MS
#define CONFIG_GNRC_IPV4_ARP_REQUEST_TIMEOUT_MS     (1000U)
#endif

/**
 * @brief   Maximum number of ARP request retransmissions before giving up
 */
#ifndef CONFIG_GNRC_IPV4_ARP_MAX_RETRIES
#define CONFIG_GNRC_IPV4_ARP_MAX_RETRIES            (3U)
#endif

/**
 * @brief   Lifetime of a resolved cache entry before it is re-resolved, in ms
 */
#ifndef CONFIG_GNRC_IPV4_ARP_CACHE_TIMEOUT_MS
#define CONFIG_GNRC_IPV4_ARP_CACHE_TIMEOUT_MS        (20U * 60U * 1000U)
#endif

/**
 * @brief   Message type for ARP retransmission and expiry timers
 */
#define GNRC_IPV4_ARP_TIMEOUT                        (0x4fe0U)

/**
 * @brief   Initializes the ARP cache
 *
 * @note    Only to be called by gnrc_ipv4_init(), from the gnrc_ipv4 thread.
 */
void gnrc_ipv4_arp_init(void);

/**
 * @brief   Resolves the link-layer address for @p dst on @p netif and sends
 *          @p pkt to it
 *
 * If the address is already resolved, @p pkt is sent immediately. Otherwise
 * @p pkt is queued (replacing any packet already queued for @p dst) and an
 * ARP request is sent (or left to be retransmitted, if one is already
 * outstanding).
 *
 * @pre `(netif != NULL) && (dst != NULL) && (pkt != NULL)`
 *
 * @param[in,out] netif the network interface
 * @param[in] dst       the destination to resolve. Must not be multicast.
 * @param[in] pkt       the packet to send once resolved. Always consumed by
 *                      this function.
 */
void gnrc_ipv4_arp_request(gnrc_netif_t *netif, const ipv4_addr_t *dst,
                          gnrc_pktsnip_t *pkt);

/**
 * @brief   Handles a received ARP packet
 *
 * Opportunistically caches the sender's mapping (this also handles
 * gratuitous ARP), and replies if the packet is a request for one of
 * @p netif's addresses.
 *
 * @pre `(netif != NULL) && (pkt != NULL)`
 *
 * @param[in] netif     the network interface the packet was received on
 * @param[in] pkt       the received ARP packet. Always consumed by this
 *                      function.
 */
void gnrc_ipv4_arp_handle_pkt(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt);

/**
 * @brief   Handles a @ref GNRC_IPV4_ARP_TIMEOUT event
 *
 * @param[in] ctx   `msg_t::content::ptr` of the timeout message
 */
void gnrc_ipv4_arp_handle_timeout(void *ctx);

#ifdef __cplusplus
}
#endif

/** @} */
