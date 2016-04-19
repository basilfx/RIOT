/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "embUnit.h"

#include "net/nrfnet.h"
#include "net/nrfnet_rbuf.h"

#include "tests-nrfnet.h"

static void test_nrfnet_rbuf_init(void)
{
    nrfnet_rbuf_t rbuf;
    nrfnet_rbuf_head_t heads[8];
    nrfnet_rbuf_hole_t holes[16];

    rbuf.heads = heads;
    rbuf.holes = holes;
    rbuf.head_count = 8;
    rbuf.hole_count = 16;

    nrfnet_rbuf_init(&rbuf);

    for (uint32_t i = 0; i < rbuf.head_count; i++) {
        TEST_ASSERT(rbuf.heads[i].used == false);
        TEST_ASSERT(rbuf.heads[i].next == NULL);
    }

    for (uint32_t i = 0; i < rbuf.hole_count; i++) {
        TEST_ASSERT(rbuf.holes[i].used == false);
        TEST_ASSERT(rbuf.holes[i].next == NULL);
    }
}

static void test_nrfnet_rbuf_add__single(void)
{
    nrfnet_rbuf_t rbuf;
    nrfnet_rbuf_head_t heads[2];
    nrfnet_rbuf_hole_t holes[4];
    nrfnet_rbuf_head_t* result;
    nrfnet_hdr_t hdr;
    uint8_t data[64];

    rbuf.heads = heads;
    rbuf.holes = holes;
    rbuf.head_count = 2;
    rbuf.hole_count = 4;

    nrfnet_rbuf_init(&rbuf);

    hdr.from.u16.u16 = 1;
    hdr.to.u16.u16 = 2;
    hdr.id = 123;
    hdr.length = 64;
    hdr.offset = 0;
    hdr.next = 1;

    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 0);

    TEST_ASSERT(result == &heads[0]);
    TEST_ASSERT(result->next == &holes[0]);
    TEST_ASSERT(result->used == true);
    TEST_ASSERT_EQUAL_INT(hdr.from.u16.u16, result->hdr.from.u16.u16);
    TEST_ASSERT_EQUAL_INT(hdr.to.u16.u16, result->hdr.to.u16.u16);
    TEST_ASSERT_EQUAL_INT(hdr.id, result->hdr.id);
    TEST_ASSERT_EQUAL_INT(hdr.length, result->hdr.length);
    TEST_ASSERT_EQUAL_INT(hdr.offset, result->hdr.offset);
    TEST_ASSERT_EQUAL_INT(hdr.next, result->hdr.next);
}

static void test_nrfnet_rbuf_add__continue(void)
{
    nrfnet_rbuf_t rbuf;
    nrfnet_rbuf_head_t heads[2];
    nrfnet_rbuf_hole_t holes[4];
    nrfnet_rbuf_head_t* result;
    nrfnet_hdr_t hdr;
    uint8_t data[64];

    rbuf.heads = heads;
    rbuf.holes = holes;
    rbuf.head_count = 2;
    rbuf.hole_count = 4;

    nrfnet_rbuf_init(&rbuf);

    hdr.from.u16.u16 = 1;
    hdr.to.u16.u16 = 2;
    hdr.id = 123;
    hdr.next = 1;
    hdr.length = 64;

    hdr.offset = 0;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 0);

    TEST_ASSERT(&heads[0] == result);
    TEST_ASSERT(holes[0].used == true);
    TEST_ASSERT(holes[1].used == false);
    TEST_ASSERT(holes[2].used == false);
    TEST_ASSERT(holes[3].used == false);

    hdr.offset = 10;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 1);

    TEST_ASSERT(result == &heads[0]);
    TEST_ASSERT(holes[0].used == true);
    TEST_ASSERT(holes[1].used == true);
    TEST_ASSERT(holes[2].used == false);
    TEST_ASSERT(holes[3].used == false);

    hdr.offset = 20;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 2);

    TEST_ASSERT(&heads[0] == result);
    TEST_ASSERT(holes[0].used == true);
    TEST_ASSERT(holes[1].used == true);
    TEST_ASSERT(holes[2].used == true);
    TEST_ASSERT(holes[3].used == false);

    TEST_ASSERT_EQUAL_INT(0, holes[0].offset);
    TEST_ASSERT_EQUAL_INT(10, holes[1].offset);
    TEST_ASSERT_EQUAL_INT(20, holes[2].offset);

    TEST_ASSERT_EQUAL_INT(0, heads[0].next->offset);
    TEST_ASSERT_EQUAL_INT(10, heads[0].next->next->offset);
    TEST_ASSERT_EQUAL_INT(20, heads[0].next->next->next->offset);
    TEST_ASSERT_NULL(heads[0].next->next->next->next);
}

