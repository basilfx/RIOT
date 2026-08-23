/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     drivers_tda74xx
 * @{
 *
 * @file
 * @brief       Definitions of the ST TDA74xx audio processors
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    TDA74xx audio processor information
 *
 * The register of the bass filter differs per device. The register of the
 * middle filter, if the device has one, follows it.
 * @{
 */
#define TDA74XX_INFO_INPUTS_DUAL    (2)     /**< Two stereo inputs */
#define TDA74XX_INFO_INPUTS_QUAD    (4)     /**< Four stereo inputs */

#define TDA74XX_INFO_BANDS_TWO      (2)     /**< Bass and treble control */
#define TDA74XX_INFO_BANDS_THREE    (3)     /**< Bass, middle and treble control */

#define TDA74XX_INFO_REG_BASS_LOW   (0x03)  /**< Bass filter at register 0x03 */
#define TDA74XX_INFO_REG_BASS_HIGH  (0x04)  /**< Bass filter at register 0x04 */
/** @} */

/**
 * @name    TDA7439 info
 * @{
 */
#define TDA7439_INFO_INPUTS         TDA74XX_INFO_INPUTS_QUAD    /**< Number of inputs */
#define TDA7439_INFO_BANDS          TDA74XX_INFO_BANDS_THREE    /**< Number of tone bands */
#define TDA7439_INFO_REG_BASS       TDA74XX_INFO_REG_BASS_LOW   /**< Bass filter register */
/** @} */

/**
 * @name    TDA7440 info
 * @{
 */
#define TDA7440_INFO_INPUTS         TDA74XX_INFO_INPUTS_QUAD    /**< Number of inputs */
#define TDA7440_INFO_BANDS          TDA74XX_INFO_BANDS_TWO      /**< Number of tone bands */
#define TDA7440_INFO_REG_BASS       TDA74XX_INFO_REG_BASS_LOW   /**< Bass filter register */
/** @} */

/**
 * @name    TDA7449 info
 * @{
 */
#define TDA7449_INFO_INPUTS         TDA74XX_INFO_INPUTS_DUAL    /**< Number of inputs */
#define TDA7449_INFO_BANDS          TDA74XX_INFO_BANDS_TWO      /**< Number of tone bands */
#define TDA7449_INFO_REG_BASS       TDA74XX_INFO_REG_BASS_HIGH  /**< Bass filter register */
/** @} */

/**
 * @brief   Struct to record TDA74xx audio processor information.
 */
typedef struct {
    uint8_t inputs;     /**< Number of stereo inputs */
    uint8_t bands;      /**< Number of tone control bands */
    uint8_t reg_bass;   /**< Register of the bass filter */
} tda74xx_info_t;

/**
 * @brief   TDA7439 info structure
 */
extern const tda74xx_info_t tda7439_info;

/**
 * @brief   TDA7440 info structure
 */
extern const tda74xx_info_t tda7440_info;

/**
 * @brief   TDA7449 info structure
 */
extern const tda74xx_info_t tda7449_info;

#ifdef __cplusplus
}
#endif

/** @} */
