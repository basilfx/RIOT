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

#include <stdlib.h>
#include <string.h>

#include "net/gnrc/ipv4/ft.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/internal.h"
#include "net/gnrc/netif/ipv4.h"
#include "net/ipv4/addr.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief   ID of the interface to configure, set to 0 to auto-select the
 *          only interface
 */
#ifndef CONFIG_GNRC_IPV4_STATIC_ADDR_IFACE
#define CONFIG_GNRC_IPV4_STATIC_ADDR_IFACE 0
#endif

/**
 * @brief   Splits an "address/prefix_len" string in place
 *
 * @param[in,out] str           the string to split, mutated to hold only the
 *                              address part
 * @param[out] prefix_len       the parsed prefix length, or 32 if @p str has
 *                              no "/prefix_len" suffix
 */
static void _split_prefix_len(char *str, uint8_t *prefix_len)
{
    char *sep = strchr(str, '/');

    if (sep == NULL) {
        *prefix_len = 32;
        return;
    }

    *sep = '\0';
    *prefix_len = (uint8_t)atoi(sep + 1);
}

static void _config_addr(gnrc_netif_t *netif)
{
    const char *static_addr =
#ifdef CONFIG_GNRC_IPV4_STATIC_ADDR
        CONFIG_GNRC_IPV4_STATIC_ADDR;

#else
        NULL;
#endif

    if (static_addr == NULL) {
        return;
    }

    char addr_str[IPV4_ADDR_MAX_STR_LEN + 4];
    uint8_t prefix_len;
    ipv4_addr_t addr;

    strncpy(addr_str, static_addr, sizeof(addr_str) - 1);
    addr_str[sizeof(addr_str) - 1] = '\0';

    _split_prefix_len(addr_str, &prefix_len);

    if (ipv4_addr_from_str(&addr, addr_str) == NULL) {
        DEBUG("gnrc_ipv4_static_addr: invalid address %s\n", static_addr);
        return;
    }

    gnrc_netif_ipv4_addr_add_internal(netif, &addr, prefix_len,
                                      GNRC_NETIF_IPV4_ADDRS_FLAGS_STATE_MANUAL);
}

static void _config_default_router(gnrc_netif_t *netif)
{
    const char *static_router =
#ifdef CONFIG_GNRC_IPV4_STATIC_DEFAULT_ROUTER
        CONFIG_GNRC_IPV4_STATIC_DEFAULT_ROUTER;

#else
        NULL;
#endif

    if (static_router == NULL) {
        return;
    }

    ipv4_addr_t router;

    if (ipv4_addr_from_str(&router, static_router) == NULL) {
        DEBUG("gnrc_ipv4_static_addr: invalid default router %s\n",
              static_router);
        return;
    }

    gnrc_ipv4_ft_add(NULL, 0, &router, netif->pid);
}

void auto_init_gnrc_ipv4_static_addr(void)
{
    gnrc_netif_t *netif;

    if (CONFIG_GNRC_IPV4_STATIC_ADDR_IFACE) {
        netif = gnrc_netif_get_by_pid(CONFIG_GNRC_IPV4_STATIC_ADDR_IFACE);
    }
    else {
        netif = gnrc_netif_iter(NULL);
    }

    if (netif == NULL) {
        DEBUG("gnrc_ipv4_static_addr: no interface found\n");
        return;
    }

    DEBUG("gnrc_ipv4_static_addr: interface %u selected\n", netif->pid);

    _config_addr(netif);
    _config_default_router(netif);
}

/** @} */
