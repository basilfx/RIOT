/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_rtl00
 * @{
 *
 * @file
 * @brief       Board specific implementations RTL-00 module
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "board.h"
#include "cpu.h"

void board_init(void)
{
    /* initialize the CPU */
    cpu_init();

    /* re-enable SWD */
    PERI_ON->CPU_PERIPHERAL_CTRL |= PERI_ON_CPU_PERIPHERAL_CTRL_SWD_PIN_EN; // re-enable SWD

    /* turn on GPIO */
    PERI_ON->PESOC_CLK_CTRL |= PERI_ON_CLK_CTRL_ACTCK_GPIO_EN | PERI_ON_CLK_CTRL_SLPCK_GPIO_EN;
    PERI_ON->SOC_PERI_FUNC1_EN |= PERI_ON_SOC_PERI_FUNC1_EN_GPIO;
}
