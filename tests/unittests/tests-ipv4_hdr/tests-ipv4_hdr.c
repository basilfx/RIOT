/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 */
#include <stdint.h>

#include "embUnit.h"

#include "net/ipv4/hdr.h"

#include "unittests-constants.h"
#include "tests-ipv4_hdr.h"

static void test_ipv4_hdr_set_version(void)
{
    uint8_t val[sizeof(ipv4_hdr_t)] = { TEST_UINT8 };

    ipv4_hdr_set_version((ipv4_hdr_t *)val);

    TEST_ASSERT_EQUAL_INT(0x40, val[0] & 0xf0);
    TEST_ASSERT_EQUAL_INT(TEST_UINT8 & 0x0f, val[0] & 0x0f);
}

static void test_ipv4_hdr_get_version(void)
{
    uint8_t val[sizeof(ipv4_hdr_t)] = { TEST_UINT8 };

    TEST_ASSERT_EQUAL_INT(TEST_UINT8 >> 4, ipv4_hdr_get_version((ipv4_hdr_t *)val));
}

static void test_ipv4_hdr_set_ihl(void)
{
    uint8_t val[sizeof(ipv4_hdr_t)] = { TEST_UINT8 };

    /* minimum internet header length */
    ipv4_hdr_set_ihl((ipv4_hdr_t *)val, 20);

    TEST_ASSERT_EQUAL_INT(TEST_UINT8 & 0xf0, val[0] & 0xf0);
    TEST_ASSERT_EQUAL_INT(5, val[0] & 0x0f);

    /* maximum internet header length */
    ipv4_hdr_set_ihl((ipv4_hdr_t *)val, 60);

    TEST_ASSERT_EQUAL_INT(TEST_UINT8 & 0xf0, val[0] & 0xf0);
    TEST_ASSERT_EQUAL_INT(15, val[0] & 0x0f);
}

static void test_ipv4_hdr_get_ihl(void)
{
    uint8_t val[sizeof(ipv4_hdr_t)];

    /* minimum internet header length */
    val[0] = 0x45;

    TEST_ASSERT_EQUAL_INT(20, ipv4_hdr_get_ihl((ipv4_hdr_t *)val));

    /* maximum internet header length */
    val[0] = 0x4f;

    TEST_ASSERT_EQUAL_INT(60, ipv4_hdr_get_ihl((ipv4_hdr_t *)val));
}

Test *tests_ipv4_hdr_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_ipv4_hdr_set_version),
        new_TestFixture(test_ipv4_hdr_get_version),
        new_TestFixture(test_ipv4_hdr_set_ihl),
        new_TestFixture(test_ipv4_hdr_get_ihl),
    };

    EMB_UNIT_TESTCALLER(ipv4_hdr_tests, NULL, NULL, fixtures);

    return (Test *)&ipv4_hdr_tests;
}

void tests_ipv4_hdr(void)
{
    TESTS_RUN(tests_ipv4_hdr_tests());
}
/** @} */
