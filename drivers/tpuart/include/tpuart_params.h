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
 * @brief       Default configuration for TPUART transceiver
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef TPUART_PARAMS_H
#define TPUART_PARAMS_H

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Default configuration parameters for the TPUART transceiver
 * @{
 */
#ifndef TPUART_PARAM_UART_DEV
#define TPUART_PARAM_UART_DEV        (UART_DEV(1))
#endif

#ifndef TPUART_PARAM_SAVE_PIN
#define TPUART_PARAM_SAVE_PIN        (GPIO_UNDEF)
#endif

#ifndef TPUART_PARAMS
#define TPUART_PARAMS                { .uart_dev = TPUART_PARAM_UART_DEV, \
                                       .save = TPUART_PARAM_SAVE_PIN }
#endif
/**@}*/

/**
 * @brief   Configure TPUART transceiver
 */
static const tpuart_params_t tpuart_params[] =
{
    TPUART_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* TPUART_PARAMS_H */
/** @} */
