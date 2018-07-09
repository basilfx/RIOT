/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_basilfx-taster
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
 *
 * The timer runs at 1000 kHz to increase accuracy, or at 32.768 kHz if
 * LETIMER is used.
 * @{
 */
#if IS_ACTIVE(CONFIG_EFM32_XTIMER_USE_LETIMER)
#define XTIMER_DEV          (TIMER_DEV(2))
#define XTIMER_HZ           (32768UL)
#define XTIMER_WIDTH        (16)
#else
#define XTIMER_DEV          (TIMER_DEV(0))
#define XTIMER_HZ           (1000000UL)
#define XTIMER_WIDTH        (32)
#endif
#define XTIMER_CHAN         (0)
/** @} */

/**
 * @name    Push button pin definitions
 * @{
 */
#define PB0_PIN             GPIO_PIN(PB, 13)
#define PB1_PIN             GPIO_UNDEF
/** @} */

/**
 * @name    LED pin definitions
 * @{
 */
#define LED0_PIN            GPIO_PIN(PB, 12)
#define LED1_PIN            GPIO_UNDEF
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

/**
 * @name    Core temperature sensor configuration
 *
 * Connection to the on-chip temperature sensor.
 * @{
 */
#define CORETEMP_ADC        ADC_LINE(0)
/** @} */

/**
 * @name    Pressure sensor configuration
 *
 * Connection to the on-board pressure sensor (BMP280).
 * @{
 */
#define BMP280_I2C              I2C_DEV(0)

#define BMX280_PARAM_I2C_DEV    BMP280_I2C
#define BMX280_PARAM_I2C_ADDR   (0x76)
/** @} */

/**
 * @name    SPI NOR Flash hardware configuration
 *
 * The board has a MX25R8035F flash chip (8MBit).
 * @{
 */
#define BASILFX_TASTER_NOR_PAGE_SIZE          (256)
#define BASILFX_TASTER_NOR_PAGES_PER_SECTOR   (16)
#define BASILFX_TASTER_NOR_SECTOR_COUNT       (256)
#define BASILFX_TASTER_NOR_FLAGS              (SPI_NOR_F_SECT_4K | SPI_NOR_F_SECT_32K)
#define BASILFX_TASTER_NOR_SPI_DEV            SPI_DEV(0)
#define BASILFX_TASTER_NOR_SPI_CLK            SPI_CLK_10MHZ
#define BASILFX_TASTER_NOR_SPI_CS             GPIO_PIN(PD, 15)
#define BASILFX_TASTER_NOR_SPI_MODE           SPI_MODE_0
/** @} */

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
