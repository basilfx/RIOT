/*
 * SPDX-FileCopyrightText: 2019 William MARTIN <william.martin@power-lan.com>
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup net_gnrc_netif
 * @{
 *
 * @file
 * @brief   IPv4 definitions for @ref net_gnrc_netif
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include "modules.h"

#include "net/ipv4/addr.h"
#include "net/gnrc/netif/conf.h"
#ifdef MODULE_NETSTATS_IPV4
#include "net/netstats.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    IPv4 unicast address flags
 * @anchor  net_gnrc_netif_ipv4_addrs_flags
 * @{
 */
/**
 * @brief   Mask for the address' state
 */
#define GNRC_NETIF_IPV4_ADDRS_FLAGS_STATE_MASK             (0x03U)

/**
 * @brief   Address slot is unused
 */
#define GNRC_NETIF_IPV4_ADDRS_FLAGS_STATE_UNUSED           (0x00U)

/**
 * @brief   Manually configured address
 */
#define GNRC_NETIF_IPV4_ADDRS_FLAGS_STATE_MANUAL           (0x01U)

/**
 * @brief   Address assigned by DHCPv4
 */
#define GNRC_NETIF_IPV4_ADDRS_FLAGS_STATE_DHCP             (0x02U)
/** @} */

/**
 * @brief   IPv4 component for @ref gnrc_netif_t
 *
 * @note only available with @ref net_gnrc_ipv4.
 */
typedef struct {
    /**
     * @brief   IPv4 unicast addresses of the interface
     *
     * @note    Only available with module `gnrc_netif_ipv4`.
     */
    ipv4_addr_t addrs[CONFIG_GNRC_NETIF_IPV4_ADDRS_NUMOF];

    /**
     * @brief   Prefix length of gnrc_netif_ipv4_t::addrs
     *
     * @note    Only available with module `gnrc_netif_ipv4`.
     */
    uint8_t prefix_lens[CONFIG_GNRC_NETIF_IPV4_ADDRS_NUMOF];

    /**
     * @brief   Flags for gnrc_netif_ipv4_t::addrs
     *
     * @see net_gnrc_netif_ipv4_addrs_flags
     *
     * @note    Only available with module `gnrc_netif_ipv4`.
     */
    uint8_t addrs_flags[CONFIG_GNRC_NETIF_IPV4_ADDRS_NUMOF];

    /**
     * @brief   IPv4 multicast groups of the interface
     *
     * @note    Only available with module `gnrc_netif_ipv4`.
     */
    ipv4_addr_t groups[CONFIG_GNRC_NETIF_IPV4_GROUPS_NUMOF];
#ifdef MODULE_NETSTATS_IPV4
    /**
     * @brief IPv4 packet statistics
     *
     * @note    Only available with module `netstats_ipv4`.
     */
    netstats_t stats;
#endif

    /**
     * @brief   Maximum transmission unit (MTU) for IPv4 packets
     *
     * @note    Only available with module `gnrc_netif_ipv4`.
     */
    uint16_t mtu;
} gnrc_netif_ipv4_t;

#ifdef __cplusplus
}
#endif

/** @} */
