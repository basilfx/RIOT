/*
 * Copyright (C) 2015-2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    boards_basilfx-taster BasilFX Taster
 * @ingroup     boards
 * @brief       Support for the BasilFX Taster KNX board
 * @{
 *
 * @file
 * @brief       Board specific definitions for the BasilFX Taster KNX board
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef BOARD_H
#define BOARD_H

#include "cpu.h"
#include "periph_conf.h"
#include "periph/gpio.h"
#include "periph/spi.h"
#include "mtd.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Xtimer configuration
 * @{
 */
#define XTIMER_HZ           (1000000UL)
#define XTIMER_WIDTH        (32)
/** @} */

/**
 * @name    Push button pin definitions
 * @{
 */
#define PB0_PIN             GPIO_PIN(PB, 13)
/** @} */

/**
 * @name    LED pin definitions
 * @{
 */
#define LED0_PIN            GPIO_PIN(PB, 12)
/** @} */

/**
 * @name    Interface pin definitions
 * @{
 */
#define IF0_PIN             GPIO_PIN(PC, 11)
#define IF1_PIN             GPIO_PIN(PC, 10)
#define IF2_PIN             GPIO_PIN(PC, 9)
#define IF3_PIN             GPIO_PIN(PC, 8)
#define IF4_PIN             GPIO_PIN(PC, 7)
#define IF5_PIN             GPIO_PIN(PC, 6)
/** @} */

/**
 * @name    I2C pin definitions
 * @{
 */
#define SDA_PIN             GPIO_PIN(PF, 3)
#define SCL_PIN             GPIO_PIN(PF, 4)
/** @} */

/**
 * @name    SPI pin definitions
 * @{
 */
#define MOSI_PIN            GPIO_PIN(PD, 13)
#define MISO_PIN            GPIO_PIN(PD, 14)
#define CLK_PIN             GPIO_PIN(PD, 12)
#define CS_PIN              GPIO_PIN(PD, 15)
/** @} */

/**
 * @name    Macros for controlling the on-board LEDs
 * @{
 */
#define LED0_ON             gpio_set(LED0_PIN)
#define LED0_OFF            gpio_clear(LED0_PIN)
#define LED0_TOGGLE         gpio_toggle(LED0_PIN)
/** @} */

#define BMX280_PARAM_I2C_ADDR        (0x76)

/**
 * @name    MTD configuration
 * @{
 */
extern mtd_dev_t *mtd0;
#define MTD_0 mtd0
/** @} */

/**
 * @brief   Initialize the board (GPIO, sensors, clocks).
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
