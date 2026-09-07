/*
 * SPDX-FileCopyrightText: 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_icmpv4  ICMPv4
 * @ingroup     net_ipv4
 * @brief       Provides types related to ICMPv4
 * @see         <a href="https://tools.ietf.org/html/rfc792">
 *                  RFC 792
 *              </a>
 * @{
 *
 * @file
 * @brief   ICMPv4 type and function definitions
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <stdint.h>

#include "byteorder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @{
 * @name Message types
 * @see <a href="https://www.iana.org/assignments/icmp-parameters/icmp-parameters.xhtml">
 *          IANA, ICMP Type Numbers
 *      </a>
 */
#define ICMPV4_ECHO_REP     (0)     /**< Echo reply message (pong) */
#define ICMPV4_DST_UNR      (3)     /**< Destination unreachable message */
#define ICMPV4_ECHO_REQ     (8)     /**< Echo request message (ping) */
#define ICMPV4_TIME_EXC     (11)    /**< Time exceeded message */
#define ICMPV4_PARAM_PROB   (12)    /**< Parameter problem message */
/**
 * @}
 */

/**
 * @{
 * @anchor net_icmpv4_error_dst_unr_codes
 * @name Codes for the destination unreachable message
 * @see <a href="https://tools.ietf.org/html/rfc792">
 *          RFC 792
 *      </a>
 */
#define ICMPV4_ERROR_DST_UNR_NET        (0) /**< net unreachable */
#define ICMPV4_ERROR_DST_UNR_HOST       (1) /**< host unreachable */
#define ICMPV4_ERROR_DST_UNR_PROTOCOL   (2) /**< protocol unreachable */
#define ICMPV4_ERROR_DST_UNR_PORT       (3) /**< port unreachable */
#define ICMPV4_ERROR_DST_UNR_FRAG       (4) /**< fragmentation needed and
                                             *   don't-fragment set */
#define ICMPV4_ERROR_DST_UNR_SRC_ROUTE_FAIL (5) /**< source route failed */
/**
 * @}
 */

/**
 * @{
 * @anchor net_icmpv4_error_time_exc_codes
 * @name Codes for the time exceeded message
 * @see <a href="https://tools.ietf.org/html/rfc792">
 *          RFC 792
 *      </a>
 */
#define ICMPV4_ERROR_TIME_EXC_TTL    (0) /**< time to live exceeded in
                                          *   transit */
#define ICMPV4_ERROR_TIME_EXC_FRAG   (1) /**< fragment reassembly time
                                          *   exceeded */
/**
 * @}
 */

/**
 * @{
 * @anchor net_icmpv4_error_param_prob_codes
 * @name Codes for the parameter problem message
 * @see <a href="https://tools.ietf.org/html/rfc792">
 *          RFC 792
 *      </a>
 */
#define ICMPV4_ERROR_PARAM_PROB_PTR  (0) /**< pointer indicates the error */
/**
 * @}
 */

/**
 * @brief   General ICMPv4 message format.
 *
 * @see <a href="https://tools.ietf.org/html/rfc792">
 *          RFC 792
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t type;           /**< message type */
    uint8_t code;           /**< message code */
    network_uint16_t csum;  /**< checksum */
} icmpv4_hdr_t;

/**
 * @brief   Echo request and reply message format.
 * @extends icmpv4_hdr_t
 *
 * @see <a href="https://tools.ietf.org/html/rfc792">
 *          RFC 792
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t type;           /**< message type */
    uint8_t code;           /**< message code */
    network_uint16_t csum;  /**< checksum */
    network_uint16_t id;    /**< identifier */
    network_uint16_t seq;   /**< sequence number */
} icmpv4_echo_t;

/**
 * @brief   Destination unreachable message format.
 * @extends icmpv4_hdr_t
 *
 * @see <a href="https://tools.ietf.org/html/rfc792">
 *          RFC 792
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t type;               /**< message type */
    uint8_t code;               /**< message code */
    network_uint16_t csum;      /**< checksum */
    network_uint32_t unused;    /**< unused field */
} icmpv4_error_dst_unr_t;

/**
 * @brief   Time exceeded message format.
 * @extends icmpv4_hdr_t
 *
 * @see <a href="https://tools.ietf.org/html/rfc792">
 *          RFC 792
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t type;               /**< message type */
    uint8_t code;               /**< message code */
    network_uint16_t csum;      /**< checksum */
    network_uint32_t unused;    /**< unused field */
} icmpv4_error_time_exc_t;

/**
 * @brief   Parameter problem message format.
 * @extends icmpv4_hdr_t
 *
 * @see <a href="https://tools.ietf.org/html/rfc792">
 *          RFC 792
 *      </a>
 */
typedef struct __attribute__((packed)) {
    uint8_t type;           /**< message type */
    uint8_t code;           /**< message code */
    network_uint16_t csum;  /**< checksum */
    uint8_t ptr;            /**< pointer to the erroneous octet, relative to
                             *   the start of the quoted datagram */
    uint8_t unused[3];      /**< unused field */
} icmpv4_error_param_prob_t;

#ifdef __cplusplus
}
#endif

/** @} */
