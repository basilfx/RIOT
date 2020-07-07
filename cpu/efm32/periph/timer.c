/*
 * Copyright (C) 2015-2017 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_efm32
 * @ingroup     drivers_periph_timer
 * @{
 *
 * @file
 * @brief       Low-level timer driver implementation
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 * @}
 */

#include "cpu.h"

#include "periph/timer.h"
#include "periph_conf.h"

#include "em_cmu.h"
#include "em_timer.h"
#include "em_timer_utils.h"

/**
 * @brief   This timer implementation has three available channels
 */
#define CC_CHANNELS      (3U)

/**
 * @brief   Timer state memory
 */
static timer_isr_ctx_t isr_ctx[TIMER_NUMOF];

/**
 * @brief   Check whether device is a using a WTIMER device (32-bit)
 */
static inline bool _is_wtimer(timer_t dev)
{
#if defined(WTIMER_COUNT) && WTIMER_COUNT > 0
    return ((uint32_t) timer_config[dev].timer.dev) >= WTIMER0_BASE;
#else
    (void) dev;

    return false;
#endif
}

int timer_init(tim_t dev, unsigned long freq, timer_cb_t callback, void *arg)
{
    TIMER_TypeDef *pre, *tim;

    /* test if given timer device is valid */
    if (dev >= TIMER_NUMOF) {
        return -1;
    }

    /* save callback */
    isr_ctx[dev].cb = callback;
    isr_ctx[dev].arg = arg;

    /* get timers */
    pre = timer_config[dev].prescaler.dev;
    tim = timer_config[dev].timer.dev;

    /* enable clocks */
#if defined(_SILICON_LABS_32B_SERIES_0) || defined(_SILICON_LABS_32B_SERIES_1)
    CMU_ClockEnable(cmuClock_HFPER, true);
#endif
    CMU_ClockEnable(timer_config[dev].prescaler.cmu, true);
    CMU_ClockEnable(timer_config[dev].timer.cmu, true);

    /* reset and initialize peripherals */
    TIMER_Init_TypeDef init_pre = TIMER_INIT_DEFAULT;
    TIMER_Init_TypeDef init_tim = TIMER_INIT_DEFAULT;

    init_pre.enable = false;
    init_pre.prescale = timerPrescale1;

    init_tim.enable = false;
    init_tim.clkSel = timerClkSelCascade;

    TIMER_Reset(tim);
    TIMER_Reset(pre);

    TIMER_Init(tim, &init_tim);
    TIMER_Init(pre, &init_pre);

    /* configure the prescaler top value */
    uint32_t freq_timer = CMU_ClockFreqGet(timer_config[dev].prescaler.cmu);
    uint32_t top = (
        freq_timer / TIMER_Prescaler2Div(init_pre.prescale) / freq) - 1;

    TIMER_TopSet(pre, top);
    TIMER_TopSet(tim, _is_wtimer(dev) ? 0xffffffff : 0xffff);

    /* enable interrupts for the channels */
#if defined(_SILICON_LABS_32B_SERIES_0) || defined(_SILICON_LABS_32B_SERIES_1)
    TIMER_IntClear(tim, TIMER_IFC_CC0 | TIMER_IFC_CC1 | TIMER_IFC_CC2);
#elif defined(_SILICON_LABS_32B_SERIES_2)
    TIMER_IntClear(tim, TIMER_IF_CC0 | TIMER_IF_CC1 | TIMER_IF_CC2);
#endif
    TIMER_IntEnable(tim, TIMER_IEN_CC0 | TIMER_IEN_CC1 | TIMER_IEN_CC2);

    NVIC_ClearPendingIRQ(timer_config[dev].irq);
    NVIC_EnableIRQ(timer_config[dev].irq);

    /* start the timers */
    TIMER_Enable(tim, true);
    TIMER_Enable(pre, true);

    return 0;
}

int timer_set_absolute(tim_t dev, int channel, unsigned int value)
{
    TIMER_TypeDef *tim;

    if (channel < 0 || channel >= (int) CC_CHANNELS) {
        return -1;
    }

    /* this accounts for some timer being 16-bit and others 32-bit */
    if (value > TIMER_TopGet(timer_config[dev].timer.dev)) {
        return -1;
    }

    tim = timer_config[dev].timer.dev;
#if defined(_SILICON_LABS_32B_SERIES_0) || defined(_SILICON_LABS_32B_SERIES_1)
    tim->CC[channel].CCV = (uint32_t) value;
    tim->CC[channel].CTRL = TIMER_CC_CTRL_MODE_OUTPUTCOMPARE;
#elif defined(_SILICON_LABS_32B_SERIES_2)
    tim->CC[channel].OC = (uint32_t) value;
    tim->CC[channel].CFG = TIMER_CC_CFG_MODE_OUTPUTCOMPARE;
#endif

    return 0;
}

int timer_clear(tim_t dev, int channel)
{
#if defined(_SILICON_LABS_32B_SERIES_0) || defined(_SILICON_LABS_32B_SERIES_1)
    timer_config[dev].timer.dev->CC[channel].CTRL = _TIMER_CC_CTRL_MODE_OFF;
#elif defined(_SILICON_LABS_32B_SERIES_2)
    timer_config[dev].timer.dev->CC[channel].CFG = _TIMER_CC_CFG_MODE_OFF;
#endif
    return 0;
}

unsigned int timer_read(tim_t dev)
{
    return (unsigned int) TIMER_CounterGet(timer_config[dev].timer.dev);
}

void timer_stop(tim_t dev)
{
    TIMER_Enable(timer_config[dev].timer.dev, false);
}

void timer_start(tim_t dev)
{
    TIMER_Enable(timer_config[dev].timer.dev, true);
}

#ifdef TIMER_0_ISR
void TIMER_0_ISR(void)
{
    TIMER_TypeDef *tim = timer_config[0].timer.dev;

    for (int i = 0; i < (int) CC_CHANNELS; i++) {
        if (tim->IF & (TIMER_IF_CC0 << i)) {
#if defined(_SILICON_LABS_32B_SERIES_0) || defined(_SILICON_LABS_32B_SERIES_1)
            tim->CC[i].CTRL = _TIMER_CC_CTRL_MODE_OFF;
            tim->IFC = (TIMER_IFC_CC0 << i);
#elif defined(_SILICON_LABS_32B_SERIES_2)
            tim->CC[i].CFG = _TIMER_CC_CFG_MODE_OFF;
            tim->IF = (TIMER_IF_CC0 << i);
#endif
            isr_ctx[0].cb(isr_ctx[0].arg, i);
        }
    }
    cortexm_isr_end();
}
#endif /* TIMER_0_ISR */
