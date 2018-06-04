/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
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
#include "u8x8_riotos.h"

#include "xtimer.h"

#include "display.h"

#define ENABLE_DEBUG 0
#include "debug.h"

static char _stack[THREAD_STACKSIZE_DEFAULT + DEBUG_EXTRA_STACKSIZE];

static u8g2_t u8g2;

static u8x8_riotos_t u8g2_user_data =
{
    .device_index = I2C_DEV(0)
};

void *_event_loop(void *args)
{
    (void)args;
    char buf1[32];
    struct tm time;

    u8g2_Setup_ssd1306_i2c_128x64_noname_1(&u8g2, U8G2_R0, u8x8_byte_hw_i2c_riotos, u8x8_gpio_and_delay_riotos);

    u8g2_SetUserPtr(&u8g2, &u8g2_user_data);
    u8g2_SetI2CAddress(&u8g2, 0x3c);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    while (1) {
        u8g2_FirstPage(&u8g2);

        rtc_get_time(&time);

        snprintf(buf1, sizeof(buf1), "%04i-%02i-%02i %02i:%02i:%02i",
                 time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
                 time.tm_hour, time.tm_min, time.tm_sec);

        do {
            u8g2_SetDrawColor(&u8g2, 1);
            u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
            u8g2_DrawStr(&u8g2, 5, 15, buf1);
        } while (u8g2_NextPage(&u8g2));

        /* sleep a little */
        xtimer_sleep(1);
    }
}

kernel_pid_t display_setup(void)
{
    return thread_create(_stack,
                         sizeof(_stack),
                         THREAD_PRIORITY_MAIN - 1,
                         THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST,
                         _event_loop,
                         NULL,
                         "display");
}
