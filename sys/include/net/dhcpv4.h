/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_dhcpv4  Dynamic Host Configuration Protocol for IPv4 (DHCPv4)
 * @ingroup     net_ipv4
 * @brief       DHCPv4 definitions
 * @{
 *
 * @file
 * @brief   DHCPv4 message format, option codes and option helpers
 * @note    This header is based on
 *          [RFC 2131](https://tools.ietf.org/html/rfc2131) and
 *          [RFC 2132](https://tools.ietf.org/html/rfc2132)
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "byteorder.h"
#include "net/ipv4/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    DHCPv4 ports
 * @see [RFC 2131, section 4]
 *      (https://tools.ietf.org/html/rfc2131#section-4)
 * @{
 */
#define DHCPV4_SERVER_PORT          (67U)   /**< server port */
#define DHCPV4_CLIENT_PORT          (68U)   /**< client port */
/** @} */

/**
 * @name    DHCPv4 op codes
 * @see [RFC 2131, section 2]
 *      (https://tools.ietf.org/html/rfc2131#section-2)
 * @{
 */
#define DHCPV4_OP_BOOTREQUEST       (1U)    /**< client to server */
#define DHCPV4_OP_BOOTREPLY         (2U)    /**< server to client */
/** @} */

/**
 * @brief   Value of @ref dhcpv4_msg_t::magic_cookie
 */
#define DHCPV4_MAGIC_COOKIE         (0x63825363U)

/**
 * @brief   Flag in @ref dhcpv4_msg_t::flags requesting the server (and any
 *          relay agent) to broadcast the reply rather than unicast it
 *
 * A client that does not yet have its lease's address configured on its
 * interface cannot receive a unicast reply (@ref net_gnrc_ipv4 is host-only
 * and drops packets not addressed to a configured or broadcast address), so
 * this flag must be set on every message sent before the client has a
 * usable lease (i.e. everything up to and including the `REQUESTING`
 * state).
 */
#define DHCPV4_FLAG_BROADCAST       (0x8000U)

/**
 * @brief   General DHCPv4 message format
 *
 * @see [RFC 2131, section 2](https://tools.ietf.org/html/rfc2131#section-2)
 */
typedef struct __attribute__((packed)) {
    uint8_t op;                     /**< @ref DHCPV4_OP_BOOTREQUEST or
                                     *   @ref DHCPV4_OP_BOOTREPLY */
    uint8_t htype;                  /**< hardware address type, see
                                     *   @ref net_arp_hwtype */
    uint8_t hlen;                   /**< hardware address length */
    uint8_t hops;                   /**< client sets to 0, used by relay agents */
    network_uint32_t xid;           /**< transaction ID */
    network_uint16_t secs;          /**< seconds elapsed since client began the
                                     *   address acquisition or renewal process */
    network_uint16_t flags;         /**< @ref DHCPV4_FLAG_BROADCAST or 0 */
    ipv4_addr_t ciaddr;             /**< client IP address, filled in by the
                                     *   client only if it can respond to ARP
                                     *   requests for it */
    ipv4_addr_t yiaddr;             /**< "your" (client) IP address */
    ipv4_addr_t siaddr;             /**< IP address of next server to use in
                                     *   bootstrap */
    ipv4_addr_t giaddr;             /**< relay agent IP address */
    uint8_t chaddr[16];             /**< client hardware address */
    uint8_t sname[64];              /**< optional server host name */
    uint8_t file[128];              /**< boot file name */
    network_uint32_t magic_cookie;  /**< always @ref DHCPV4_MAGIC_COOKIE */
    /* options follow, see @ref dhcpv4_opt_get() and @ref dhcpv4_opt_add() */
} dhcpv4_msg_t;

/**
 * @brief   Size of @ref dhcpv4_msg_t, up to and including
 *          dhcpv4_msg_t::magic_cookie
 */
#define DHCPV4_MSG_MIN_SIZE         (sizeof(dhcpv4_msg_t))

/**
 * @name    DHCPv4 message types
 * @anchor  net_dhcpv4_msg_types
 * @see [RFC 2131, section 3](https://tools.ietf.org/html/rfc2131#section-3)
 * @{
 */
#define DHCPV4_MSG_DISCOVER         (1U)    /**< DHCPDISCOVER */
#define DHCPV4_MSG_OFFER            (2U)    /**< DHCPOFFER */
#define DHCPV4_MSG_REQUEST          (3U)    /**< DHCPREQUEST */
#define DHCPV4_MSG_DECLINE          (4U)    /**< DHCPDECLINE */
#define DHCPV4_MSG_ACK              (5U)    /**< DHCPACK */
#define DHCPV4_MSG_NAK              (6U)    /**< DHCPNAK */
#define DHCPV4_MSG_RELEASE          (7U)    /**< DHCPRELEASE */
#define DHCPV4_MSG_INFORM           (8U)    /**< DHCPINFORM */
/** @} */

/**
 * @name    DHCPv4 option codes
 * @anchor  net_dhcpv4_opt_codes
 * @see [RFC 2132](https://tools.ietf.org/html/rfc2132)
 * @{
 */
#define DHCPV4_OPT_PAD              (0U)    /**< padding, no length/value */
#define DHCPV4_OPT_SUBNET_MASK      (1U)    /**< subnet mask, 4 bytes */
#define DHCPV4_OPT_ROUTER           (3U)    /**< router list, N * 4 bytes */
#define DHCPV4_OPT_DNS_SERVER       (6U)    /**< DNS server list, N * 4 bytes */
#define DHCPV4_OPT_HOST_NAME        (12U)   /**< host name, string */
#define DHCPV4_OPT_REQUESTED_IP     (50U)   /**< requested IP address,
                                             *   4 bytes */
#define DHCPV4_OPT_LEASE_TIME       (51U)   /**< IP address lease time, in
                                             *   seconds, 4 bytes */
#define DHCPV4_OPT_MSG_TYPE         (53U)   /**< DHCP message type, see
                                             *   @ref net_dhcpv4_msg_types,
                                             *   1 byte */
#define DHCPV4_OPT_SERVER_ID        (54U)   /**< server identifier, 4 bytes */
#define DHCPV4_OPT_PARAM_REQ_LIST   (55U)   /**< parameter request list,
                                             *   N * 1 byte */
#define DHCPV4_OPT_MAX_MSG_SIZE     (57U)   /**< maximum DHCP message size,
                                             *   2 bytes */
#define DHCPV4_OPT_RENEWAL_TIME     (58U)   /**< renewal (T1) time value, in
                                             *   seconds, 4 bytes */
#define DHCPV4_OPT_REBINDING_TIME   (59U)   /**< rebinding (T2) time value, in
                                             *   seconds, 4 bytes */
#define DHCPV4_OPT_CLIENT_ID        (61U)   /**< client identifier */
#define DHCPV4_OPT_CLASSLESS_ROUTES (121U)  /**< classless static routes, see
                                             *   [RFC 3442]
                                             *   (https://tools.ietf.org/html/rfc3442) */
#define DHCPV4_OPT_END              (255U)  /**< end of options, no
                                             *   length/value */
/** @} */

/**
 * @brief   Looks up the first occurrence of option @p type in a DHCPv4
 *          options buffer
 *
 * @pre `(opts != NULL) || (opts_len == 0)`
 * @pre `len != NULL`
 *
 * @param[in] opts      Start of the options buffer (i.e. directly after
 *                      dhcpv4_msg_t::magic_cookie).
 * @param[in] opts_len  Length of @p opts in bytes.
 * @param[in] type      The option code to look up.
 * @param[out] len      Length of the returned option's value.
 *
 * @return  Pointer to the value of the first option of @p type.
 * @return  NULL, if no such option was found or the options buffer is
 *          malformed.
 */
static inline const uint8_t *dhcpv4_opt_get(const uint8_t *opts,
                                            size_t opts_len,
                                            uint8_t type, uint8_t *len)
{
    size_t i = 0;

    while (i < opts_len) {
        uint8_t opt_type = opts[i++];

        if (opt_type == DHCPV4_OPT_PAD) {
            continue;
        }
        if (opt_type == DHCPV4_OPT_END) {
            break;
        }
        if (i >= opts_len) {
            break;
        }
        uint8_t opt_len = opts[i++];

        if ((i + opt_len) > opts_len) {
            break;
        }
        if (opt_type == type) {
            *len = opt_len;
            return &opts[i];
        }
        i += opt_len;
    }
    return NULL;
}

/**
 * @brief   Appends an option to a DHCPv4 options buffer
 *
 * @pre `(buf != NULL) && ((data != NULL) || (len == 0))`
 *
 * @param[out] buf  Where to append the option. Must have at least
 *                  `len + 2` bytes of space.
 * @param[in] type  The option code.
 * @param[in] data  The option value. May be NULL if @p len is 0.
 * @param[in] len   Length of @p data in bytes.
 *
 * @return  Number of bytes written to @p buf, i.e. `len + 2`.
 */
static inline size_t dhcpv4_opt_add(uint8_t *buf, uint8_t type,
                                    const void *data, uint8_t len)
{
    buf[0] = type;
    buf[1] = len;
    if (len > 0) {
        memcpy(&buf[2], data, len);
    }
    return (size_t)len + 2;
}

#ifdef __cplusplus
}
#endif

/** @} */
