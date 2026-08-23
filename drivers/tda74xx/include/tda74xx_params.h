/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     drivers_tda74xx
 *
 * @{
 * @file
 * @brief       Default configuration for the ST TDA74xx audio processors
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @note        The address of all devices is fixed, and corresponds to the
 *              chip address byte 0x88 as documented by the datasheets.
 */

#include "board.h"

#include "tda74xx.h"
#include "tda74xx_info.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters
 * @{
 */
#ifdef MODULE_TDA7439
#  ifndef TDA7439_PARAM_I2C_DEV
#    define TDA7439_PARAM_I2C_DEV   I2C_DEV(0)
#  endif
#  ifndef TDA7439_PARAM_ADDR
#    define TDA7439_PARAM_ADDR      (0x44)
#  endif

#  ifndef TDA7439_PARAMS
#    define TDA7439_PARAMS          { .i2c_dev = TDA7439_PARAM_I2C_DEV, \
                                      .address = TDA7439_PARAM_ADDR, \
                                      .info = &tda7439_info }
#  endif
#endif /* MODULE_TDA7439 */

#ifdef MODULE_TDA7440
#  ifndef TDA7440_PARAM_I2C_DEV
#    define TDA7440_PARAM_I2C_DEV   I2C_DEV(0)
#  endif
#  ifndef TDA7440_PARAM_ADDR
#    define TDA7440_PARAM_ADDR      (0x44)
#  endif

#  ifndef TDA7440_PARAMS
#    define TDA7440_PARAMS          { .i2c_dev = TDA7440_PARAM_I2C_DEV, \
                                      .address = TDA7440_PARAM_ADDR, \
                                      .info = &tda7440_info }
#  endif
#endif /* MODULE_TDA7440 */

#ifdef MODULE_TDA7449
#  ifndef TDA7449_PARAM_I2C_DEV
#    define TDA7449_PARAM_I2C_DEV   I2C_DEV(0)
#  endif
#  ifndef TDA7449_PARAM_ADDR
#    define TDA7449_PARAM_ADDR      (0x44)
#  endif

#  ifndef TDA7449_PARAMS
#    define TDA7449_PARAMS          { .i2c_dev = TDA7449_PARAM_I2C_DEV, \
                                      .address = TDA7449_PARAM_ADDR, \
                                      .info = &tda7449_info }
#  endif
#endif /* MODULE_TDA7449 */
/** @} */

/**
 * @brief   Configuration struct
 */
static const tda74xx_params_t tda74xx_params[] =
{
#if MODULE_TDA7439
    TDA7439_PARAMS,
#endif
#if MODULE_TDA7440
    TDA7440_PARAMS,
#endif
#if MODULE_TDA7449
    TDA7449_PARAMS,
#endif
};

#ifdef __cplusplus
}
#endif

/** @} */
