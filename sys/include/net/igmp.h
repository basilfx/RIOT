/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_igmp    IGMPv2
 * @ingroup     net_ipv4
 * @brief       Provides types related to IGMPv2
 * @see         <a href="https://tools.ietf.org/html/rfc2236">
 *                  RFC 2236
 *              </a>
 * @{
 *
 * @file
 * @brief   IGMPv2 type and function definitions
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <stdint.h>

#include "byteorder.h"
#include "net/ipv4/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @{
 * @name Message types
 * @see <a href="https://tools.ietf.org/html/rfc2236#section-2">
 *          RFC 2236, section 2
 *      </a>
 */
#define IGMP_MEMBERSHIP_QUERY       (0x11)  /**< Membership query */
#define IGMP_V1_MEMBERSHIP_REPORT   (0x12)  /**< Version 1 membership report */
#define IGMP_V2_MEMBERSHIP_REPORT   (0x16)  /**< Version 2 membership report */
#define IGMP_V2_LEAVE_GROUP         (0x17)  /**< Leave group */
/**
 * @}
 */

/**
 * @brief   General IGMPv2 message format.
 *
 * @see <a href="https://tools.ietf.org/html/rfc2236#section-2">
 *          RFC 2236, section 2
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t type;               /**< message type */
    uint8_t max_resp_time;      /**< max response time, in units of 1/10 s
                                *   (queries only; 0 in reports/leaves) */
    network_uint16_t csum;      /**< checksum */
    ipv4_addr_t group_addr;     /**< group address (0.0.0.0 in a general
                                 *   query) */
} igmp_hdr_t;

#ifdef __cplusplus
}
#endif

/** @} */
