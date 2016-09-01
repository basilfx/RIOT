/*
 * Copyright (C) 2015-2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_efm32_common
 * @{
 *
 * @file
 * @brief       Low-level UART driver implementation.
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Ryan Kurte <ryankurte@gmail.com>
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "cpu.h"
#include "sched.h"
#include "thread.h"

#include "periph/uart.h"
#include "periph/gpio.h"

/* guard file in case no UART device is defined */
#if UART_NUMOF

/**
 * @brief   Allocate memory to store the callback functions
 */
//static uart_isr_ctx_t isr_ctx[UART_NUMOF];

#define mask32_set(target, mask, value) do{ (target) = ((target) & ~((uint32_t)(mask))) | mask32(mask, value); }while(0)
#define mask32(mask, value)             ((((uint32_t)(value)) << __builtin_ctz((mask))) & ((uint32_t)(mask)))

int uart_init(uart_t dev, uint32_t baudrate, uart_rx_cb_t rx_cb, void *arg)
{
    PERI_ON->CPU_PERIPHERAL_CTRL |= PERI_ON_CPU_PERIPHERAL_CTRL_SWD_PIN_EN; // re-enable SWD

	PERI_ON->PESOC_CLK_CTRL |= PERI_ON_CLK_CTRL_ACTCK_GPIO_EN | PERI_ON_CLK_CTRL_SLPCK_GPIO_EN; // enable gpio peripheral clock
	PERI_ON->SOC_PERI_FUNC1_EN |= PERI_ON_SOC_PERI_FUNC1_EN_GPIO; // enable gpio peripheral

    PERI_ON->PESOC_CLK_CTRL |= PERI_ON_CLK_CTRL_ACTCK_LOG_UART_EN | PERI_ON_CLK_CTRL_SLPCK_LOG_UART_EN; // enable LOG_UART peripheral clock
	PERI_ON->SOC_FUNC_EN |= PERI_ON_SOC_FUNC_EN_LOG_UART; // enable LOG_UART peripheral
	//while(!(LOG_UART->LSR & LOG_UART_LSR_TEMT)); // wait until bootloader messages are printed
	mask32_set(PERI_ON->CPU_PERIPHERAL_CTRL, PERI_ON_CPU_PERIPHERAL_CTRL_LOG_UART_PIN_SEL, 0); // default pins are B0/B1
	PERI_ON->CPU_PERIPHERAL_CTRL |= PERI_ON_CPU_PERIPHERAL_CTRL_LOG_UART_PIN_EN; // enable LOG_UART pins

	LOG_UART->IER = 0;
	LOG_UART->LCR |= LOG_UART_LCR_DLAB;
	LOG_UART->DLL = 0x22; // default firmware sets DLL to 0x22 and DLH to 0... -> baudrate to 38400 (used mww and mdw commands with openocd to set DLAB_EN bit and read values)
	// LOG_UART->DLL = 0x11; // bootloader sets DLL to 0x11 and DLH to 0... -> baudrate to 38400 (used mww and mdw commands with openocd to set DLAB_EN bit and read values)
	LOG_UART->DLH = 0x00; // look rtl8195a_uart.c file in the SDK how to calculate baud rate
	LOG_UART->LCR = mask32(LOG_UART_LCR_DLS, 3);
	LOG_UART->FCR = 0;

    return 0;
}

void uart_write(uart_t dev, const uint8_t *data, size_t len)
{
    while (len--) {
        LOG_UART->THR = *(data++);

        for (int i = 0 ; i < 2000; i++) {
            __asm__("nop");
        }
    }
}

void uart_poweron(uart_t dev)
{
    return;
}

void uart_poweroff(uart_t dev)
{
    return;
}

static inline void rx_irq(uart_t dev)
{
    if (sched_context_switch_request) {
        thread_yield();
    }
}

#ifdef UART_0_ISR_RX
void UART_0_ISR_RX(void)
{
    rx_irq(0);
}
#endif

#ifdef UART_1_ISR_RX
void UART_1_ISR_RX(void)
{
    rx_irq(1);
}
#endif

#ifdef UART_2_ISR_RX
void UART_2_ISR_RX(void)
{
    rx_irq(2);
}
#endif

#ifdef UART_3_ISR_RX
void UART_3_ISR_RX(void)
{
    rx_irq(3);
}
#endif

#ifdef UART_4_ISR_RX
void UART_4_ISR_RX(void)
{
    rx_irq(4);
}
#endif

#endif /* UART_NUMOF */
