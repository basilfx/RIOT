/*
 * SPDX-FileCopyrightText: 2015 Martine Lenders <mlenders@inf.fu-berlin.de>
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    net_gnrc_icmpv4_echo  ICMPv4 echo messages
 * @ingroup     net_gnrc_icmpv4
 * @brief       ICMPv4 echo request and reply
 * @{
 *
 * @file
 * @brief   ICMPv4 echo message definitions
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <inttypes.h>

#include "byteorder.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/icmpv4.h"
#include "net/ipv4/hdr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Builds an ICMPv4 echo message of type @p type for sending.
 *
 * @param[in] type      Type of the echo message. Expected to be either
 *                      ICMPV4_ECHO_REQ or ICMPV4_ECHO_REP.
 * @param[in] id        ID for the echo message in host byte-order
 * @param[in] seq       Sequence number for the echo message in host byte-order
 * @param[in] data      Payload for the echo message
 * @param[in] data_len  Length of @p data
 *
 * @return  The echo message on success
 * @return  NULL, on failure
 */
gnrc_pktsnip_t *gnrc_icmpv4_echo_build(uint8_t type, uint16_t id, uint16_t seq,
                                      const void *data, size_t data_len);

/**
 * @brief   ICMPv4 echo request handler
 *
 * @param[in] netif     The interface the echo request was received on.
 * @param[in] ipv4_hdr  The IPv4 header of the echo request.
 * @param[in] echo      The Echo Request message.
 * @param[in] len       Length of the echo request message (ipv4_hdr_t::tl of
 *                      @p ipv4_hdr minus the IPv4 header length).
 */
void gnrc_icmpv4_echo_req_handle(gnrc_netif_t *netif, ipv4_hdr_t *ipv4_hdr,
                                 icmpv4_echo_t *echo, uint16_t len);

/**
 * @brief   Send out an ICMPv4 echo request
 *
 * @param[in] addr      The destination address of the echo request
 * @param[in] id        ID for the echo message in host byte-order
 * @param[in] seq       Sequence number for the echo message in host byte-order
 * @param[in] ttl       Time to live of the echo request. 0 lets gnrc_ipv4
 *                      select the interface's default.
 * @param[in] len       Length of the payload
 *
 * @return  0 on success
 * @return <0 on error
 */
int gnrc_icmpv4_echo_send(const ipv4_addr_t *addr, uint16_t id, uint16_t seq,
                          uint8_t ttl, size_t len);

/**
 * @brief   ICMPv4 echo response callback
 *
 * @param[in] pkt       Packet containing the ICMPv4 response
 * @param[in] corrupt   Offset of corrupt payload, -1 if no corruption detected
 * @param[in] rtt_us    round-trip-time in µs (0 if this information is not available)
 * @param[in] ctx       User supplied context
 *
 * @return  0 on success
 * @return <0 on error
 */
typedef int (*gnrc_icmpv4_echo_rsp_handle_cb_t)(gnrc_pktsnip_t *pkt,
                                                int corrupt, uint32_t rtt_us, void *ctx);

/**
 * @brief   Parse ICMPv4 echo response
 *
 * @param[in] pkt       Incoming ICMPv4 packet
 * @param[in] len       Expected echo response payload length
 * @param[in] cb        Callback function to execute
 * @param[in] ctx       Callback function context
 *
 * @return  0 on success
 * @return <0 on error
 */
int gnrc_icmpv4_echo_rsp_handle(gnrc_pktsnip_t *pkt, size_t len,
                                gnrc_icmpv4_echo_rsp_handle_cb_t cb, void *ctx);

#ifdef __cplusplus
}
#endif

/** @} */
