/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @brief   RIOT-OS interface for picoTCP.
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "xtimer.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Map to RIOT-OS debug macro.
 */
#define dbg(...)            DEBUG(...)

/**
 * @brief   No dynamic allocation of memory is supported.
 * @{
 */
#define pico_zalloc(x)      NULL
#define pico_free(x)        do {} while(0)
/** @} */

/**
 * TODO: Not sure what to do here. Block current thread so the idle thread
 * kicks in?
 */
#define PICO_IDLE()         do {} while(0)

/**
 * @brief   Map time methods to xtimer.
 * @{
 */
#define PICO_TIME()         ((uint32_t) (xtimer_now64() / 1000 / 1000))
#define PICO_TIME_MS()      ((uint32_t) (xtimer_now64 / 1000))
/** @} */
