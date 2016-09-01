/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_rtl8710
 * @{
 *
 * @file
 * @brief       Implementation of the CPU initialization
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "cpu.h"
#include "periph_conf.h"

void cpu_init(void)
{
    /* initialize the Cortex-M core */
    cortexm_init();
}
