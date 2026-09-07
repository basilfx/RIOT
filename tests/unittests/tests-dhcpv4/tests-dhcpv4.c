/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 */
#include <string.h>

#include "embUnit/embUnit.h"

#include "byteorder.h"
#include "net/dhcpv4.h"

#include "tests-dhcpv4.h"

static void test_dhcpv4_opt_add__single(void)
{
    uint8_t buf[8] = { 0 };
    uint8_t value = 42;
    size_t len = dhcpv4_opt_add(buf, DHCPV4_OPT_MSG_TYPE, &value, sizeof(value));

    TEST_ASSERT_EQUAL_INT(3, len);
    TEST_ASSERT_EQUAL_INT(DHCPV4_OPT_MSG_TYPE, buf[0]);
    TEST_ASSERT_EQUAL_INT(1, buf[1]);
    TEST_ASSERT_EQUAL_INT(42, buf[2]);
}

static void test_dhcpv4_opt_add__zero_length(void)
{
    uint8_t buf[8] = { 0xff, 0xff, 0xff };
    size_t len = dhcpv4_opt_add(buf, DHCPV4_OPT_END, NULL, 0);

    TEST_ASSERT_EQUAL_INT(2, len);
    TEST_ASSERT_EQUAL_INT(DHCPV4_OPT_END, buf[0]);
    TEST_ASSERT_EQUAL_INT(0, buf[1]);
}

static void test_dhcpv4_opt_get__found(void)
{
    uint8_t buf[16];
    size_t off = 0;
    uint32_t lease_time = byteorder_htonl(3600).u32;
    uint8_t len;
    const uint8_t *val;

    off += dhcpv4_opt_add(&buf[off], DHCPV4_OPT_LEASE_TIME, &lease_time, 4);
    off += dhcpv4_opt_add(&buf[off], DHCPV4_OPT_END, NULL, 0);

    val = dhcpv4_opt_get(buf, off, DHCPV4_OPT_LEASE_TIME, &len);
    TEST_ASSERT_NOT_NULL(val);
    TEST_ASSERT_EQUAL_INT(4, len);
    TEST_ASSERT(memcmp(val, &lease_time, 4) == 0);
}

static void test_dhcpv4_opt_get__not_found(void)
{
    uint8_t buf[16];
    size_t off = 0;
    uint8_t mask[4] = { 255, 255, 255, 0 };
    uint8_t len;

    off += dhcpv4_opt_add(&buf[off], DHCPV4_OPT_SUBNET_MASK, mask, 4);
    off += dhcpv4_opt_add(&buf[off], DHCPV4_OPT_END, NULL, 0);

    TEST_ASSERT_NULL(dhcpv4_opt_get(buf, off, DHCPV4_OPT_ROUTER, &len));
}

static void test_dhcpv4_opt_get__skips_pad(void)
{
    uint8_t buf[16] = { DHCPV4_OPT_PAD, DHCPV4_OPT_PAD };
    size_t off = 2;
    uint8_t value = 3;
    uint8_t len;
    const uint8_t *val;

    off += dhcpv4_opt_add(&buf[off], DHCPV4_OPT_MSG_TYPE, &value, 1);
    off += dhcpv4_opt_add(&buf[off], DHCPV4_OPT_END, NULL, 0);

    val = dhcpv4_opt_get(buf, off, DHCPV4_OPT_MSG_TYPE, &len);
    TEST_ASSERT_NOT_NULL(val);
    TEST_ASSERT_EQUAL_INT(1, len);
    TEST_ASSERT_EQUAL_INT(3, val[0]);
}

static void test_dhcpv4_opt_get__stops_at_end(void)
{
    uint8_t buf[16];
    size_t off = 0;
    uint8_t value = 7;
    uint8_t len;

    off += dhcpv4_opt_add(&buf[off], DHCPV4_OPT_END, NULL, 0);
    /* anything after DHCPV4_OPT_END must be ignored */
    off += dhcpv4_opt_add(&buf[off], DHCPV4_OPT_MSG_TYPE, &value, 1);

    TEST_ASSERT_NULL(dhcpv4_opt_get(buf, off, DHCPV4_OPT_MSG_TYPE, &len));
}

static void test_dhcpv4_opt_get__truncated_length_byte_missing(void)
{
    uint8_t buf[1] = { DHCPV4_OPT_MSG_TYPE };
    uint8_t len;

    TEST_ASSERT_NULL(dhcpv4_opt_get(buf, sizeof(buf), DHCPV4_OPT_MSG_TYPE, &len));
}

static void test_dhcpv4_opt_get__truncated_value_overflows_buffer(void)
{
    uint8_t buf[3] = { DHCPV4_OPT_LEASE_TIME, 4, 0 };
    uint8_t len;

    TEST_ASSERT_NULL(dhcpv4_opt_get(buf, sizeof(buf), DHCPV4_OPT_LEASE_TIME, &len));
}

Test *tests_dhcpv4_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_dhcpv4_opt_add__single),
        new_TestFixture(test_dhcpv4_opt_add__zero_length),
        new_TestFixture(test_dhcpv4_opt_get__found),
        new_TestFixture(test_dhcpv4_opt_get__not_found),
        new_TestFixture(test_dhcpv4_opt_get__skips_pad),
        new_TestFixture(test_dhcpv4_opt_get__stops_at_end),
        new_TestFixture(test_dhcpv4_opt_get__truncated_length_byte_missing),
        new_TestFixture(test_dhcpv4_opt_get__truncated_value_overflows_buffer),
    };

    EMB_UNIT_TESTCALLER(dhcpv4_tests, NULL, NULL, fixtures);

    return (Test *)&dhcpv4_tests;
}

void tests_dhcpv4(void)
{
    TESTS_RUN(tests_dhcpv4_tests());
}
/** @} */
