/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "embUnit.h"

#include "net/nrfnet.h"

#include "tests-nrfnet.h"

static void test_nrfnet_addr_info_init(void)
{
    nrfnet_addr_t addr;
    nrfnet_addr_info_t addr_info;

    addr.u16 = byteorder_htons(00);
    nrfnet_addr_info_init(&addr_info, &addr);
    TEST_ASSERT_EQUAL_INT(0, addr_info.level);
    TEST_ASSERT_EQUAL_INT(00, byteorder_ntohs(addr_info.addr.u16));
    TEST_ASSERT_EQUAL_INT(00, byteorder_ntohs(addr_info.mask.u16));
    TEST_ASSERT_EQUAL_INT(00, byteorder_ntohs(addr_info.parent_addr.u16));
    TEST_ASSERT_EQUAL_INT(0, addr_info.parent_pipe);

    addr.u16 = byteorder_htons(01);
    nrfnet_addr_info_init(&addr_info, &addr);
    TEST_ASSERT_EQUAL_INT(1, addr_info.level);
    TEST_ASSERT_EQUAL_INT(01, byteorder_ntohs(addr_info.addr.u16));
    TEST_ASSERT_EQUAL_INT(07, byteorder_ntohs(addr_info.mask.u16));
    TEST_ASSERT_EQUAL_INT(00, byteorder_ntohs(addr_info.parent_addr.u16));
    TEST_ASSERT_EQUAL_INT(1, addr_info.parent_pipe);

    addr.u16 = byteorder_htons(021);
    nrfnet_addr_info_init(&addr_info, &addr);
    TEST_ASSERT_EQUAL_INT(2, addr_info.level);
    TEST_ASSERT_EQUAL_INT(021, byteorder_ntohs(addr_info.addr.u16));
    TEST_ASSERT_EQUAL_INT(077, byteorder_ntohs(addr_info.mask.u16));
    TEST_ASSERT_EQUAL_INT(01, byteorder_ntohs(addr_info.parent_addr.u16));
    TEST_ASSERT_EQUAL_INT(2, addr_info.parent_pipe);

    addr.u16 = byteorder_htons(0321);
    nrfnet_addr_info_init(&addr_info, &addr);
    TEST_ASSERT_EQUAL_INT(3, addr_info.level);
    TEST_ASSERT_EQUAL_INT(0321, byteorder_ntohs(addr_info.addr.u16));
    TEST_ASSERT_EQUAL_INT(0777, byteorder_ntohs(addr_info.mask.u16));
    TEST_ASSERT_EQUAL_INT(021, byteorder_ntohs(addr_info.parent_addr.u16));
    TEST_ASSERT_EQUAL_INT(3, addr_info.parent_pipe);

    addr.u16 = byteorder_htons(05555);
    nrfnet_addr_info_init(&addr_info, &addr);
    TEST_ASSERT_EQUAL_INT(4, addr_info.level);
    TEST_ASSERT_EQUAL_INT(05555, byteorder_ntohs(addr_info.addr.u16));
    TEST_ASSERT_EQUAL_INT(07777, byteorder_ntohs(addr_info.mask.u16));
    TEST_ASSERT_EQUAL_INT(0555, byteorder_ntohs(addr_info.parent_addr.u16));
    TEST_ASSERT_EQUAL_INT(5, addr_info.parent_pipe);
}

static void test_nrfnet_addr_info_is_descendant(void)
{
    nrfnet_addr_t addr;
    nrfnet_addr_t addr_descendant_a;
    nrfnet_addr_t addr_descendant_b;
    nrfnet_addr_t addr_parent;
    nrfnet_addr_info_t addr_info;

    addr.u16 = byteorder_htons(021);
    addr_descendant_a.u16 = byteorder_htons(0321);
    addr_descendant_b.u16 = byteorder_htons(04321);
    addr_parent.u16 = byteorder_htons(01);
    nrfnet_addr_info_init(&addr_info, &addr);

    TEST_ASSERT(nrfnet_addr_info_is_descendant(&addr_info,
                                               &addr_descendant_a) == true);
    TEST_ASSERT(nrfnet_addr_info_is_descendant(&addr_info,
                                               &addr_descendant_b) == true);
    TEST_ASSERT(nrfnet_addr_info_is_descendant(&addr_info,
                                               &addr_parent) == false);
}

static void test_nrfnet_addr_info_is_child(void)
{
    nrfnet_addr_t addr;
    nrfnet_addr_t addr_descendant_a;
    nrfnet_addr_t addr_descendant_b;
    nrfnet_addr_t addr_parent;
    nrfnet_addr_info_t addr_info;

    addr.u16 = byteorder_htons(021);
    addr_descendant_a.u16 = byteorder_htons(0321);
    addr_descendant_b.u16 = byteorder_htons(04321);
    addr_parent.u16 = byteorder_htons(01);
    nrfnet_addr_info_init(&addr_info, &addr);

    TEST_ASSERT(nrfnet_addr_info_is_child(&addr_info,
                                          &addr_descendant_a) == true);
    TEST_ASSERT(nrfnet_addr_info_is_child(&addr_info,
                                          &addr_descendant_b) == false);
    TEST_ASSERT(nrfnet_addr_info_is_child(&addr_info, &addr_parent) == false);
}

static void test_nrfnet_addr_info_child(void)
{
    nrfnet_addr_t addr;
    nrfnet_addr_t addr_child;
    nrfnet_addr_t addr_descendant;
    nrfnet_addr_info_t addr_info;

    addr.u16 = byteorder_htons(021);
    addr_descendant.u16 = byteorder_htons(04321);
    nrfnet_addr_info_init(&addr_info, &addr);

    nrfnet_addr_info_child(&addr_child, &addr_info, &addr_descendant);

    TEST_ASSERT_EQUAL_INT(0321, byteorder_ntohs(addr_child.u16));
}

Test *tests_nrfnet_addr_info_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_nrfnet_addr_info_init),
        new_TestFixture(test_nrfnet_addr_info_is_descendant),
        new_TestFixture(test_nrfnet_addr_info_is_child),
        new_TestFixture(test_nrfnet_addr_info_child)
    };

    EMB_UNIT_TESTCALLER(tests_nrfnet_addr_info_tests,
                        NULL,
                        NULL,
                        fixtures);

    return (Test *)&tests_nrfnet_addr_info_tests;
}
