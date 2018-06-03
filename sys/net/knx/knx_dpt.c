/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
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

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief   Mapping from type to (maximum) size in bytes
 */
static uint8_t sizes[] = {
    [KNX_DPT_1] = 1,
    [KNX_DPT_2] = 2,
    [KNX_DPT_3] = 4,
    [KNX_DPT_4] = 8,
    [KNX_DPT_5] = 8,
    [KNX_DPT_6] = 8,
    [KNX_DPT_7] = 16,
    [KNX_DPT_8] = 16,
    [KNX_DPT_9] = 16,
    [KNX_DPT_10] = 24,
    [KNX_DPT_11] = 24,
    [KNX_DPT_12] = 32,
    [KNX_DPT_13] = 32,
    [KNX_DPT_14] = 32,
    [KNX_DPT_15] = 32,
    [KNX_DPT_16] = 112,
    [KNX_DPT_17] = 8,
    [KNX_DPT_18] = 8,
    [KNX_DPT_19] = 64,
    [KNX_DPT_20] = 8,
    [KNX_DPT_21] = 8,
    [KNX_DPT_22] = 16,
    [KNX_DPT_23] = 2,
    [KNX_DPT_25] = 8,
    [KNX_DPT_26] = 8,
    [KNX_DPT_27] = 32,
    [KNX_DPT_29] = 64,
    [KNX_DPT_30] = 24,
    [KNX_DPT_206] = 24,
    [KNX_DPT_217] = 16,
    [KNX_DPT_219] = 48,
    [KNX_DPT_222] = 48,
    [KNX_DPT_225] = 24,
    [KNX_DPT_229] = 48,
    [KNX_DPT_230] = 64,
    [KNX_DPT_232] = 24,
    [KNX_DPT_234] = 16,
    [KNX_DPT_235] = 48,
    [KNX_DPT_237] = 16,
    [KNX_DPT_238] = 8,
    [KNX_DPT_240] = 24,
    [KNX_DPT_241] = 32,
    [KNX_DPT_244] = 16,
    [KNX_DPT_245] = 48,
    [KNX_DPT_249] = 48,
    [KNX_DPT_250] = 24,
    [KNX_DPT_251] = 48
};

size_t knx_dpt_size(knx_dpt_t type)
{
    assert(type < KNX_DPT_NUMOF);

    return (sizes[type] + 7) / 8;
}

size_t knx_dpt_size_bits(knx_dpt_t type)
{
    assert(type < KNX_DPT_NUMOF);

    return sizes[type];
}

void knx_dpt_to_dpt9xxx(float in, uint8_t *out)
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

    *((uint16_t *)out) = ((significand & 0xff) << 8) |
                         ((sign ? 1 : 0) << 7) |
                         (exponent << 3) |
                         (significand >> 8);
}

void knx_dpt_to_dpt14xxx(float in, uint8_t *out)
{
    assert(out != NULL);

    union {
        uint32_t u32;
        float f;
    } v = { .f = in };

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    v.u32 = htonl(v.u32);
#endif

    *((uint32_t *)out) = v.u32;
}

float knx_dpt_from_dpt14xxx(uint8_t *in)
{
    assert(in != NULL);

    union {
        uint32_t u32;
        float f;
    } v = { .u32 = *(uint32_t *)in } ;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    v.u32 = ntohl(v.u32);
#endif

    return v.f;
}

void knx_dpt_to_dpt19xxx(const struct tm *in, uint8_t *out)
{
    assert(in != NULL);
    assert(out != NULL);

    out[0] = in->tm_year;
    out[1] = (in->tm_mon + 1) & 0x0f;
    out[2] = in->tm_mday & 0x1f;
    out[3] = ((in->tm_wday << 5) & 0xe0) | (in->tm_hour & 0x1f);
    out[4] = in->tm_min & 0x3f;
    out[5] = in->tm_sec & 0x3f;
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
