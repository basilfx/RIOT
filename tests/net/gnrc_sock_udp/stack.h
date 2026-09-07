/*
 * SPDX-FileCopyrightText: 2016 Freie Universität Berlin
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     tests
 * @brief
 * @{
 *
 * @file
 * @brief
 *
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 * @}
 */

#include <stdbool.h>
#include <stdint.h>

#include "kernel_defines.h"
#include "net/ipv6/addr.h"

#if IS_USED(MODULE_GNRC_IPV4)
#include "net/ipv4/addr.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initializes networking for tests
 */
void _net_init(void);

/**
 * @brief   Does what ever preparations are needed to check the packets sent
 */
void _prepare_send_checks(void);

/**
 * @brief   Auxiliary data to inject
 */
typedef struct {
    uint64_t timestamp; /**< Timestamp of reception */
    int16_t rssi;       /**< Fake RSSI value */
} inject_aux_t;

/**
 * @brief   Injects a received UDP packet into the stack
 *
 * @param[in] src       The source address of the UDP packet
 * @param[in] dst       The destination address of the UDP packet
 * @param[in] src_port  The source port of the UDP packet
 * @param[in] dst_port  The destination port of the UDP packet
 * @param[in] data      The payload of the UDP packet
 * @param[in] data_len  The payload length of the UDP packet
 * @param[in] netif     The interface the packet came over
 *
 * @return  true, if packet was successfully injected
 * @return  false, if an error occurred during injection
 */
bool _inject_packet_aux(const ipv6_addr_t *src, const ipv6_addr_t *dst,
                        uint16_t src_port, uint16_t dst_port,
                        void *data, size_t data_len, uint16_t netif,
                        const inject_aux_t *aux);

/**
 * @brief   Injects a received UDP packet into the stack
 *
 * @param[in] src       The source address of the UDP packet
 * @param[in] dst       The destination address of the UDP packet
 * @param[in] src_port  The source port of the UDP packet
 * @param[in] dst_port  The destination port of the UDP packet
 * @param[in] data      The payload of the UDP packet
 * @param[in] data_len  The payload length of the UDP packet
 * @param[in] netif     The interface the packet came over
 *
 * @return  true, if packet was successfully injected
 * @return  false, if an error occurred during injection
 */
static inline bool _inject_packet(const ipv6_addr_t *src,
                                  const ipv6_addr_t *dst,
                                  uint16_t src_port, uint16_t dst_port,
                                  void *data, size_t data_len, uint16_t netif)
{
    return _inject_packet_aux(src, dst, src_port, dst_port, data, data_len,
                              netif, NULL);
}

/**
 * @brief   Checks networking state (e.g. packet buffer state)
 *
 * @return  true, if networking component is still in valid state
 * @return  false, if networking component is in an invalid state
 */
bool _check_net(void);

/**
 * @brief   Checks if a UDP packet was sent by the networking component
 *
 * @param[in] src               Expected source address of the UDP packet
 * @param[in] dst               Expected destination address of the UDP packet
 * @param[in] src_port          Expected source port of the UDP packet
 * @param[in] dst_port          Expected destination port of the UDP packet
 * @param[in] data              Expected payload of the UDP packet
 * @param[in] data_len          Expected payload length of the UDP packet
 * @param[in] netif             Expected interface the packet is supposed to
 *                              be send over
 * @param[in] random_src_port   Do not check source port, it might be random
 *
 * @return  true, if all parameters match as expected
 * @return  false, if not.
 */
bool _check_packet(const ipv6_addr_t *src, const ipv6_addr_t *dst,
                   uint16_t src_port, uint16_t dst_port,
                   void *data, size_t data_len, uint16_t netif,
                   bool random_src_port);

#if IS_USED(MODULE_GNRC_IPV4)
/**
 * @brief   Injects a received IPv4 UDP packet into the stack
 *
 * @see _inject_packet_aux
 */
bool _inject_packet4_aux(const ipv4_addr_t *src, const ipv4_addr_t *dst,
                         uint16_t src_port, uint16_t dst_port,
                         void *data, size_t data_len, uint16_t netif,
                         const inject_aux_t *aux);

/**
 * @brief   Injects a received IPv4 UDP packet into the stack
 *
 * @see _inject_packet
 */
static inline bool _inject_packet4(const ipv4_addr_t *src,
                                   const ipv4_addr_t *dst,
                                   uint16_t src_port, uint16_t dst_port,
                                   void *data, size_t data_len, uint16_t netif)
{
    return _inject_packet4_aux(src, dst, src_port, dst_port, data, data_len,
                               netif, NULL);
}

/**
 * @brief   Checks if an IPv4 UDP packet was sent by the networking component
 *
 * @see _check_packet
 */
bool _check_packet4(const ipv4_addr_t *src, const ipv4_addr_t *dst,
                    uint16_t src_port, uint16_t dst_port,
                    void *data, size_t data_len, uint16_t netif,
                    bool random_src_port);
#endif /* IS_USED(MODULE_GNRC_IPV4) */

#ifdef __cplusplus
}
#endif
