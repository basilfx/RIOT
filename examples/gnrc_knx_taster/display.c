/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>

#include "thread.h"

#include "periph/i2c.h"
#include "periph/rtc.h"

#include "u8g2.h"
#include "xtimer.h"

#include "display.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static char _stack[THREAD_STACKSIZE_DEFAULT + DEBUG_EXTRA_STACKSIZE];

static u8g2_t u8g2;

void *_event_loop(void *args)
{
    (void)args;
    char buf1[32];
    char buf2[32];
    struct tm time;

    u8g2_Setup_ssd1306_i2c_128x64_noname_1(&u8g2, U8G2_R0, u8x8_byte_riotos_hw_i2c, u8x8_gpio_and_delay_riotos);

    //u8g2_SetPins(&u8g2, pins, pins_enabled);
    u8g2_SetDevice(&u8g2, I2C_DEV(0));
    u8g2_SetI2CAddress(&u8g2, 0x3c);

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    while (1) {
        u8g2_FirstPage(&u8g2);

        rtc_get_time(&time);

        snprintf(buf1, sizeof(buf1), "%04i-%02i-%02i",
                 time.tm_year + 1900, time.tm_mon + 1, time.tm_mday);
        snprintf(buf2, sizeof(buf2), "%02i:%02i:%02i",
                 time.tm_hour, time.tm_min, time.tm_sec);

        do {
            u8g2_SetDrawColor(&u8g2, 1);
            u8g2_SetFont(&u8g2, u8g2_font_helvB12_tf);
            u8g2_DrawStr(&u8g2, 12, 22, buf1);
            u8g2_DrawStr(&u8g2, 12, 42, buf2);
        } while (u8g2_NextPage(&u8g2));

        /* sleep a little */
        xtimer_sleep(1);
    }
}

kernel_pid_t display_setup(void)
{
    return thread_create(_stack, sizeof(_stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST, _event_loop, NULL, "display");
}
