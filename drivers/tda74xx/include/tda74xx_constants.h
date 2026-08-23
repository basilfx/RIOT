/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     drivers_tda74xx
 * @{
 * @file
 * @brief       Internal addresses, registers and constants
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name TDA74xx registers
 *
 * The datasheets refer to these registers as subaddresses. The registers of
 * the bass and middle filters differ per device, and are therefore part of
 * @ref tda74xx_info_t.
 * @{
 */
#define TDA74XX_REG_INPUT_SELECT    (0x00)  /**< Register for input selection */
#define TDA74XX_REG_INPUT_GAIN      (0x01)  /**< Register for input gain */
#define TDA74XX_REG_VOLUME          (0x02)  /**< Register for volume */
#define TDA74XX_REG_TREBLE          (0x05)  /**< Register for treble gain */
#define TDA74XX_REG_SPEAKER_RIGHT   (0x06)  /**< Register for right speaker attenuation */
#define TDA74XX_REG_SPEAKER_LEFT    (0x07)  /**< Register for left speaker attenuation */
/** @} */

/**
 * @brief   Register flag to auto increment the register after each written
 *          byte
 */
#define TDA74XX_REG_INCREMENT       (0x10)

/**
 * @name TDA74xx register values
 * @{
 */
#define TDA74XX_VOLUME_MUTE         (0x38)  /**< Value to mute the volume stage */
#define TDA74XX_TONE_FLAT           (0x07)  /**< Value for a tone gain of 0 dB */
#define TDA74XX_TONE_BOOST          (0x08)  /**< Value bit to boost instead of cut */
/** @} */

#ifdef __cplusplus
}
#endif

/** @} */
