/*
 * SPDX-FileCopyrightText: 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_gnrc_ipv4_hdr IPv4 header definitions
 * @ingroup     net_gnrc_ipv4
 * @{
 *
 * @file
 * @brief   IPv4 header
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <stdint.h>

#include "net/gnrc/pkt.h"
#include "net/ipv4/hdr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Builds an IPv4 header for sending and adds it to the packet
 *          buffer.
 *
 * @details Initializes version to 4, IHL to 5 (20 bytes, no options), type of
 *          service, identification, flags and fragment offset with 0, TTL
 *          with 0, next protocol with @ref PROTNUM_RESERVED, and checksum
 *          with 0.
 *
 * @param[in] payload   Payload for the packet.
 * @param[in] src       Source address for the header. Can be NULL if not
 *                      known or required.
 * @param[in] dst       Destination address for the header. Can be NULL if not
 *                      known or required.
 *
 * @return  The IPv4 header in packet buffer on success.
 * @return  NULL on error.
 */
gnrc_pktsnip_t *gnrc_ipv4_hdr_build(gnrc_pktsnip_t *payload,
                                   const ipv4_addr_t *src,
                                   const ipv4_addr_t *dst);

#ifdef __cplusplus
}
#endif

/** @} */
