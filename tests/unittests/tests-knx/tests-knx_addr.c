/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
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
 * @brief       Test cases for the KNX address implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "embUnit/embUnit.h"

#include "net/knx/addr.h"

#include "tests-knx.h"

static void test_knx_addr_equal__unequal(void)
{
    knx_addr_t a = { { 0x00, 0x11 } };
    knx_addr_t b = { { 0x11, 0x00 } };

    TEST_ASSERT(!knx_addr_equal(&a, &b));
}

static void test_knx_addr_equal__equal(void)
{
    knx_addr_t a = { { 0x00, 0x11 } };
    knx_addr_t b = { { 0x00, 0x11 } };

    TEST_ASSERT(knx_addr_equal(&a, &b));
}

static void test_knx_addr_compare__equal(void)
{
    knx_addr_t a = { { 0x00, 0x11 } };
    knx_addr_t b = { { 0x00, 0x11 } };

    TEST_ASSERT(knx_addr_compare(&a, &b) == 0);
}

static void test_knx_addr_compare__smaller(void)
{
    knx_addr_t a = { { 0x00, 0x11 } };
    knx_addr_t b = { { 0x11, 0x00 } };

    TEST_ASSERT(knx_addr_compare(&a, &b) < 0);
}

static void test_knx_addr_compare__bigger(void)
{
    knx_addr_t a = { { 0x11, 0x00 } };
    knx_addr_t b = { { 0x00, 0x11 } };

    TEST_ASSERT(knx_addr_compare(&a, &b) > 0);
}

static void test_knx_addr_physical__valid(void)
{
    knx_addr_physical_t tmp;

    TEST_ASSERT(knx_addr_physical(&tmp, 1, 1, 8) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0x11);
    TEST_ASSERT(tmp.u8[1] == 0x08);
}

static void test_knx_addr_physical__invalid(void)
{
    knx_addr_physical_t tmp;

    TEST_ASSERT(knx_addr_physical(&tmp, 255, 0, 0) == NULL);
    TEST_ASSERT(knx_addr_physical(&tmp, 0, 255, 0) == NULL);
    TEST_ASSERT(knx_addr_physical(&tmp, 255, 255, 0) == NULL);
}

static void test_knx_addr_physical__null(void)
{
    TEST_ASSERT(knx_addr_physical(NULL, 1, 1, 8) == NULL);
}

static void test_knx_addr_group__valid(void)
{
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group(&tmp, 1, 1, 8) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0x09);
    TEST_ASSERT(tmp.u8[1] == 0x08);
}

static void test_knx_addr_group__invalid(void)
{
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group(&tmp, 255, 0, 0) == NULL);
    TEST_ASSERT(knx_addr_group(&tmp, 0, 255, 0) == NULL);
    TEST_ASSERT(knx_addr_group(&tmp, 255, 255, 0) == NULL);
}

static void test_knx_addr_group__null(void)
{
    TEST_ASSERT(knx_addr_group(NULL, 1, 1, 1) == NULL);
}

static void test_knx_addr_group2__valid(void)
{
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group2(&tmp, 31, 2047) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0xff);
    TEST_ASSERT(tmp.u8[1] == 0xff);
}

static void test_knx_addr_group2__invalid(void)
{
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group2(&tmp, 32, 0) == NULL);
    TEST_ASSERT(knx_addr_group2(&tmp, 0, 2048) == NULL);
    TEST_ASSERT(knx_addr_group2(&tmp, 32, 2048) == NULL);
}

static void test_knx_addr_group2__null(void)
{
    TEST_ASSERT(knx_addr_group2(NULL, 1, 1) == NULL);
}

static void test_knx_addr_physical_from_str__valid(void)
{
    char *addr = "1.1.8";
    knx_addr_physical_t tmp;

    TEST_ASSERT(knx_addr_physical_from_str(&tmp, addr) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0x11);
    TEST_ASSERT(tmp.u8[1] == 0x08);
}

static void test_knx_addr_physical_from_str__invalid(void)
{
    char *addr = "-1.-1.-8";
    knx_addr_physical_t tmp;

    TEST_ASSERT(knx_addr_physical_from_str(&tmp, addr) == NULL);
}

static void test_knx_addr_physical_from_str__null(void)
{
    char *addr = "1.1.8";

    TEST_ASSERT(knx_addr_physical_from_str(NULL, addr) == NULL);
}

static void test_knx_addr_physical_from_str__null_addr(void)
{
    knx_addr_physical_t tmp;

    TEST_ASSERT(knx_addr_physical_from_str(&tmp, NULL) == NULL);
}

static void test_knx_addr_physical_from_str__incomplete(void)
{
    char *addr = "1.1.";
    knx_addr_physical_t tmp;

    TEST_ASSERT(knx_addr_physical_from_str(&tmp, addr) == NULL);
}

