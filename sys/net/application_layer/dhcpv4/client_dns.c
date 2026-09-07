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

#include <string.h>

#include "kernel_defines.h"
#include "net/dhcpv4/client.h"
#if IS_USED(MODULE_SOCK_DNS)
#include "net/sock/dns.h"
#endif

#include "_dhcpv4.h"

#define ENABLE_DEBUG 0
#include "debug.h"

void dhcpv4_client_dns_conf(const ipv4_addr_t *dns, unsigned netif)
{
#if IS_USED(MODULE_SOCK_DNS) && IS_ACTIVE(SOCK_HAS_IPV4)
    DEBUG("dhcpv4_client: overriding sock_dns_server\n");
    sock_dns_server.port = SOCK_DNS_PORT;
    sock_dns_server.family = AF_INET;
    sock_dns_server.netif = netif;
    memcpy(sock_dns_server.addr.ipv4, dns->u8, sizeof(sock_dns_server.addr.ipv4));
#else
    (void)dns;
    (void)netif;
#endif
}

/** @} */
