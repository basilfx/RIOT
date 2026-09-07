/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_gnrc_igmp   IGMPv2
 * @ingroup     net_gnrc_ipv4
 * @brief       GNRC's implementation of IGMPv2 host-mode multicast group
 *              membership reporting
 *
 * GNRC IPv4 is host-only (see @ref net_gnrc_ipv4): this module implements
 * only the host side of RFC 2236 -- sending unsolicited and query-triggered
 * membership reports and leave-group messages. It never originates a
 * membership query.
 *
 * @see <a href="https://tools.ietf.org/html/rfc2236">
 *          RFC 2236
 *      </a>
 * @{
 *
 * @file
 * @brief       Definitions for GNRC's IGMPv2 implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#include "net/gnrc/netif.h"
#include "net/gnrc/pkt.h"
#include "net/ipv4/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup    net_gnrc_igmp_conf  GNRC IGMP compile configurations
 * @ingroup     net_gnrc_igmp
 * @ingroup     net_gnrc_conf
 * @{
 */
/**
 * @brief   Maximum number of joined groups tracked across all interfaces
 */
#ifndef CONFIG_GNRC_IGMP_GROUP_CACHE_SIZE
#define CONFIG_GNRC_IGMP_GROUP_CACHE_SIZE               (4U)
#endif

/**
 * @brief   Delay, in ms, between an unsolicited membership report and its
 *          one repetition, and the response window used for a query whose
 *          Max Response Time is 0 (IGMPv1 compatibility, see
 *          @ref gnrc_igmp_demux())
 */
#ifndef CONFIG_GNRC_IGMP_UNSOLICITED_REPORT_INTERVAL_MS
#define CONFIG_GNRC_IGMP_UNSOLICITED_REPORT_INTERVAL_MS (10U * 1000U)
#endif
/** @} */

/**
 * @brief   Message type for IGMP report/query response timers
 */
#define GNRC_IPV4_IGMP_TIMEOUT                          (0x4fe2U)

/**
 * @brief   Initializes the IGMP module
 *
 * @note    Only to be called by gnrc_ipv4_init(), from the gnrc_ipv4 thread.
 */
void gnrc_igmp_init(void);

/**
 * @brief   Demultiplexes a received IGMP packet according to its type field
 *
 * @pre     `(netif != NULL) && (pkt != NULL)`
 * @pre     @p pkt's first snip is the (unmarked) IGMP message, i.e. the
 *          shape @ref gnrc_ipv4.c's `_demux()` hands to a protocol handler.
 *
 * @param[in] netif     The receiving interface
 * @param[in] pkt       The packet to demultiplex. Always consumed by this
 *                      function.
 */
void gnrc_igmp_demux(gnrc_netif_t *netif, gnrc_pktsnip_t *pkt);

/**
 * @brief   Handles a @ref GNRC_IPV4_IGMP_TIMEOUT event
 *
 * @param[in] ctx   `msg_t::content::ptr` of the timeout message
 */
void gnrc_igmp_handle_timeout(void *ctx);

/**
 * @brief   Notifies IGMP that @p netif joined the multicast group @p addr
 *
 * Sends an unsolicited membership report immediately, and once more after
 * @ref CONFIG_GNRC_IGMP_UNSOLICITED_REPORT_INTERVAL_MS to cover the
 * possibility of the first one being lost (RFC 2236, section 3).
 *
 * @pre `(netif != NULL) && (addr != NULL)`
 * @pre @p addr is already reflected in `netif->ipv4.groups[]`
 *
 * @param[in] netif the network interface that joined the group
 * @param[in] addr  the address of the multicast group
 */
void gnrc_igmp_group_joined(gnrc_netif_t *netif, const ipv4_addr_t *addr);

/**
 * @brief   Notifies IGMP that @p netif left the multicast group @p addr
 *
 * If this host was the last one to report membership in @p addr (i.e. no
 * other host's report was heard since), sends a Leave Group message to the
 * all-routers group (224.0.0.2), per RFC 2236 section 3.
 *
 * @pre `(netif != NULL) && (addr != NULL)`
 *
 * @param[in] netif the network interface that left the group
 * @param[in] addr  the address of the multicast group
 */
void gnrc_igmp_group_left(gnrc_netif_t *netif, const ipv4_addr_t *addr);

/**
 * @brief   Builds an IGMPv2 message for sending, with checksum already
 *          calculated.
 *
 * @internal Exposed for unit tests only.
 *
 * @param[in] type          Type for the IGMP message.
 * @param[in] max_resp_time Max Response Time field (queries only, 0
 *                          otherwise).
 * @param[in] group         Group address for the message.
 *
 * @return  The IGMP message on success
 * @return  NULL, on failure
 */
gnrc_pktsnip_t *gnrc_igmp_build(uint8_t type, uint8_t max_resp_time,
                                const ipv4_addr_t *group);

#ifdef __cplusplus
}
#endif

/** @} */
