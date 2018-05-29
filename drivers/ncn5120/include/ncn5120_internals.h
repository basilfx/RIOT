/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ncn5120
 *
 * @{
 * @file
 * @brief       Internal addresses, registers, constants for the NCN5120
 *              transceiver.
 *
 * @see         http://www.onsemi.com/pub/Collateral/NCN5120-D.PDF
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NCN5120_INTERNALS_H
#define NCN5120_INTERNALS_H

#include "timex.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name NCN5120 timeout values
 * @{
 */
#define NCN5120_TIMEOUT_END_OF_TELEGRAM      (2.6 * US_PER_MS)
#define NCN5120_TIMEOUT_WAIT_FOR_ACK         (5.2 * US_PER_MS)
#define NCN5120_TIMEOUT_WAIT_FOR_RESPONSE    (100 * US_PER_MS)
/** @} */

/**
 * @name Services provided/supported by NCN5120
 * @{
 */
#define NCN5120_SERVICE_L_DATA_ACKN_INDICATION     (0x00)
#define NCN5120_SERVICE_L_DATA_CONFIRM             (0x0B)
#define NCN5120_SERVICE_L_DATA_EXTENDED_INDICATION (0x10)
#define NCN5120_SERVICE_L_DATA_STANDARD_INDICATION (0x90)
#define NCN5120_SERVICE_L_POLL_DATA_INDICATION     (0xF0)
#define NCN5120_SERVICE_U_ACKN_REQUEST             (0x10)
#define NCN5120_SERVICE_U_BUSMON_REQUEST           (0x05)
#define NCN5120_SERVICE_U_CONFIGURE_INDICATION     (0x01)
#define NCN5120_SERVICE_U_CONFIGURE_REQUEST        (0x18)
#define NCN5120_SERVICE_U_EXIT_STOP_MODE_REQUEST   (0x0F)
#define NCN5120_SERVICE_U_FRAME_END_INDICATION     (0xCB)
#define NCN5120_SERVICE_U_FRAME_STATE_INDICATION   (0x03)
#define NCN5120_SERVICE_U_INT_REG_RD_REQUEST       (0x38)
#define NCN5120_SERVICE_U_INT_REG_WR_REQUEST       (0x28)
#define NCN5120_SERVICE_U_L_DATA_CONTINUE_REQUEST  (0x80)
#define NCN5120_SERVICE_U_L_DATA_END_REQUEST       (0x40)
#define NCN5120_SERVICE_U_L_DATA_OFFSET_REQUEST    (0x08)
#define NCN5120_SERVICE_U_L_DATA_START_REQUEST     (0x80)
#define NCN5120_SERVICE_U_POLLING_STATE_REQUEST    (0xE0)
#define NCN5120_SERVICE_U_QUIT_BUSY_REQUEST        (0x04)
#define NCN5120_SERVICE_U_RESET_INDICATION         (0x03)
#define NCN5120_SERVICE_U_RESET_REQUEST            (0x01)
#define NCN5120_SERVICE_U_SET_ADDRESS_REQUEST      (0xF1)
#define NCN5120_SERVICE_U_SET_BUSY_REQUEST         (0x21)
#define NCN5120_SERVICE_U_SET_REPETITION_REQUEST   (0xF2)
#define NCN5120_SERVICE_U_STATE_INDICATION         (0x07)
#define NCN5120_SERVICE_U_STATE_REQUEST            (0x02)
#define NCN5120_SERVICE_U_STOP_MODE                (0x0E)
#define NCN5120_SERVICE_U_STOP_MODE_INDICATION     (0x2B)
#define NCN5120_SERVICE_U_SYSTEM_STATE_INDICATION  (0x4B)
#define NCN5120_SERVICE_U_SYSTEM_STATE_REQUEST     (0x0D)
/** @} */

/**
 * @brief Helper for matching any of the supported services
 */
#define NCN5120_MATCHES(x, y)           ((x & y) == y)

#ifdef __cplusplus
}
#endif

#endif /* NCN5120_INTERNALS_H */
/** @} */
