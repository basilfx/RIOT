/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    boards_rtl00 RTL-00 module
 * @ingroup     boards
 * @brief       Support for the RTL-00 module
 * @{
 *
 * @file
 * @brief       Board specific definitions for the RTL-00 module
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef BOARD_H
#define BOARD_H

#include "cpu.h"

#include "periph_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize board specific hardware, including clock, LEDs and stdio
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
