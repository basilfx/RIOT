/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "log.h"
#include "net/arp.h"
#include "net/dhcpv4/client.h"
#include "net/gnrc/ipv4/ft.h"
#include "net/gnrc/netapi.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/internal.h"
#include "net/netdev.h"
#include "net/sock.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief   The lease last applied via @ref dhcpv4_client_conf_lease(), kept
 *          around so @ref dhcpv4_client_release_lease() knows exactly what
 *          to undo (the address, the default route, and every classless
 *          static route it added).
 */
static dhcpv4_client_lease_t _applied;
static bool _has_lease;

int dhcpv4_client_get_hwaddr(uint16_t *netif_pid, uint8_t *hwaddr,
                             uint8_t *hwaddr_len)
{
    gnrc_netif_t *netif;
    int res;

    if (*netif_pid == SOCK_ADDR_ANY_NETIF) {
        netif = gnrc_netif_iter(NULL);
    }
    else {
        netif = gnrc_netif_get_by_pid(*netif_pid);
    }
    if (netif == NULL) {
        return -ENODEV;
    }

    switch (netif->device_type) {
    case NETDEV_TYPE_ETHERNET:
        res = gnrc_netapi_get(netif->pid, NETOPT_ADDRESS, 0, hwaddr,
                              GNRC_NETIF_L2ADDR_MAXLEN);
        if (res <= 0) {
            return -ENOTSUP;
        }
        *hwaddr_len = (uint8_t)res;
        /* resolve SOCK_ADDR_ANY_NETIF to the concrete interface just
         * picked -- every later call needs a real interface */
        *netif_pid = netif->pid;
        return ARP_HWTYPE_ETHERNET;
    default:
        LOG_ERROR("gnrc_dhcpv4_client: link-layer type of interface %u "
                  "not supported\n", netif->pid);
        return -ENOTSUP;
    }
}

void dhcpv4_client_conf_lease(unsigned netif_pid,
                              const dhcpv4_client_lease_t *lease)
{
    gnrc_netif_t *netif = gnrc_netif_get_by_pid(netif_pid);
    int idx;

    if (netif == NULL) {
        return;
    }

    idx = gnrc_netif_ipv4_addr_idx(netif, &lease->addr);
    if (idx < 0) {
        idx = gnrc_netif_ipv4_addr_add_internal(
            netif, &lease->addr, lease->prefix_len,
            GNRC_NETIF_IPV4_ADDRS_FLAGS_STATE_DHCP);
    }
    if (idx < 0) {
        LOG_ERROR("gnrc_dhcpv4_client: unable to configure leased address "
                  "on interface %u\n", netif_pid);
        return;
    }

    if (!ipv4_addr_is_unspecified(&lease->router)) {
        gnrc_ipv4_ft_add(NULL, 0, &lease->router, netif->pid);
    }
    for (unsigned i = 0; i < lease->routes_numof; i++) {
        const dhcpv4_client_route_t *route = &lease->routes[i];

        gnrc_ipv4_ft_add(&route->dst, route->dst_len, &route->gateway,
                         netif->pid);
    }

    _applied = *lease;
    _has_lease = true;
}

void dhcpv4_client_release_lease(unsigned netif_pid)
{
    gnrc_netif_t *netif = gnrc_netif_get_by_pid(netif_pid);

    if ((netif == NULL) || !_has_lease) {
        return;
    }

    for (unsigned i = 0; i < _applied.routes_numof; i++) {
        gnrc_ipv4_ft_remove(&_applied.routes[i].dst,
                            _applied.routes[i].dst_len);
    }
    if (!ipv4_addr_is_unspecified(&_applied.router)) {
        gnrc_ipv4_ft_remove(NULL, 0);
    }
    gnrc_netif_ipv4_addr_remove_internal(netif, &_applied.addr);
    _has_lease = false;
}

/** @} */
