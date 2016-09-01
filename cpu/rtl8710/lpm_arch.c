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
 * @brief       Implementation of the kernels power management interface
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "arch/lpm_arch.h"

void lpm_arch_init(void)
{
    return;
}

enum lpm_mode lpm_arch_set(enum lpm_mode target)
{
    (void) target;

    return LPM_ON;
}

enum lpm_mode lpm_arch_get(void)
{
    return LPM_ON;
}

void lpm_arch_awake(void)
{
    return;
}

void lpm_arch_begin_awake(void)
{
    return;
}

void lpm_arch_end_awake(void)
{
    return;
}
