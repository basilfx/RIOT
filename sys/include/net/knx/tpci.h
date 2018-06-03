/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_knx_tpci    KNX TPCI services
 * @ingroup     net_knx
 * @brief       KNX TPCI services
 *
 * @{
 *
 * @file
 * @brief       Definitions for KNX TPCI services
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NET_KNX_TPCI_H
#define NET_KNX_TPCI_H

#include <stdint.h>

#include "net/knx/telegram.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   KNX TPCI services
 */
typedef enum {
    KNX_TPCI_UDP    = 0x00, /**< Unnumbered data packet */
    KNX_TPCI_NDP    = 0x01, /**< Numbered data packet */
    KNX_TPCI_UCD    = 0x02, /**< Unnumbered control data */
    KNX_TPCI_NCD    = 0x03, /**< Numbered control data */
} knx_tpci_t;

/**
 * @brief   KNX TPCI unnumbered control data (UCD) services
 */
typedef enum {
    KNX_TPCI_UCD_CONNECT    = 0x00,     /**< Connect request */
    KNX_TPCI_UCD_DISCONNECT = 0x01,     /**< Disconnect request */
} knx_tpci_ucd_t;

/**
 * @brief   KNX TPCI numbered control data (NCD) services
 */
typedef enum {
    KNX_TPCI_NCD_ACK    = 0x02,         /**< Acknowledged */
    KNX_TPCI_NCD_NACK   = 0x03,         /**< Not acknowledged */
} knx_tpci_ncd_t;

/**
 * @brief   Get the TPCI service of a telegram
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  A @p knx_tpci_t service
 */
knx_tpci_t knx_tpci_get(const knx_telegram_t *telegram);

/**
 * @brief   Set the TPCI service of a telegram
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] tpci          The TPCI service to set
 */
void knx_tpci_set(knx_telegram_t *telegram, knx_tpci_t tpci);

/**
 * @brief   Get the UCD type of a TPCI telegram
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  A @p knx_tpci_ucd_t service.
 */
knx_tpci_ucd_t knx_tpci_ucd_get(const knx_telegram_t *telegram);

/**
 * @brief   Set the UCD type of a TPCI telegram
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] ucd           The TPCI UCD service to set
 */
void knx_tpci_ucd_set(knx_telegram_t *telegram, knx_tpci_ucd_t ucd);

/**
 * @brief   Get the NCD type of a telegram
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  A @p knx_tpci_ncd_t service
 */
knx_tpci_ncd_t knx_tpci_ncd_get(const knx_telegram_t *telegram);

/**
 * @brief   Set the NCD type of a telegram
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] ncd           The TPCI NCD service to set
 */
void knx_tpci_ncd_set(knx_telegram_t *telegram, knx_tpci_ncd_t ncd);

/**
 * @brief   Get the sequence number of a NCD or NDP type packet
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  The sequence number
 */
uint8_t knx_tpci_get_seq_number(const knx_telegram_t *telegram);

/**
 * @brief   Set the sequence number of a NCD or NDP type packet
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] seq           The sequence number to set
 */
void knx_tpci_set_seq_number(knx_telegram_t *telegram, uint8_t seq);

#ifdef __cplusplus
}
#endif

#endif /* NET_KNX_TPCI_H */
/** @} */