static void test_nrfnet_rbuf_add__continue_reverse(void)
{
    nrfnet_rbuf_t rbuf;
    nrfnet_rbuf_head_t heads[2];
    nrfnet_rbuf_hole_t holes[4];
    nrfnet_rbuf_head_t* result;
    nrfnet_hdr_t hdr;
    uint8_t data[64];

    rbuf.heads = heads;
    rbuf.holes = holes;
    rbuf.head_count = 2;
    rbuf.hole_count = 4;

    nrfnet_rbuf_init(&rbuf);

    hdr.from.u16.u16 = 1;
    hdr.to.u16.u16 = 2;
    hdr.id = 123;
    hdr.next = 1;
    hdr.length = 64;

    hdr.offset = 20;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 0);

    hdr.offset = 10;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 1);

    hdr.offset = 0;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 2);

    TEST_ASSERT(&heads[0] == result);

    TEST_ASSERT_EQUAL_INT(20, holes[0].offset);
    TEST_ASSERT_EQUAL_INT(10, holes[1].offset);
    TEST_ASSERT_EQUAL_INT(0, holes[2].offset);

    TEST_ASSERT_EQUAL_INT(0, heads[0].next->offset);
    TEST_ASSERT_EQUAL_INT(10, heads[0].next->next->offset);
    TEST_ASSERT_EQUAL_INT(20, heads[0].next->next->next->offset);
    TEST_ASSERT_NULL(heads[0].next->next->next->next);
}

static void test_nrfnet_rbuf_add__new_packet(void)
{
    nrfnet_rbuf_t rbuf;
    nrfnet_rbuf_head_t heads[2];
    nrfnet_rbuf_hole_t holes[4];
    nrfnet_rbuf_head_t* result;
    nrfnet_hdr_t hdr;
    uint8_t data[64];

    rbuf.heads = heads;
    rbuf.holes = holes;
    rbuf.head_count = 2;
    rbuf.hole_count = 4;

    nrfnet_rbuf_init(&rbuf);

    hdr.from.u16.u16 = 1;
    hdr.to.u16.u16 = 2;
    hdr.next = 1;
    hdr.length = 64;
    hdr.offset = 0;

    hdr.id = 123;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 0);

    TEST_ASSERT(result == &heads[0]);
    TEST_ASSERT(holes[0].used == true);
    TEST_ASSERT(holes[1].used == false);
    TEST_ASSERT(holes[2].used == false);
    TEST_ASSERT(holes[3].used == false);

    hdr.id = 456;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 1);

    TEST_ASSERT(result == &heads[1]);
    TEST_ASSERT(holes[0].used == true);
    TEST_ASSERT(holes[1].used == true);
    TEST_ASSERT(holes[2].used == false);
    TEST_ASSERT(holes[3].used == false);
}

static void test_nrfnet_rbuf_add__no_free_heads(void)
{
    nrfnet_rbuf_t rbuf;
    nrfnet_rbuf_head_t heads[1];
    nrfnet_rbuf_hole_t holes[2];
    nrfnet_rbuf_head_t* result;
    nrfnet_hdr_t hdr;
    uint8_t data[64];

    rbuf.heads = heads;
    rbuf.holes = holes;
    rbuf.head_count = 1;
    rbuf.hole_count = 2;

    nrfnet_rbuf_init(&rbuf);

    hdr.from.u16.u16 = 1;
    hdr.to.u16.u16 = 2;
    hdr.next = 1;
    hdr.length = 64;
    hdr.offset = 0;

    hdr.id = 123;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 0);

    TEST_ASSERT(result == &heads[0]);

    hdr.id = 456;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 1);

    TEST_ASSERT(result == NULL);
}

