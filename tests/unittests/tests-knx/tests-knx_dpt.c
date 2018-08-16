/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     unittests
 * @{
 *
 * @file
 * @brief       Test cases for the KNX DPT implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include "embUnit/embUnit.h"

#include "net/knx/dpt.h"

#include "tests-knx.h"

static void test_knx_dpt_size(void)
{
    /* bits */
    TEST_ASSERT(knx_dpt_size(KNX_DPT_1) == 1);
    TEST_ASSERT(knx_dpt_size(KNX_DPT_2) == 1);
    TEST_ASSERT(knx_dpt_size(KNX_DPT_3) == 1);

    /* single byte */
    TEST_ASSERT(knx_dpt_size(KNX_DPT_4) == 1);
    TEST_ASSERT(knx_dpt_size(KNX_DPT_17) == 1);
    TEST_ASSERT(knx_dpt_size(KNX_DPT_238) == 1);

    /* multiple bytes */
    TEST_ASSERT(knx_dpt_size(KNX_DPT_10) == 3);
    TEST_ASSERT(knx_dpt_size(KNX_DPT_16) == 14);
    TEST_ASSERT(knx_dpt_size(KNX_DPT_251) == 6);
}

static void test_knx_dpt_size_bits(void)
{
    /* bits */
    TEST_ASSERT(knx_dpt_size_bits(KNX_DPT_1) == 1);
    TEST_ASSERT(knx_dpt_size_bits(KNX_DPT_2) == 2);
    TEST_ASSERT(knx_dpt_size_bits(KNX_DPT_3) == 4);

    /* single byte */
    TEST_ASSERT(knx_dpt_size_bits(KNX_DPT_4) == 8);
    TEST_ASSERT(knx_dpt_size_bits(KNX_DPT_17) == 8);
    TEST_ASSERT(knx_dpt_size_bits(KNX_DPT_238) == 8);

    /* multiple bytes */
    TEST_ASSERT(knx_dpt_size_bits(KNX_DPT_10) == 24);
    TEST_ASSERT(knx_dpt_size_bits(KNX_DPT_16) == 112);
    TEST_ASSERT(knx_dpt_size_bits(KNX_DPT_251) == 48);
}

static void test_knx_dpt_to_dpt9xxx(void)
{
    uint8_t out[2];

    knx_dpt_to_dpt9xxx(3.14, out);

    TEST_ASSERT(out[0] == 0x01);
    TEST_ASSERT(out[1] == 0x3a);
}

static void test_knx_dpt_to_dpt14xxx(void)
{
    uint8_t out[4];

    knx_dpt_to_dpt14xxx(3.14, out);

    TEST_ASSERT(out[0] == 0x40);
    TEST_ASSERT(out[1] == 0x48);
    TEST_ASSERT(out[2] == 0xf5);
    TEST_ASSERT(out[3] == 0xc3);
}

static void test_knx_dpt_from_dpt14xxx(void)
{
    uint8_t in[] = { 0x40, 0x48, 0xf5, 0xc3 };

    float out = knx_dpt_from_dpt14xxx(in);

    TEST_ASSERT(out - 3.14 <= FLT_EPSILON);
}

static void test_knx_dpt_to_dpt19xxx(void)
{
    struct tm in;
    uint8_t out[6];

    /* 2012-01-12 20:36:34 */
    in.tm_year = 121;
    in.tm_mon = 0;
    in.tm_mday = 12;
    in.tm_wday = 2;
    in.tm_hour = 20;
    in.tm_min = 36;
    in.tm_sec = 34;

    knx_dpt_to_dpt19xxx(&in, out);

    TEST_ASSERT(out[0] == 0x79);
    TEST_ASSERT(out[1] == 0x01);
    TEST_ASSERT(out[2] == 0x0c);
    TEST_ASSERT(out[3] == 0x54);
    TEST_ASSERT(out[4] == 0x24);
    TEST_ASSERT(out[5] == 0x22);
}

static void test_knx_dpt_from_dpt19xxx(void)
{
    uint8_t in[] = { 0x79, 0x01, 0x0c, 0x54, 0x24, 0x22 };
    struct tm out;

    knx_dpt_from_dpt19xxx(in, &out);

    /* 2012-01-12 20:36:34 */
    TEST_ASSERT(out.tm_year == 121);
    TEST_ASSERT(out.tm_mon == 0);
    TEST_ASSERT(out.tm_mday == 12);
    TEST_ASSERT(out.tm_wday == 2);
    TEST_ASSERT(out.tm_hour == 20);
    TEST_ASSERT(out.tm_min == 36);
    TEST_ASSERT(out.tm_sec == 34);
}

Test *tests_knx_dpt_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_knx_dpt_size),
        new_TestFixture(test_knx_dpt_size_bits),
        new_TestFixture(test_knx_dpt_to_dpt9xxx),
        new_TestFixture(test_knx_dpt_to_dpt14xxx),
        new_TestFixture(test_knx_dpt_from_dpt14xxx),
        new_TestFixture(test_knx_dpt_to_dpt19xxx),
        new_TestFixture(test_knx_dpt_from_dpt19xxx),
    };

    EMB_UNIT_TESTCALLER(knx_dpt_tests, NULL, NULL, fixtures);

    return (Test *)&knx_dpt_tests;
}
