/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_dhcpv4_client   DHCPv4 client
 * @ingroup     net_dhcpv4
 * @brief       DHCPv4 client implementation
 *
 * This client implements the host side of the RFC 2131 state machine:
 * `INIT` -> `SELECTING` -> `REQUESTING` -> `BOUND` -> `RENEWING` ->
 * `REBINDING` -> (back to `INIT` on failure or lease expiry). It is stack
 * independent: it never touches a network interface directly, instead
 * calling the [stack-specific functions](@ref net_dhcpv4_client_stack) below
 * to read the interface's hardware address and to apply or release a lease.
 * @{
 *
 * @file
 * @brief   DHCPv4 client definitions
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <stdint.h>

#include "event.h"
#include "net/ipv4/addr.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Auto-initialization parameters
 * @{
 */
#ifndef DHCPV4_CLIENT_STACK_SIZE
#define DHCPV4_CLIENT_STACK_SIZE    (THREAD_STACKSIZE_DEFAULT)  /**< stack size */
#endif

#ifndef DHCPV4_CLIENT_PRIORITY
#define DHCPV4_CLIENT_PRIORITY      (THREAD_PRIORITY_MAIN - 2)  /**< priority */
#endif
/** @} */

/**
 * @defgroup net_dhcpv4_client_conf DHCPv4 client compile configurations
 * @ingroup  config
 * @{
 */
/**
 * @brief   Maximum number of classless static routes (option 121) applied
 *          from a single lease
 */
#ifndef CONFIG_DHCPV4_CLIENT_ROUTE_MAX
#define CONFIG_DHCPV4_CLIENT_ROUTE_MAX  (2U)
#endif

/**
 * @brief   Maximum random delay before the first DISCOVER (and before every
 *          restart at `INIT`), in ms
 */
#ifndef CONFIG_DHCPV4_CLIENT_INIT_DELAY_MS
#define CONFIG_DHCPV4_CLIENT_INIT_DELAY_MS      (2000U)
#endif

/**
 * @brief   Initial retransmission timeout for DISCOVER/REQUEST/RENEW/REBIND,
 *          in ms
 *
 * @see [RFC 2131, section 4.1](https://tools.ietf.org/html/rfc2131#section-4.1)
 */
#ifndef CONFIG_DHCPV4_CLIENT_INITIAL_RT_MS
#define CONFIG_DHCPV4_CLIENT_INITIAL_RT_MS      (4000U)
#endif

/**
 * @brief   Upper bound the retransmission timeout is doubled to, in ms
 */
#ifndef CONFIG_DHCPV4_CLIENT_MAX_RT_MS
#define CONFIG_DHCPV4_CLIENT_MAX_RT_MS          (64000U)
#endif

/**
 * @brief   Maximum number of REQUEST retransmissions in `SELECTING` before
 *          giving up on the offer and restarting at `INIT`
 */
#ifndef CONFIG_DHCPV4_CLIENT_REQUEST_MAX_RC
#define CONFIG_DHCPV4_CLIENT_REQUEST_MAX_RC     (4U)
#endif

/**
 * @brief   Delay before restarting at `INIT` after a DECLINE, in ms
 *
 * @see [RFC 2131, section 3.1, step 5]
 *      (https://tools.ietf.org/html/rfc2131#section-3.1)
 */
#ifndef CONFIG_DHCPV4_CLIENT_DECLINE_DELAY_MS
#define CONFIG_DHCPV4_CLIENT_DECLINE_DELAY_MS   (10000U)
#endif
/** @} */

/**
 * @brief   A classless static route (option 121) as parsed from a lease
 */
typedef struct {
    ipv4_addr_t dst;        /**< destination or prefix. Unspecified (0.0.0.0)
                             *   for the default route */
    ipv4_addr_t gateway;    /**< gateway to dhcpv4_client_route_t::dst */
    uint8_t dst_len;        /**< prefix length in bits of
                             *   dhcpv4_client_route_t::dst. 0 for the default
                             *   route */
} dhcpv4_client_route_t;

/**
 * @brief   A DHCPv4 lease, as handed to @ref dhcpv4_client_conf_lease()
 */
typedef struct {
    ipv4_addr_t addr;       /**< the leased address (yiaddr) */
    uint8_t prefix_len;     /**< prefix length derived from the subnet mask
                             *   option, or 32 if that option was absent */
    ipv4_addr_t router;     /**< the default router (option 3), or the
                             *   unspecified address if that option was
                             *   absent or empty */
    uint32_t lease_time;    /**< lease time in seconds. @ref UINT32_MAX means
                             *   an infinite lease */
    /**
     * @brief   Classless static routes (option 121)
     */
    dhcpv4_client_route_t routes[CONFIG_DHCPV4_CLIENT_ROUTE_MAX];
    unsigned routes_numof;  /**< number of entries used in
                             *   dhcpv4_client_lease_t::routes */
} dhcpv4_client_lease_t;

#if defined(MODULE_AUTO_INIT_DHCPV4_CLIENT) || defined(DOXYGEN)
/**
 * @brief   Auto-initializes the client in its own thread, for
 *          @ref SOCK_ADDR_ANY_NETIF
 *
 * @note    Only available with (and called by) the `auto_init_dhcpv4_client`
 *          module.
 */
void dhcpv4_client_auto_init(void);
#endif

/**
 * @brief   Initializes the client
 *
 * @pre `event_queue->waiter != NULL`
 *
 * @param[in] event_queue   Event queue to use with the client. Needs to be
 *                          initialized in the handler thread.
 * @param[in] netif         The network interface the client should acquire
 *                          a lease for.
 */
void dhcpv4_client_init(event_queue_t *event_queue, uint16_t netif);

/**
 * @brief   Starts the client
 *
 * @pre @ref dhcpv4_client_init() was called.
 */
void dhcpv4_client_start(void);

/**
 * @brief   Declines the current (or most recently offered) lease
 *
 * Sends a DHCPDECLINE for the address currently being requested or already
 * leased (e.g. because some external mechanism, such as a manual ARP probe,
 * detected that it is already in use by another host), releases any local
 * configuration for it, and restarts the client at `INIT` after a short
 * delay, per [RFC 2131, section 3.1, step 5]
 * (https://tools.ietf.org/html/rfc2131#section-3.1).
 *
 * A no-op if the client has no address to decline (i.e. it is currently in
 * `INIT` or `SELECTING`).
 */
void dhcpv4_client_decline(void);

/**
 * @name    Stack-specific functions
 * @anchor  net_dhcpv4_client_stack
 *
 * These functions need to be provided by the network-stack implementation.
 * @{
 */
/**
 * @brief   Gets the hardware address of @p netif for the client's `chaddr`
 *          field
 *
 * @param[in,out] netif     The network interface the client is bound to, as
 *                          passed to @ref dhcpv4_client_init(). If this is
 *                          @ref SOCK_ADDR_ANY_NETIF, it is resolved to
 *                          whichever concrete interface was picked and
 *                          updated in place, since every later call the
 *                          client makes (@ref dhcpv4_client_conf_lease(),
 *                          @ref dhcpv4_client_release_lease()) needs a real
 *                          interface, not the wildcard.
 * @param[out] hwaddr       The resulting hardware address.
 * @param[out] hwaddr_len   Length of @p hwaddr in bytes.
 *
 * @return  The hardware type, see @ref net_arp_hwtype, on success.
 * @return  A negative errno on error.
 */
int dhcpv4_client_get_hwaddr(uint16_t *netif, uint8_t *hwaddr,
                             uint8_t *hwaddr_len);

/**
 * @brief   Applies a lease acquired via DHCPv4 to @p netif
 *
 * @param[in] netif     Network interface the lease was acquired on.
 * @param[in] lease     The lease to apply.
 */
void dhcpv4_client_conf_lease(unsigned netif,
                              const dhcpv4_client_lease_t *lease);

/**
 * @brief   Releases a previously applied lease from @p netif
 *
 * Called when a lease is declined, NAKed during renewal, or expires without
 * being renewed.
 *
 * @param[in] netif     Network interface to release the lease from.
 */
void dhcpv4_client_release_lease(unsigned netif);
/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