static void test_nrfnet_rbuf_add__no_free_holes(void)
{
    nrfnet_rbuf_t rbuf;
    nrfnet_rbuf_head_t heads[2];
    nrfnet_rbuf_hole_t holes[1];
    nrfnet_rbuf_head_t* result;
    nrfnet_hdr_t hdr;
    uint8_t data[64];

    rbuf.heads = heads;
    rbuf.holes = holes;
    rbuf.head_count = 2;
    rbuf.hole_count = 1;

    nrfnet_rbuf_init(&rbuf);

    hdr.from.u16.u16 = 1;
    hdr.to.u16.u16 = 2;
    hdr.next = 1;
    hdr.id = 123;
    hdr.length = 64;

    hdr.offset = 0;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 0);

    TEST_ASSERT(result == &heads[0]);
    TEST_ASSERT_EQUAL_INT(2, heads[0].fragments);

    hdr.offset = 10;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 1);

    TEST_ASSERT(result == NULL);
    TEST_ASSERT_EQUAL_INT(2, heads[0].fragments);
}

static void test_nrfnet_rbuf_gc(void)
{
    nrfnet_rbuf_t rbuf;
    nrfnet_rbuf_head_t heads[1];
    nrfnet_rbuf_hole_t holes[2];
    nrfnet_rbuf_head_t* result;
    nrfnet_hdr_t hdr;
    uint8_t data[64];

    rbuf.heads = heads;
    rbuf.holes = holes;
    rbuf.head_count = 1;
    rbuf.hole_count = 2;

    nrfnet_rbuf_init(&rbuf);

    hdr.from.u16.u16 = 1;
    hdr.to.u16.u16 = 2;
    hdr.next = 1;
    hdr.length = 64;
    hdr.offset = 0;

    hdr.id = 12;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 0);

    TEST_ASSERT(result == &heads[0]);
    TEST_ASSERT_EQUAL_INT(12, heads[0].hdr.id);

    hdr.id = 34;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 10);

    TEST_ASSERT_NULL(result);
    TEST_ASSERT_EQUAL_INT(12, heads[0].hdr.id);

    /* garbage collect with lifetime 10 at t=10 */
    nrfnet_rbuf_gc(&rbuf, 10, 10);

    hdr.id = 56;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 20);

    TEST_ASSERT_NULL(result);
    TEST_ASSERT_EQUAL_INT(12, heads[0].hdr.id);

    /* garbage collect with lifetime 10 at t=20 */
    nrfnet_rbuf_gc(&rbuf, 20, 10);

    hdr.id = 78;
    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 30);

    TEST_ASSERT(result == &heads[0]);
    TEST_ASSERT_EQUAL_INT(78, heads[0].hdr.id);
}

static void test_nrfnet_rbuf_free(void)
{
    nrfnet_rbuf_t rbuf;
    nrfnet_rbuf_head_t heads[2];
    nrfnet_rbuf_hole_t holes[4];
    nrfnet_rbuf_head_t* result;
    nrfnet_hdr_t hdr;
    uint8_t data[64];

    rbuf.heads = heads;
    rbuf.holes = holes;
    rbuf.head_count = 2;
    rbuf.hole_count = 1;

    nrfnet_rbuf_init(&rbuf);

    hdr.from.u16.u16 = 1;
    hdr.to.u16.u16 = 2;
    hdr.next = 1;
    hdr.id = 123;
    hdr.length = 64;
    hdr.offset = 0;

    result = nrfnet_rbuf_add(&rbuf, &hdr, data, NRFNET_DATA_SIZE, 0);

    TEST_ASSERT(result == &heads[0]);
    TEST_ASSERT(result->used == true);
    TEST_ASSERT(result->next == &holes[0]);
    TEST_ASSERT(holes[0].used == true);

    nrfnet_rbuf_free(result);

    TEST_ASSERT(result == &heads[0]);
    TEST_ASSERT(result->used == false);
    TEST_ASSERT_NULL(result->next);
    TEST_ASSERT(holes[0].used == false);
}

static void test_nrfnet_rbuf_cpy__complete(void)
{
    nrfnet_rbuf_t rbuf;
    nrfnet_rbuf_head_t heads[2];
    nrfnet_rbuf_hole_t holes[4];
    nrfnet_rbuf_head_t* result;
    nrfnet_hdr_t hdr;
    uint8_t data[64 + 1];
    uint8_t* result_data;

    rbuf.heads = heads;
    rbuf.holes = holes;
    rbuf.head_count = 2;
    rbuf.hole_count = 4;

    nrfnet_rbuf_init(&rbuf);

    for (uint8_t i = 0; i < sizeof(data) - 1; i++) {
        data[i] = i;
    }

    /* stack canary to check if copy doesn't overflow */
    data[64] = 128;

    hdr.from.u16.u16 = 1;
    hdr.to.u16.u16 = 2;
    hdr.next = 1;
    hdr.id = 123;
    hdr.length = sizeof(data) - 1;

    hdr.offset = (NRFNET_DATA_SIZE * 0);
    result = nrfnet_rbuf_add(&rbuf, &hdr, &data[hdr.offset], NRFNET_DATA_SIZE, 0);

    hdr.offset = (NRFNET_DATA_SIZE * 1);
    result = nrfnet_rbuf_add(&rbuf, &hdr, &data[hdr.offset], NRFNET_DATA_SIZE, 1);

    hdr.offset = (NRFNET_DATA_SIZE * 2);
    result = nrfnet_rbuf_add(&rbuf, &hdr, &data[hdr.offset], sizeof(data) - hdr.offset - 1, 2);

    /* clear buffer */
    for (uint8_t i = 0; i < sizeof(data) - 1; i++) {
        data[i] = 0;
    }

    result_data = nrfnet_rbuf_cpy(result, data, sizeof(data) - 1);

    TEST_ASSERT(result_data == data);

    for (uint8_t i = 0; i < sizeof(data) - 1; i++) {
        TEST_ASSERT_EQUAL_INT(i, data[i]);
    }

    TEST_ASSERT_EQUAL_INT(128, data[64]);
}

/**
 * @brief   Assert that an incomplete packet cannot be copied.
 */
static void test_nrfnet_rbuf_cpy__incomplete(void)
{
    nrfnet_rbuf_t rbuf;
    nrfnet_rbuf_head_t heads[2];
    nrfnet_rbuf_hole_t holes[4];
    nrfnet_rbuf_head_t* result;
    nrfnet_hdr_t hdr;
    uint8_t data[64 + 1];
    uint8_t* result_data;

    rbuf.heads = heads;
    rbuf.holes = holes;
    rbuf.head_count = 2;
    rbuf.hole_count = 4;

    nrfnet_rbuf_init(&rbuf);

    hdr.from.u16.u16 = 1;
    hdr.to.u16.u16 = 2;
    hdr.next = 1;
    hdr.id = 123;
    hdr.length = sizeof(data) - 1;

    hdr.offset = (NRFNET_DATA_SIZE * 0);
    result = nrfnet_rbuf_add(&rbuf, &hdr, &data[hdr.offset], NRFNET_DATA_SIZE, 0);

    result_data = nrfnet_rbuf_cpy(result, data, sizeof(data) - 1);

    TEST_ASSERT_NULL(result_data);
}

Test *tests_nrfnet_rbuf_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_nrfnet_rbuf_init),
        new_TestFixture(test_nrfnet_rbuf_add__single),
        new_TestFixture(test_nrfnet_rbuf_add__continue),
        new_TestFixture(test_nrfnet_rbuf_add__continue_reverse),
        new_TestFixture(test_nrfnet_rbuf_add__new_packet),
        new_TestFixture(test_nrfnet_rbuf_add__no_free_heads),
        new_TestFixture(test_nrfnet_rbuf_add__no_free_holes),
        new_TestFixture(test_nrfnet_rbuf_gc),
        new_TestFixture(test_nrfnet_rbuf_free),
        new_TestFixture(test_nrfnet_rbuf_cpy__complete),
        new_TestFixture(test_nrfnet_rbuf_cpy__incomplete)
    };

    EMB_UNIT_TESTCALLER(tests_nrfnet_rbuf_tests,
                        NULL,
                        NULL,
                        fixtures);

    return (Test *)&tests_nrfnet_rbuf_tests;
}
