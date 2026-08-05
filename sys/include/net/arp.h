/*
 * SPDX-FileCopyrightText: 2018 Freie Universität Berlin
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_arp    Address resolution protocol (ARP)
 * @ingroup     net_ipv4
 * @brief       ARP definitions
 * @{
 *
 * @file
 * @brief   ARP definitions
 *
 * @author  Martine Lenders <m.lenders@fu-berlin.de>
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <stdint.h>

#include "byteorder.h"
#include "net/ethernet/hdr.h"
#include "net/ipv4/addr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Operation codes
 * @anchor  net_arp_opcode
 * @see     [IANA ARP parameters]
 *          (https://www.iana.org/assignments/arp-parameters#arp-parameters-1)
 * @{
 */
#define ARP_OPCODE_REQUEST  (1U)        /**< ARP request */
#define ARP_OPCODE_REPLY    (2U)        /**< ARP reply */
/** @} */

/**
 * @name    Hardware types
 * @anchor  net_arp_hwtype
 * @see     [IANA ARP parameters]
 *          (https://www.iana.org/assignments/arp-parameters#arp-parameters-2)
 * @{
 */
#define ARP_HWTYPE_ETHERNET (1U)        /**< Ethernet */
#define ARP_HWTYPE_SERIAL   (20U)       /**< Serial Line */
#define ARP_HWTYPE_EUI64    (27U)       /**< EUI-64 */
/** @} */

/**
 * @name    Protocol types
 * @anchor  net_arp_prototype
 * @brief   Values of @ref arp_hdr_t::pro, taken from the EtherType number
 *          space.
 * @see     [IANA ARP parameters]
 *          (https://www.iana.org/assignments/arp-parameters#arp-parameters-3)
 * @{
 */
#define ARP_PROTOTYPE_IPV4  (0x0800U)   /**< IPv4 */
/** @} */

/**
 * @brief   The fixed-size fields common to every ARP packet, independent of
 *          hardware and protocol address length.
 *
 * @see     [RFC 826](https://tools.ietf.org/html/rfc826)
 */
typedef struct __attribute__((packed)) {
    network_uint16_t hrd;   /**< hardware type, see @ref net_arp_hwtype */
    network_uint16_t pro;   /**< protocol type, see @ref net_arp_prototype */
    uint8_t hln;            /**< hardware address length */
    uint8_t pln;            /**< protocol address length */
    network_uint16_t op;    /**< operation, see @ref net_arp_opcode */
} arp_hdr_t;

/**
 * @brief   An ARP packet for the combination of Ethernet hardware addresses
 *          and IPv4 protocol addresses.
 *
 * @see     [RFC 826](https://tools.ietf.org/html/rfc826)
 */
typedef struct __attribute__((packed)) {
    arp_hdr_t hdr;                      /**< fixed-size header */
    uint8_t sha[ETHERNET_ADDR_LEN];     /**< sender hardware address */
    ipv4_addr_t spa;                    /**< sender protocol address */
    uint8_t tha[ETHERNET_ADDR_LEN];     /**< target hardware address */
    ipv4_addr_t tpa;                    /**< target protocol address */
} arp_ipv4_hdr_t;

#ifdef __cplusplus
}
#endif

/** @} */
