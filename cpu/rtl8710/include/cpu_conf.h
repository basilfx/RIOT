/*
* Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
*
* This file is subject to the terms and conditions of the GNU Lesser
* General Public License v2.1. See the file LICENSE in the top level
* directory for more details.
*/

/**
* @defgroup    cpu_rtl8710 Realtek RTL8710
* @ingroup     cpu
* @brief       Support for the Realtek RTL8710
* @{
*
* @file
* @brief       Implementation specific CPU configuration options
*
* @author      Bas Stottelaar <basstottelaar@gmail.com>
*/

#ifndef CPU_CONF_H
#define CPU_CONF_H

#include "cpu_conf_common.h"

#include "rtl8710.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   ARM Cortex-M3 specific CPU configuration
 * @{
 */
#define CPU_DEFAULT_IRQ_PRIO            (1U)
#define CPU_IRQ_NUMOF                   (240)
#define CPU_FLASH_BASE                  (0x10001000)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __CPU_CONF_H */
/** @} */
