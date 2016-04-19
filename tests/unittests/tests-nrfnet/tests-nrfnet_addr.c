/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/* clib includes */

#include "embUnit.h"

#include "net/nrfnet.h"

#include "tests-nrfnet.h"

static void test_nrfnet_addr_equal__basic(void)
{
    nrfnet_addr_t a;
    nrfnet_addr_t b;
    nrfnet_addr_t c;

    a.u16 = byteorder_htons(100);
    b.u16 = byteorder_htons(100);
    c.u16 = byteorder_htons(200);

    TEST_ASSERT(nrfnet_addr_equal(&a, &b) == true);
    TEST_ASSERT(nrfnet_addr_equal(&b, &a) == true);
    TEST_ASSERT(nrfnet_addr_equal(&a, &c) == false);
    TEST_ASSERT(nrfnet_addr_equal(&b, &c) == false);
}

static void tets_nrfnet_addr_equal__nulls(void)
{
    nrfnet_addr_t a;

    a.u16 = byteorder_htons(100);

    TEST_ASSERT(nrfnet_addr_equal(&a, NULL) == false);
    TEST_ASSERT(nrfnet_addr_equal(NULL, &a) == false);
    TEST_ASSERT(nrfnet_addr_equal(NULL, NULL) == false);
}

static void test_nrfnet_addr_is_valid(void)
{
    nrfnet_addr_t addr;

    addr.u16 = byteorder_htons(00);
    TEST_ASSERT(nrfnet_addr_is_valid(&addr) == true);

    addr.u16 = byteorder_htons(01);
    TEST_ASSERT(nrfnet_addr_is_valid(&addr) == true);

    addr.u16 = byteorder_htons(021);
    TEST_ASSERT(nrfnet_addr_is_valid(&addr) == true);

    addr.u16 = byteorder_htons(0321);
    TEST_ASSERT(nrfnet_addr_is_valid(&addr) == true);

    addr.u16 = byteorder_htons(05555);
    TEST_ASSERT(nrfnet_addr_is_valid(&addr) == true);

    addr.u16 = byteorder_htons(05556);
    TEST_ASSERT(nrfnet_addr_is_valid(&addr) == false);
}

static void test_nrfnet_addr_to_str__valid(void)
{
    char actual[NRFNET_ADDR_MAX_STR_LEN] = "-----";
    nrfnet_addr_t addr;

    addr.u16 = byteorder_htons(00);
    nrfnet_addr_to_str(actual, &addr, NRFNET_ADDR_MAX_STR_LEN);
    TEST_ASSERT_EQUAL_STRING("00", (char *) actual);

    addr.u16 = byteorder_htons(01);
    nrfnet_addr_to_str(actual, &addr, NRFNET_ADDR_MAX_STR_LEN);
    TEST_ASSERT_EQUAL_STRING("01", (char *) actual);

    addr.u16 = byteorder_htons(021);
    nrfnet_addr_to_str(actual, &addr, NRFNET_ADDR_MAX_STR_LEN);
    TEST_ASSERT_EQUAL_STRING("021", (char *) actual);

    addr.u16 = byteorder_htons(0321);
    nrfnet_addr_to_str(actual, &addr, NRFNET_ADDR_MAX_STR_LEN);
    TEST_ASSERT_EQUAL_STRING("0321", (char *) actual);

    addr.u16 = byteorder_htons(05555);
    nrfnet_addr_to_str(actual, &addr, NRFNET_ADDR_MAX_STR_LEN);
    TEST_ASSERT_EQUAL_STRING("05555", (char *) actual);
}

static void test_nrfnet_addr_to_str__invalid(void)
{
    char actual[NRFNET_ADDR_MAX_STR_LEN] = "-----";
    nrfnet_addr_t addr;

    TEST_ASSERT_NULL(nrfnet_addr_to_str(actual, &addr, NRFNET_ADDR_MAX_STR_LEN - 1));
    TEST_ASSERT_NULL(nrfnet_addr_to_str(actual, &addr, 0));
    TEST_ASSERT_NULL(nrfnet_addr_to_str(actual, NULL, NRFNET_ADDR_MAX_STR_LEN));
    TEST_ASSERT_NULL(nrfnet_addr_to_str(NULL, &addr, NRFNET_ADDR_MAX_STR_LEN));
    TEST_ASSERT_NULL(nrfnet_addr_to_str(NULL, NULL, NRFNET_ADDR_MAX_STR_LEN));
}

static void test_nrfnet_addr_from_str__valid(void)
{
    nrfnet_addr_t addr;

    nrfnet_addr_from_str(&addr, "00");
    TEST_ASSERT_EQUAL_INT(00, byteorder_ntohs(addr.u16));

    nrfnet_addr_from_str(&addr, "01");
    TEST_ASSERT_EQUAL_INT(01, byteorder_ntohs(addr.u16));

    nrfnet_addr_from_str(&addr, "021");
    TEST_ASSERT_EQUAL_INT(021, byteorder_ntohs(addr.u16));

    nrfnet_addr_from_str(&addr, "0321");
    TEST_ASSERT_EQUAL_INT(0321, byteorder_ntohs(addr.u16));

    nrfnet_addr_from_str(&addr, "05555");
    TEST_ASSERT_EQUAL_INT(05555, byteorder_ntohs(addr.u16));
}

static void test_nrfnet_addr_from_str__invalid(void)
{
    nrfnet_addr_t addr;

    TEST_ASSERT_NULL(nrfnet_addr_from_str(&addr, "055555"));
    TEST_ASSERT_NULL(nrfnet_addr_from_str(&addr, "100"));
    TEST_ASSERT_NULL(nrfnet_addr_from_str(&addr, NULL));
    TEST_ASSERT_NULL(nrfnet_addr_from_str(NULL, NULL));
}

Test *tests_nrfnet_addr_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_nrfnet_addr_equal__basic),
        new_TestFixture(tets_nrfnet_addr_equal__nulls),
        new_TestFixture(test_nrfnet_addr_is_valid),
        new_TestFixture(test_nrfnet_addr_to_str__valid),
        new_TestFixture(test_nrfnet_addr_to_str__invalid),
        new_TestFixture(test_nrfnet_addr_from_str__valid),
        new_TestFixture(test_nrfnet_addr_from_str__invalid)
    };

    EMB_UNIT_TESTCALLER(tests_nrfnet_addr_tests,
                        NULL,
                        NULL,
                        fixtures);

    return (Test *)&tests_nrfnet_addr_tests;
}
