/*
 * Copyright (C) 2015-2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_slstk3402a
 * @{
 *
 * @file
 * @brief       Configuration of CPU peripherals for the SLSTK3402A starter kit
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include "cpu.h"
#include "periph_cpu.h"
#include "em_cmu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Clock configuration
 * @{
 */
#ifndef CLOCK_HF
#define CLOCK_HF            cmuSelect_HFXO
#endif
#ifndef CLOCK_CORE_DIV
#define CLOCK_CORE_DIV      cmuClkDiv_1
#endif
#ifndef CLOCK_LFA
#define CLOCK_LFA           cmuSelect_LFXO
#endif
#ifndef CLOCK_LFB
#define CLOCK_LFB           cmuSelect_LFXO
#endif
#ifndef CLOCK_LFE
#define CLOCK_LFE           cmuSelect_LFXO
#endif
/** @} */

/**
 * @name    DC-DC configuration
 * @{
 */
#define EMU_DCDCINIT_OFF
/** @} */

/**
 * @name    ADC configuration
 * @{
 */
static const adc_conf_t adc_config[] = {
    {
        .dev = ADC0,
        .cmu = cmuClock_ADC0,
    }
};

static const adc_chan_conf_t adc_channel_config[] = {
    {
        .dev = 0,
        .input = adcPosSelTEMP,
        .reference = adcRef1V25,
        .acq_time = adcAcqTime8
    },
    {
        .dev = 0,
        .input = adcPosSelAVDD,
        .reference = adcRef5V,
        .acq_time = adcAcqTime8
    }
};

#define ADC_DEV_NUMOF       ARRAY_SIZE(adc_config)
#define ADC_NUMOF           ARRAY_SIZE(adc_channel_config)
/** @} */

/**
 * @name    I2C configuration
 * @{
 */
static const i2c_conf_t i2c_config[] = {
    {
        .dev = I2C0,
        .sda_pin = GPIO_PIN(PF, 3),
        .scl_pin = GPIO_PIN(PF, 4),
        .loc = I2C_ROUTELOC0_SDALOC_LOC27 |
               I2C_ROUTELOC0_SCLLOC_LOC27,
        .cmu = cmuClock_I2C0,
        .irq = I2C0_IRQn,
        .speed = I2C_SPEED_NORMAL
    }
};

#define I2C_NUMOF           ARRAY_SIZE(i2c_config)
#define I2C_0_ISR           isr_i2c0
/** @} */

/**
 * @brief   RTC configuration
 */
#define RTC_NUMOF           (1U)

/**
 * @name    RTT configuration
 * @{
 */
#define RTT_NUMOF           (1U)

#define RTT_MAX_VALUE       (0xFFFFFFFF)
#define RTT_FREQUENCY       (1U)
/** @} */

/**
 * @name    SPI configuration
 * @{
 */
static const spi_dev_t spi_config[] = {
    {
        .dev = USART3,
        .mosi_pin = GPIO_PIN(PD, 13),
        .miso_pin = GPIO_PIN(PD, 14),
        .clk_pin = GPIO_PIN(PD, 12),
        .loc = USART_ROUTELOC0_RXLOC_LOC5 |
               USART_ROUTELOC0_TXLOC_LOC5 |
               USART_ROUTELOC0_CLKLOC_LOC2,
        .cmu = cmuClock_USART3,
        .irq = USART3_RX_IRQn
    }
};

#define SPI_NUMOF           ARRAY_SIZE(spi_config)
/** @} */

/**
 * @name    Timer configuration
 *
 * The implementation uses two timers in cascade mode.
 * @{
 */
static const timer_conf_t timer_config[] = {
    {
        {
            .dev = WTIMER0,
            .cmu = cmuClock_WTIMER0
        },
        {
            .dev = WTIMER1,
            .cmu = cmuClock_WTIMER1
        },
        .irq = WTIMER1_IRQn
    }
};

#define TIMER_NUMOF         ARRAY_SIZE(timer_config)
#define TIMER_0_ISR         isr_wtimer1
/** @} */

/**
 * @name    UART configuration
 * @{
 */
static const uart_conf_t uart_config[] = {
    {
        .dev = USART1,
        .rx_pin = GPIO_PIN(PA, 1),
        .tx_pin = GPIO_PIN(PA, 0),
        .loc = USART_ROUTELOC0_RXLOC_LOC0 |
               USART_ROUTELOC0_TXLOC_LOC0,
        .cmu = cmuClock_USART1,
        .irq = USART1_RX_IRQn
    },
    {
        .dev = USART0,
        .rx_pin = GPIO_PIN(PF, 6),
        .tx_pin = GPIO_PIN(PF, 7),
        .loc = USART_ROUTELOC0_RXLOC_LOC29 |
               USART_ROUTELOC0_TXLOC_LOC31,
        .cmu = cmuClock_USART0,
        .irq = USART0_RX_IRQn
    }
};

#define UART_NUMOF          ARRAY_SIZE(uart_config)
#define UART_0_ISR_RX       isr_usart1_rx
#define UART_1_ISR_RX       isr_usart0_rx
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */
