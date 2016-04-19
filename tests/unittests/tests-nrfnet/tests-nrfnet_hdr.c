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

static void test_nrfnet_hdr_fragments(void)
{
    nrfnet_hdr_t hdr;

    hdr.length = 128;
    TEST_ASSERT_EQUAL_INT(6, nrfnet_hdr_fragments(&hdr));

    hdr.length = NRFNET_PAYLOAD_SIZE;
    TEST_ASSERT_EQUAL_INT(2, nrfnet_hdr_fragments(&hdr));

    hdr.length = NRFNET_DATA_SIZE + 1;
    TEST_ASSERT_EQUAL_INT(2, nrfnet_hdr_fragments(&hdr));

    hdr.length = NRFNET_DATA_SIZE;
    TEST_ASSERT_EQUAL_INT(1, nrfnet_hdr_fragments(&hdr));

    hdr.length = NRFNET_DATA_SIZE - 1;
    TEST_ASSERT_EQUAL_INT(1, nrfnet_hdr_fragments(&hdr));
}

static void test_nrfnet_hdr_is_fragmented(void)
{
    nrfnet_hdr_t hdr;

    hdr.length = 128;
    TEST_ASSERT(nrfnet_hdr_is_fragmented(&hdr) == true);

    hdr.length = NRFNET_PAYLOAD_SIZE;
    TEST_ASSERT(nrfnet_hdr_is_fragmented(&hdr) == true);

    hdr.length = NRFNET_DATA_SIZE + 1;
    TEST_ASSERT(nrfnet_hdr_is_fragmented(&hdr) == true);

    hdr.length = NRFNET_DATA_SIZE;
    TEST_ASSERT(nrfnet_hdr_is_fragmented(&hdr) == false);

    hdr.length = NRFNET_DATA_SIZE - 1;
    TEST_ASSERT(nrfnet_hdr_is_fragmented(&hdr) == false);
}

static void test_nrfnet_hdr_is_valid__valid(void)
{
    nrfnet_hdr_t hdr;

    hdr.from.u16 = byteorder_htons(04321);
    hdr.to.u16 = byteorder_htons(0321);
    hdr.id = 1337;
    hdr.length = 192;
    hdr.offset = 5;
    hdr.next = 4;

    TEST_ASSERT(nrfnet_hdr_is_valid(&hdr) == true);
}

static void test_nrfnet_hdr_is_valid__invalid_address(void)
{
    nrfnet_hdr_t hdr;

    hdr.from.u16 = byteorder_htons(07777);
    hdr.to.u16 = byteorder_htons(07777);
    hdr.id = 1337;
    hdr.length = 192;
    hdr.offset = 5;
    hdr.next = 4;

    TEST_ASSERT(nrfnet_hdr_is_valid(&hdr) == false);
}

static void test_nrfnet_hdr_is_valid__invalid_length(void)
{
    nrfnet_hdr_t hdr;

    hdr.from.u16 = byteorder_htons(04321);
    hdr.to.u16 = byteorder_htons(0321);
    hdr.id = 1337;
    hdr.length = 0;
    hdr.offset = 5;
    hdr.next = 4;

    TEST_ASSERT(nrfnet_hdr_is_valid(&hdr) == false);
}

static void test_nrfnet_hdr_is_valid__invalid_offset(void)
{
    nrfnet_hdr_t hdr;

    hdr.from.u16 = byteorder_htons(04321);
    hdr.to.u16 = byteorder_htons(0321);
    hdr.id = 1337;
    hdr.length = 192;
    hdr.offset = 32;
    hdr.next = 4;

    TEST_ASSERT(nrfnet_hdr_is_valid(&hdr) == false);
}

static void test_nrfnet_hdr_is_valid__invalid_next(void)
{
    nrfnet_hdr_t hdr;

    hdr.from.u16 = byteorder_htons(04321);
    hdr.to.u16 = byteorder_htons(0321);
    hdr.id = 1337;
    hdr.length = 192;
    hdr.offset = 5;
    hdr.next = 32;

    TEST_ASSERT(nrfnet_hdr_is_valid(&hdr) == false);
}

static void test_nrfnet_hdr_uncompress(void)
{
    nrfnet_hdr_t hdr;
    nrfnet_comp_hdr_t hdr_comp;

    hdr_comp.from = 2925;
    hdr_comp.to = 2924;
    hdr_comp.id = 1337;
    hdr_comp.length = 192;
    hdr_comp.offset = 5;
    hdr_comp.next = 4;

    nrfnet_hdr_uncompress(&hdr, &hdr_comp);

    TEST_ASSERT_EQUAL_INT(hdr.from.u16.u16, 2925);
    TEST_ASSERT_EQUAL_INT(hdr.to.u16.u16, 2924);
    TEST_ASSERT_EQUAL_INT(hdr.id, 1337);
    TEST_ASSERT_EQUAL_INT(hdr.length, 192);
    TEST_ASSERT_EQUAL_INT(hdr.offset, 5);
    TEST_ASSERT_EQUAL_INT(hdr.next, 4);
}

static void test_nrfnet_hdr_compress(void)
{
    nrfnet_hdr_t hdr;
    nrfnet_comp_hdr_t hdr_comp;

    hdr.from.u16.u16 = 2925;
    hdr.to.u16.u16 = 2924;
    hdr.id = 1337;
    hdr.length = 192;
    hdr.offset = 5;
    hdr.next = 4;

    nrfnet_hdr_compress(&hdr_comp, &hdr);

    TEST_ASSERT_EQUAL_INT(2925, hdr_comp.from);
    TEST_ASSERT_EQUAL_INT(2924, hdr_comp.to);
    TEST_ASSERT_EQUAL_INT(1337, hdr_comp.id);
    TEST_ASSERT_EQUAL_INT(192, hdr_comp.length);
    TEST_ASSERT_EQUAL_INT(5, hdr_comp.offset);
    TEST_ASSERT_EQUAL_INT(4, hdr_comp.next);
    TEST_ASSERT_EQUAL_INT(0, hdr_comp.reserved);
}

Test *tests_nrfnet_hdr_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_nrfnet_hdr_fragments),
        new_TestFixture(test_nrfnet_hdr_is_fragmented),
        new_TestFixture(test_nrfnet_hdr_is_valid__valid),
        new_TestFixture(test_nrfnet_hdr_is_valid__invalid_address),
        new_TestFixture(test_nrfnet_hdr_is_valid__invalid_length),
        new_TestFixture(test_nrfnet_hdr_is_valid__invalid_offset),
        new_TestFixture(test_nrfnet_hdr_is_valid__invalid_next),
        new_TestFixture(test_nrfnet_hdr_uncompress),
        new_TestFixture(test_nrfnet_hdr_compress)
    };

    EMB_UNIT_TESTCALLER(tests_nrfnet_hdr_tests,
                        NULL,
                        NULL,
                        fixtures);

    return (Test *)&tests_nrfnet_hdr_tests;
}
