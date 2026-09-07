/*
 * SPDX-FileCopyrightText: 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_gnrc_ipv4 IPv4
 * @ingroup     net_gnrc
 * @brief       GNRC's IPv4 implementation
 *
 * This module is for usage with the @ref net_gnrc_netapi
 *
 * GNRC IPv4 is host-only: it never forwards a packet between interfaces.
 * A packet not addressed to this host is simply dropped.
 *
 * # Supported NETAPI commands
 *
 * This module handles the following @ref net_gnrc_netapi message types:
 *
 * ## `GNRC_NETAPI_MSG_TYPE_RCV`
 *
 * @ref GNRC_NETAPI_MSG_TYPE_RCV expects a @ref net_gnrc_pkt (referred to as
 * "packet" in the following) in receive order (payload first, headers ordered
 * down the stack). It must at least contain a link-layer payload starting
 * with a valid @ref net_ipv4_hdr.
 *
 * If the destination address is an address on this host, and a suitable
 * upper layer is registered the packet will be forwarded to that upper
 * layer. An upper layer is suitable with its registration if
 *
 *   - it is registered with a tuple (@ref GNRC_NETTYPE_IPV4, `protocol`), or
 *   - it is registered with a tuple (@ref gnrc_nettype_from_protnum(`protocol`),
 *     @ref GNRC_NETREG_DEMUX_CTX_ALL).
 *
 * In both cases `protocol` is the [protocol field of the IPv4 header]
 * (@ref ipv4_hdr_t::protocol). Any other packet is dropped.
 *
 * ## `GNRC_NETAPI_MSG_TYPE_SND`
 *
 * @ref GNRC_NETAPI_MSG_TYPE_SND expects a @ref net_gnrc_pkt (referred to as
 * "packet" in the following) in send order (headers ordered up the stack,
 * payload last). It must at least contain a snip of type
 * @ref GNRC_NETTYPE_IPV4 as its first or second snip. If the first snip is
 * not of type @ref GNRC_NETTYPE_IPV4, it must be of type
 * @ref GNRC_NETTYPE_NETIF.
 *
 * If the destination address within the @ref GNRC_NETTYPE_IPV4 snip is an
 * address on this host, the packet will be [reversed]
 * (@ref gnrc_pktbuf_reverse_snips()) and [merged](@ref gnrc_pktbuf_merge())
 * so that it has a format as though it came from a network interface. It
 * will then be handled as a received packet ("looped back") by the IPv4
 * module (see previous section).
 *
 * Otherwise, the IPv4 header will be filled for fields that were not set by
 * upper layers, the next hop (the destination itself if on-link, otherwise a
 * gateway from @ref net_gnrc_ipv4_ft) is resolved to a link-layer address via
 * @ref net_gnrc_ipv4_arp, and the packet is handed over to the link layer
 * once that address is known.
 *
 * ## `GNRC_NETAPI_MSG_TYPE_SET`
 *
 * `GNRC_NETAPI_MSG_TYPE_SET` is not supported.
 *
 * ## `GNRC_NETAPI_MSG_TYPE_GET`
 *
 * `GNRC_NETAPI_MSG_TYPE_GET` is not supported.
 *
 * @{
 *
 * @file
 * @brief       Definitions for GNRC's IPv4 implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#include "sched.h"
#include "thread.h"

#include "net/gnrc/ipv4/arp.h"
#include "net/gnrc/ipv4/ft.h"
#include "net/gnrc/ipv4/hdr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup    net_gnrc_ipv4_conf  GNRC IPv4 compile configurations
 * @ingroup     net_gnrc_ipv4
 * @ingroup     net_gnrc_conf
 * @{
 */
/**
 * @brief   Default stack size to use for the IPv4 thread
 */
#ifndef GNRC_IPV4_STACK_SIZE
#define GNRC_IPV4_STACK_SIZE        ((THREAD_STACKSIZE_DEFAULT) - 64)
#endif

/**
 * @brief   Default priority for the IPv4 thread
 */
#ifndef GNRC_IPV4_PRIO
#define GNRC_IPV4_PRIO              (THREAD_PRIORITY_MAIN - 3)
#endif

/**
 * @brief   Default message queue size to use for the IPv4 thread (as
 *          exponent of 2^n).
 */
#ifndef CONFIG_GNRC_IPV4_MSG_QUEUE_SIZE_EXP
#define CONFIG_GNRC_IPV4_MSG_QUEUE_SIZE_EXP    (3U)
#endif

/**
 * @brief   Message queue size to use for the IPv4 thread.
 */
#ifndef GNRC_IPV4_MSG_QUEUE_SIZE
#define GNRC_IPV4_MSG_QUEUE_SIZE    (1 << CONFIG_GNRC_IPV4_MSG_QUEUE_SIZE_EXP)
#endif
/** @} */

/**
 * @brief   The PID to the IPv4 thread.
 *
 * @note    Use @ref gnrc_ipv4_init() to initialize. **Do not set by hand**.
 *
 * @details This variable is preferred for IPv4 internal communication *only*.
 *          Please use @ref net_gnrc_netreg for external communication.
 */
extern kernel_pid_t gnrc_ipv4_pid;

/**
 * @brief   Initialization of the IPv4 thread.
 *
 * @return  The PID to the IPv4 thread, on success.
 * @return  a negative errno on error.
 * @return  -EOVERFLOW, if there are too many threads running already
 * @return  -EEXIST, if IPv4 was already initialized.
 */
kernel_pid_t gnrc_ipv4_init(void);

/**
 * @brief   Get the IPv4 header from a given list of @ref gnrc_pktsnip_t
 *
 * @param[in] pkt    The pointer to the first @ref gnrc_pktsnip_t of the
 *                   packet.
 *
 * @return A pointer to the @ref ipv4_hdr_t of the packet.
 * @return NULL if the packet does not contain an IPv4 header.
 */
ipv4_hdr_t *gnrc_ipv4_get_header(gnrc_pktsnip_t *pkt);

#ifdef __cplusplus
}
#endif

/** @} */
