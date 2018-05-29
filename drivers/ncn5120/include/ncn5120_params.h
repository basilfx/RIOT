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
 * @brief       Default configuration for NCN5120 transceiver
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NCN5120_PARAMS_H
#define NCN5120_PARAMS_H

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Default configuration parameters for the NCN5120 transceiver
 * @{
 */
#ifndef NCN5120_PARAM_UART_DEV
#define NCN5120_PARAM_UART_DEV      (UART_DEV(1))
#endif

#ifndef NCN5120_PARAM_SAVE_PIN
#define NCN5120_PARAM_SAVE_PIN      (GPIO_UNDEF)
#endif

#ifndef NCN5120_PARAMS
#define NCN5120_PARAMS              { .uart_dev = NCN5120_PARAM_UART_DEV, \
                                      .save = NCN5120_PARAM_SAVE_PIN }
#endif
/**@}*/

/**
 * @brief   Configure NCN5120 transceiver
 */
static const ncn5120_params_t ncn5120_params[] =
{
    NCN5120_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* NCN5120_PARAMS_H */
/** @} */
