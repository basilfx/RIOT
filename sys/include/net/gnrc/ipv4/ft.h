/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_gnrc_ipv4_ft    Forwarding table
 * @ingroup     net_gnrc_ipv4
 * @brief       IPv4 forwarding (route) table
 *
 * GNRC IPv4 is host-only: the table is only consulted to find the next hop
 * (a gateway) for destinations that are not on a directly connected subnet.
 * On-link destinations are recognized directly from the addresses configured
 * on an interface (@ref gnrc_netif_ipv4_addr_match) and never need an entry
 * here.
 * @{
 *
 * @file
 * @brief   Forwarding table definitions
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <stdint.h>

#include "net/ipv4/addr.h"
#include "sched.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Maximum number of entries in the forwarding table
 */
#ifndef CONFIG_GNRC_IPV4_FT_SIZE
#define CONFIG_GNRC_IPV4_FT_SIZE    (4)
#endif

/**
 * @brief   A forwarding table entry
 */
typedef struct {
    ipv4_addr_t dst;        /**< destination or prefix */
    ipv4_addr_t next_hop;   /**< gateway to gnrc_ipv4_ft_t::dst */
    uint8_t dst_len;        /**< prefix length in bits of
                             *   gnrc_ipv4_ft_t::dst. 0 for the default
                             *   route */
    kernel_pid_t iface;     /**< interface to gnrc_ipv4_ft_t::next_hop */
} gnrc_ipv4_ft_t;

/**
 * @brief   Gets the best (longest prefix) matching forwarding table entry to
 *          a destination
 *
 * @pre `(dst != NULL) && (fte != NULL)`
 *
 * @param[in] dst   The destination.
 * @param[out] fte  The resulting forwarding table entry.
 *
 * @return  0, on success.
 * @return  -ENETUNREACH, if no route was found.
 */
int gnrc_ipv4_ft_get(const ipv4_addr_t *dst, gnrc_ipv4_ft_t *fte);

/**
 * @brief   Adds a new route to the forwarding table
 *
 * If an entry for @p dst/@p dst_len already exists, it is overwritten.
 *
 * @param[in] dst       The destination of the route. May be NULL or
 *                      the unspecified address (0.0.0.0) for the default
 *                      route.
 * @param[in] dst_len   The prefix length of @p dst in bits. 0 for the
 *                      default route.
 * @param[in] next_hop  The gateway to @p dst/@p dst_len.
 * @param[in] iface     The interface to @p next_hop.
 *
 * @return  0, on success.
 * @return  -ENOMEM, if there was no space left in the forwarding table.
 */
int gnrc_ipv4_ft_add(const ipv4_addr_t *dst, uint8_t dst_len,
                     const ipv4_addr_t *next_hop, kernel_pid_t iface);

/**
 * @brief   Removes a route from the forwarding table
 *
 * @param[in] dst       The destination of the route to remove. May be NULL
 *                      or the unspecified address (0.0.0.0) for the default
 *                      route.
 * @param[in] dst_len   The prefix length of @p dst in bits. 0 for the
 *                      default route.
 */
void gnrc_ipv4_ft_remove(const ipv4_addr_t *dst, uint8_t dst_len);

#ifdef __cplusplus
}
#endif

/** @} */
