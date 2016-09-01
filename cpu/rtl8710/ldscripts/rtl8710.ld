/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup      cpu_rtl8710
 * @{
 *
 * @file
 * @brief           Memory definitions for the RTL8710 CPU
 *
 * @}
 */

MEMORY
{
    rom (rwx)   : ORIGIN = 0x10001000, LENGTH = 393216
    ram (rwx)   : ORIGIN = 0x1FFF0000, LENGTH = 65536
}

INCLUDE cortexm_base.ld
