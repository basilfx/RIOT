/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 */
#include <errno.h>
#include <string.h>

#include "embUnit/embUnit.h"

#include "net/ipv4/addr.h"
#include "net/l2util.h"
#include "net/netdev.h"

#include "tests-l2util.h"

static void test_l2util_ipv4_group_to_l2_group__ethernet_all_hosts(void)
{
    ipv4_addr_t group = { .u8 = { 224, 0, 0, 1 } };
    uint8_t l2_group[6];
    uint8_t expect[6] = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x01 };

    TEST_ASSERT_EQUAL_INT(6, l2util_ipv4_group_to_l2_group(NETDEV_TYPE_ETHERNET,
                                                           &group, l2_group));
    TEST_ASSERT_MESSAGE(memcmp(expect, l2_group, sizeof(expect)) == 0,
                        "unexpected L2 group address");
}

static void test_l2util_ipv4_group_to_l2_group__ethernet_high_bit_masked(void)
{
    /* RFC 1112: only the low-order 23 bits of the group address go into the
     * MAC address, so the high bit of the 2nd octet must be masked off */
    ipv4_addr_t group = { .u8 = { 239, 255, 255, 255 } };
    uint8_t l2_group[6];
    uint8_t expect[6] = { 0x01, 0x00, 0x5e, 0x7f, 0xff, 0xff };

    TEST_ASSERT_EQUAL_INT(6, l2util_ipv4_group_to_l2_group(NETDEV_TYPE_ETHERNET,
                                                           &group, l2_group));
    TEST_ASSERT_MESSAGE(memcmp(expect, l2_group, sizeof(expect)) == 0,
                        "unexpected L2 group address");
}

static void test_l2util_ipv4_group_to_l2_group__unsupported_device(void)
{
    ipv4_addr_t group = { .u8 = { 224, 0, 0, 1 } };
    uint8_t l2_group[6];

    TEST_ASSERT_EQUAL_INT(-ENOTSUP,
                          l2util_ipv4_group_to_l2_group(NETDEV_TYPE_UNKNOWN,
                                                        &group, l2_group));
}

Test *tests_l2util_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_l2util_ipv4_group_to_l2_group__ethernet_all_hosts),
        new_TestFixture(test_l2util_ipv4_group_to_l2_group__ethernet_high_bit_masked),
        new_TestFixture(test_l2util_ipv4_group_to_l2_group__unsupported_device),
    };

    EMB_UNIT_TESTCALLER(l2util_tests, NULL, NULL, fixtures);

    return (Test *)&l2util_tests;
}

void tests_l2util(void)
{
    TESTS_RUN(tests_l2util_tests());
}
/** @} */
