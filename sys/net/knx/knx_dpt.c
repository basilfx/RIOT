/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_knx_dpt
 * @{
 *
 * @file
 * @brief       KNX data point types implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdbool.h>

#include "net/knx/dpt.h"

#include "assert.h"
#include "byteorder.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

void knx_dpt_to_dpt9xxx(float in, uint16_t *out)
{
    assert(out != NULL);

    bool sign = (in < 0);

    uint8_t exponent = 0;
    long significand = sign ? (-100 * in) : (100 * in);

    while (significand < -2048 || significand > 2048) {
        exponent += 1;
        significand = significand >> 1;
    }

    if (sign) {
        significand = significand ^ 0x7ff;
        significand += 1;
    }

    *out = ((significand & 0xff) << 8) |
           ((sign ? 1 : 0) << 7) | (exponent << 3) | (significand >> 8);
}

void knx_dpt_to_dpt14xxx(float in, uint32_t *out)
{
    assert(out != NULL);

    uint32_t *tmp = (uint32_t *)&in;

    *out = htonl(*tmp);
}

void knx_dpt_from_dpt19xxx(const uint8_t *in, struct tm *out)
{
    assert(in != NULL);
    assert(out != NULL);

    out->tm_year = in[0];
    out->tm_mon = (in[1] & 0x0f) - 1;
    out->tm_mday = in[2] & 0x1f;
    out->tm_wday = (in[3] & 0xe0) >> 5;
    out->tm_hour = in[3] & 0x1f;
    out->tm_min = in[4] & 0x3f;
    out->tm_sec = in[5] & 0x3f;
}