static void test_knx_addr_physical_from_str__too_long(void)
{
    char *addr = "1.1-too-long";
    knx_addr_physical_t tmp;

    TEST_ASSERT(knx_addr_physical_from_str(&tmp, addr) == NULL);
}

static void test_knx_addr_group_from_str__valid(void)
{
    char *addr = "1/1/8";
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group_from_str(&tmp, addr) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0x09);
    TEST_ASSERT(tmp.u8[1] == 0x08);
}

static void test_knx_addr_group_from_str__invalid(void)
{
    char *addr = "-1/-1/-8";
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group_from_str(&tmp, addr) == NULL);
}

static void test_knx_addr_group_from_str__null(void)
{
    char *addr = "1/1/8";

    TEST_ASSERT(knx_addr_group_from_str(NULL, addr) == NULL);
}

static void test_knx_addr_group_from_str__null_addr(void)
{
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group_from_str(&tmp, NULL) == NULL);
}

static void test_knx_addr_group_from_str__incomplete(void)
{
    char *addr = "1/1/";
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group_from_str(&tmp, addr) == NULL);
}

static void test_knx_addr_group_from_str__too_long(void)
{
    char *addr = "1/1-too-long";
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group_from_str(&tmp, addr) == NULL);
}

static void test_knx_addr_group2_from_str__valid(void)
{
    char *addr = "1/8";
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group2_from_str(&tmp, addr) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0x08);
    TEST_ASSERT(tmp.u8[1] == 0x08);
}

static void test_knx_addr_group2_from_str__invalid(void)
{
    char *addr = "-1/-8";
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group2_from_str(&tmp, addr) == NULL);
}

static void test_knx_addr_group2_from_str__null(void)
{
    char *addr = "1/8";

    TEST_ASSERT(knx_addr_group2_from_str(NULL, addr) == NULL);
}

static void test_knx_addr_group2_from_str__null_addr(void)
{
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group2_from_str(&tmp, NULL) == NULL);
}

static void test_knx_addr_group2_from_str__incomplete(void)
{
    char *addr = "1/";
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group2_from_str(&tmp, addr) == NULL);
}

static void test_knx_addr_group2_from_str__too_long(void)
{
    char *addr = "1-too-long";
    knx_addr_group_t tmp;

    TEST_ASSERT(knx_addr_group2_from_str(&tmp, addr) == NULL);
}

static void test_knx_addr_physical_to_str__valid(void)
{
    knx_addr_physical_t addr = { { 0x11, 0x08 } };
    char tmp[KNX_ADDR_MAX_STR_LEN];

    TEST_ASSERT(knx_addr_physical_to_str(tmp, &addr, sizeof(tmp)) != NULL);

    TEST_ASSERT(strcmp(tmp, "1.1.8") == 0);
}

static void test_knx_addr_physical_to_str__too_short(void)
{
    knx_addr_physical_t addr = { { 0x11, 0x08 } };
    char tmp[KNX_ADDR_MAX_STR_LEN - 1];

    TEST_ASSERT(knx_addr_physical_to_str(tmp, &addr, sizeof(tmp)) == NULL);
}

static void test_knx_addr_physical_to_str__null(void)
{
    knx_addr_physical_t addr = { { 0x11, 0x08 } };

    TEST_ASSERT(knx_addr_physical_to_str(NULL, &addr, KNX_ADDR_MAX_STR_LEN) == NULL);
}

static void test_knx_addr_physical_to_str__null_addr(void)
{
    char tmp[KNX_ADDR_MAX_STR_LEN];

    TEST_ASSERT(knx_addr_physical_to_str(tmp, NULL, KNX_ADDR_MAX_STR_LEN) == NULL);
}

static void test_knx_addr_group_to_str__valid(void)
{
    knx_addr_group_t addr = { { 0x11, 0x08 } };
    char tmp[KNX_ADDR_MAX_STR_LEN];

    TEST_ASSERT(knx_addr_group_to_str(tmp, &addr, sizeof(tmp)) != NULL);

    TEST_ASSERT(strcmp(tmp, "1/1/8") == 0);
}

static void test_knx_addr_group_to_str__too_short(void)
{
    knx_addr_group_t addr = { { 0x11, 0x08 } };
    char tmp[KNX_ADDR_MAX_STR_LEN - 1];

    TEST_ASSERT(knx_addr_group_to_str(tmp, &addr, sizeof(tmp)) == NULL);
}

static void test_knx_addr_group_to_str__null(void)
{
    knx_addr_group_t addr = { { 0x11, 0x08 } };

    TEST_ASSERT(knx_addr_group_to_str(NULL, &addr, KNX_ADDR_MAX_STR_LEN) == NULL);
}

