/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @{
 *
 * @file
 * @brief   Internal DHCPv4 client definitions, shared between client.c and
 *          client_dns.c
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include "net/ipv4/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Length of the send and receive buffer
 *
 * Large enough for the fixed DHCPv4 header plus every option this client
 * builds or is prepared to parse (classless static routes in particular can
 * be sizable when a server offers many subnets).
 */
#ifndef DHCPV4_CLIENT_BUFLEN
#define DHCPV4_CLIENT_BUFLEN        (548)
#endif

/**
 * @brief   Configures @ref sock_dns_server from a DHCPv4-provided DNS server
 *
 * Stack independent: only touches `sock_dns_server`, guarded by module
 * `sock_dns`. A no-op if that module is not used.
 *
 * @param[in] dns   The DNS server address.
 * @param[in] netif The network interface the lease was acquired on.
 */
void dhcpv4_client_dns_conf(const ipv4_addr_t *dns, unsigned netif);

#ifdef __cplusplus
}
#endif

/** @} */
