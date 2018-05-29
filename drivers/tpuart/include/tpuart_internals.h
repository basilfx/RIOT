/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_tpuart
 *
 * @{
 * @file
 * @brief       Internal addresses, registers, constants for the TPUART
 *              transceiver.
 *
 * @see         http://www.hqs.sbt.siemens.com/cps_product_data/gamma-b2b/tpuart.pdf
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef TPUART_INTERNALS_H
#define TPUART_INTERNALS_H

#include "timex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name TPUART timeout values
 * @{
 */
#define TPUART_TIMEOUT_END_OF_TELEGRAM      (2.5 * US_PER_MS)
#define TPUART_TIMEOUT_WAIT_FOR_ACK         (5.0 * US_PER_MS)
#define TPUART_TIMEOUT_WAIT_FOR_RESPONSE    (100 * US_PER_MS)
/** @} */

/**
 * @name Services provided/supported by TPUART
 * @{
 */
#define TPUART_SERVICE_L_DATA_CONFIRM       (0x0b)
#define TPUART_SERVICE_L_DATA_REQUEST       (0x90)
#define TPUART_SERVICE_L_EXT_DATA_REQUEST   (0x10)
#define TPUART_SERVICE_L_POLLDATA_REQUEST   (0xF0)
#define TPUART_SERVICE_U_ACTIVATE_BUSMON    (0x05)
#define TPUART_SERVICE_U_ACTIVATE_BUSY_MODE (0x21)
#define TPUART_SERVICE_U_ACTIVATE_CRC       (0x25)
#define TPUART_SERVICE_U_L_DATA_CONTINUE    (0x80)
#define TPUART_SERVICE_U_L_DATA_END         (0x40)
#define TPUART_SERVICE_U_L_DATA_START       (0x80)
#define TPUART_SERVICE_U_MX_RST_CNT         (0x24)
#define TPUART_SERVICE_U_PRODUCT_ID_REQUEST (0x20)
#define TPUART_SERVICE_U_RESET_BUSY_MODE    (0x22)
#define TPUART_SERVICE_U_RESET_REQUEST      (0x01)
#define TPUART_SERVICE_U_RESET_RESPONSE     (0x03)
#define TPUART_SERVICE_U_SET_ADDRESS        (0x28)
#define TPUART_SERVICE_U_STATE_REQUEST      (0x02)
#define TPUART_SERVICE_U_STATE_RESPONSE     (0x07)
/** @} */

/**
 * @brief Helper for matching any of the supported services
 */
#define TPUART_MATCHES(x, y)            ((x & y) == y)

#ifdef __cplusplus
}
#endif

#endif /* TPUART_INTERNALS_H */
/** @} */
