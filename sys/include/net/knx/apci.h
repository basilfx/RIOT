/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_knx_apci    KNX APCI services
 * @ingroup     net_knx
 * @brief       KNX APCI services
 *
 * @{
 *
 * @file
 * @brief       Definitions for KNX APCI services
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NET_KNX_APCI_H
#define NET_KNX_APCI_H

#include <stdint.h>

#include "net/knx/telegram.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   KNX APCI services
 *
 * This list is complete.
 */
typedef enum {
    KNX_APCI_GROUP_VALUE_READ           = 0x00,
    KNX_APCI_GROUP_VALUE_RESPONSE       = 0x01,
    KNX_APCI_GROUP_VALUE_WRITE          = 0x02,
    KNX_APCI_INDIVIDUAL_ADDR_WRITE      = 0x03,
    KNX_APCI_INDIVIDUAL_ADDR_READ       = 0x04,
    KNX_APCI_INDIVIDUAL_ADDR_RESPONSE   = 0x05,
    KNX_APCI_ADC_READ                   = 0x06,
    KNX_APCI_ADC_RESPONSE               = 0x07,
    KNX_APCI_MEMORY_READ                = 0x08,
    KNX_APCI_MEMORY_RESPONSE            = 0x09,
    KNX_APCI_MEMORY_WRITE               = 0x0a,
    KNX_APCI_USER_MESSAGE               = 0x0b,
    KNX_APCI_MASK_VERSION_READ          = 0x0c,
    KNX_APCI_MASK_VERSION_RESPONSE      = 0x0d,
    KNX_APCI_RESTART                    = 0x0e,
    KNX_APCI_ESCAPE                     = 0x0f,
} knx_apci_t;

/**
 * @brief   KNX extended APCI services
 *
 * This list is incomplete, and may be extended if needed.
 */
typedef enum {
    KNX_APCI_EXTENDED_DEVICE_DESCRIPTOR_READ            = 0x0300,
    KNX_APCI_EXTENDED_DEVICE_DESCRIPTOR_RESPONSE        = 0x0340,
    KNX_APCI_EXTENDED_AUTHORIZE_REQUEST                 = 0x03d1,
    KNX_APCI_EXTENDED_AUTHORIZE_RESPONSE                = 0x03d2,
    KNX_APCI_EXTENDED_PROPERTY_VALUE_READ               = 0x03d5,
    KNX_APCI_EXTENDED_PROPERTY_VALUE_RESPONSE           = 0x03d6,
    KNX_APCI_EXTENDED_PROPERTY_VALUE_WRITE              = 0x03d7,
    KNX_APCI_EXTENDED_PROPERTY_DESCRIPTION_READ         = 0x03d8,
    KNX_APCI_EXTENDED_PROPERTY_DESCRIPTION_RESPONSE     = 0x03d9,
    KNX_APCI_EXTENDED_INDIVIDUAL_ADDR_SERIAL_READ       = 0x03dc,
    KNX_APCI_EXTENDED_INDIVIDUAL_ADDR_SERIAL_RESPONSE   = 0x03dd,
    KNX_APCI_EXTENDED_INDIVIDUAL_ADDR_SERIAL_WRITE      = 0x03de,
} knx_apci_extended_t;

/**
 * @brief   Return the APCI service of a UDP or NDP telegram
 *
 * If @p KNX_APCI_ESCAPE is returned, use @p knx_apci_extended_get to get the
 * extended APCI services.
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  An @p knx_apci_t service.
 */
knx_apci_t knx_apci_get(const knx_telegram_t *telegram);

/**
 * @brief   Set the APCI service of a UDP or NDP telegram
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] apci          The APCI service to set
 */
void knx_apci_set(knx_telegram_t *telegram, knx_apci_t apci);

/**
 * @brief   Return the extended APCI service of a UDP or NDP telegram
 *
 * @note    Only applies to a telegram that has APCI service
 *          @p KNX_APCI_ESCAPE.
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  A @p knx_apci_t service.
 */
knx_apci_extended_t knx_apci_extended_get(const knx_telegram_t *telegram);

/**
 * @brief   Set the extended APCI service of a UDP or NDP telegram
 *
 * @note    Only applies to a telegram that has APCI service
 *          @p KNX_APCI_ESCAPE.
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] apci          The extended APCI service to set
 */
void knx_apci_extended_set(knx_telegram_t *telegram, knx_apci_extended_t apci);

#ifdef __cplusplus
}
#endif

#endif /* NET_KNX_APCI_H */
/** @} */