static void test_knx_addr_group_to_str__null_addr(void)
{
    char tmp[KNX_ADDR_MAX_STR_LEN];

    TEST_ASSERT(knx_addr_group_to_str(tmp, NULL, KNX_ADDR_MAX_STR_LEN) == NULL);
}

static void test_knx_addr_group2_to_str__valid(void)
{
    knx_addr_group_t addr = { { 0x11, 0x08 } };
    char tmp[KNX_ADDR_MAX_STR_LEN];

    TEST_ASSERT(knx_addr_group2_to_str(tmp, &addr, sizeof(tmp)) != NULL);

    TEST_ASSERT(strcmp(tmp, "2/264") == 0);
}

static void test_knx_addr_group2_to_str__too_short(void)
{
    knx_addr_group_t addr = { { 0x11, 0x08 } };
    char tmp[KNX_ADDR_MAX_STR_LEN - 1];

    TEST_ASSERT(knx_addr_group2_to_str(tmp, &addr, sizeof(tmp)) == NULL);
}

static void test_knx_addr_group2_to_str__null(void)
{
    knx_addr_group_t addr = { { 0x11, 0x08 } };

    TEST_ASSERT(knx_addr_group2_to_str(NULL, &addr, KNX_ADDR_MAX_STR_LEN) == NULL);
}

static void test_knx_addr_group2_to_str__null_addr(void)
{
    char tmp[KNX_ADDR_MAX_STR_LEN];

    TEST_ASSERT(knx_addr_group2_to_str(tmp, NULL, KNX_ADDR_MAX_STR_LEN) == NULL);
}

Test *tests_knx_addr_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_knx_addr_equal__unequal),
        new_TestFixture(test_knx_addr_equal__equal),
        new_TestFixture(test_knx_addr_compare__equal),
        new_TestFixture(test_knx_addr_compare__smaller),
        new_TestFixture(test_knx_addr_compare__bigger),
        new_TestFixture(test_knx_addr_physical__valid),
        new_TestFixture(test_knx_addr_physical__invalid),
        new_TestFixture(test_knx_addr_physical__null),
        new_TestFixture(test_knx_addr_group__valid),
        new_TestFixture(test_knx_addr_group__invalid),
        new_TestFixture(test_knx_addr_group__null),
        new_TestFixture(test_knx_addr_group2__valid),
        new_TestFixture(test_knx_addr_group2__invalid),
        new_TestFixture(test_knx_addr_group2__null),
        new_TestFixture(test_knx_addr_physical_from_str__valid),
        new_TestFixture(test_knx_addr_physical_from_str__invalid),
        new_TestFixture(test_knx_addr_physical_from_str__null),
        new_TestFixture(test_knx_addr_physical_from_str__null_addr),
        new_TestFixture(test_knx_addr_physical_from_str__incomplete),
        new_TestFixture(test_knx_addr_physical_from_str__too_long),
        new_TestFixture(test_knx_addr_group_from_str__valid),
        new_TestFixture(test_knx_addr_group_from_str__invalid),
        new_TestFixture(test_knx_addr_group_from_str__null),
        new_TestFixture(test_knx_addr_group_from_str__null_addr),
        new_TestFixture(test_knx_addr_group_from_str__incomplete),
        new_TestFixture(test_knx_addr_group_from_str__too_long),
        new_TestFixture(test_knx_addr_group2_from_str__valid),
        new_TestFixture(test_knx_addr_group2_from_str__invalid),
        new_TestFixture(test_knx_addr_group2_from_str__null),
        new_TestFixture(test_knx_addr_group2_from_str__null_addr),
        new_TestFixture(test_knx_addr_group2_from_str__incomplete),
        new_TestFixture(test_knx_addr_group2_from_str__too_long),
        new_TestFixture(test_knx_addr_physical_to_str__valid),
        new_TestFixture(test_knx_addr_physical_to_str__too_short),
        new_TestFixture(test_knx_addr_physical_to_str__null),
        new_TestFixture(test_knx_addr_physical_to_str__null_addr),
        new_TestFixture(test_knx_addr_group_to_str__valid),
        new_TestFixture(test_knx_addr_group_to_str__too_short),
        new_TestFixture(test_knx_addr_group_to_str__null),
        new_TestFixture(test_knx_addr_group_to_str__null_addr),
        new_TestFixture(test_knx_addr_group2_to_str__valid),
        new_TestFixture(test_knx_addr_group2_to_str__too_short),
        new_TestFixture(test_knx_addr_group2_to_str__null),
        new_TestFixture(test_knx_addr_group2_to_str__null_addr),
    };

    EMB_UNIT_TESTCALLER(knx_addr_tests, NULL, NULL, fixtures);

    return (Test *)&knx_addr_tests;
}
