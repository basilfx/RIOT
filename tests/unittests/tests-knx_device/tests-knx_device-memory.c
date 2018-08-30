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
 * @brief       Test cases for the KNX device memory implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <string.h>

#include "embUnit/embUnit.h"

#include "knx_device/system7/memory.h"

#include "tests-knx_device.h"

static uint8_t ram1[10];
static uint8_t ram2[10];
static uint8_t ram3[10];

static knx_memory_segment_t segments[] = {
    {
        .start = 0x0000,
        .size = sizeof(ram1),
        .type = KNX_MEMORY_TYPE_RAM,
        .flags = KNX_MEMORY_FLAG_READABLE | KNX_MEMORY_FLAG_WRITABLE,
        .ptr = &ram1
    },
    {
        .start = 0x1000,
        .size = sizeof(ram2),
        .type = KNX_MEMORY_TYPE_RAM,
        .flags = KNX_MEMORY_FLAG_READABLE,
        .ptr = &ram2
    },
    {
        .start = 0x2000,
        .size = sizeof(ram3),
        .type = KNX_MEMORY_TYPE_RAM,
        .flags = KNX_MEMORY_FLAG_WRITABLE,
        .ptr = &ram3
    },
    KNX_MEMORY_SEGMENT_END
};

static void test_knx_memory_find__existing_partial(void)
{
    knx_memory_segment_t *segment = knx_memory_find(segments, 0x0000, 5);

    TEST_ASSERT(segment == &segments[0]);
}

static void test_knx_memory_find__existing_full(void)
{
    knx_memory_segment_t *segment = knx_memory_find(segments, 0x0000, 10);

    TEST_ASSERT(segment == &segments[0]);
}

static void test_knx_memory_find__existing_too_large(void)
{
    knx_memory_segment_t *segment = knx_memory_find(segments, 0x0000, 255);

    TEST_ASSERT(segment == NULL);
}

static void test_knx_memory_find__non_existing(void)
{
    knx_memory_segment_t *segment = knx_memory_find(segments, 0xf000, 10);

    TEST_ASSERT(segment == NULL);
}

static void test_knx_memory_read__partial(void)
{
    uint8_t tmp[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    uint8_t buf[10];

    memcpy(ram1, tmp, sizeof(ram1));

    ssize_t result = knx_memory_read(&segments[0], 0x0000, 5, buf);

    TEST_ASSERT(result == 5);
    for (unsigned i = 0; i < 5; i++) {
        TEST_ASSERT(buf[i] == tmp[i]);
    }
}

static void test_knx_memory_read__full(void)
{
    uint8_t tmp[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    memcpy(ram1, tmp, sizeof(ram1));

    uint8_t buf[10];

    ssize_t result = knx_memory_read(&segments[0], 0x0000, 10, buf);

    TEST_ASSERT(result == 10);
    for (unsigned i = 0; i < 10; i++) {
        TEST_ASSERT(buf[i] == tmp[i]);
    }
}

static void test_knx_memory_read__non_readable(void)
{
    uint8_t buf[10];

    ssize_t result = knx_memory_read(&segments[2], 0x2000, 10, buf);

    TEST_ASSERT(result == -1);
}

static void test_knx_memory_read__null(void)
{
    ssize_t result = knx_memory_read(&segments[0], 0x0000, 10, NULL);

    TEST_ASSERT(result == -2);
}

static void test_knx_memory_write__partial(void)
{
    memset(ram1, 0, sizeof(ram1));

    uint8_t buf[10] = { 0, 1, 2, 3, 4, 0, 0, 0, 0, 0 };

    ssize_t result = knx_memory_write(&segments[0], 0x0000, 5, buf);

    TEST_ASSERT(result == 5);
    for (unsigned i = 0; i < 10; i++) {
        TEST_ASSERT(ram1[i] == buf[i]);
    }
}

static void test_knx_memory_write__full(void)
{
    memset(ram1, 0, sizeof(ram1));

    uint8_t buf[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    ssize_t result = knx_memory_write(&segments[0], 0x0000, 10, buf);

    TEST_ASSERT(result == 10);
    for (unsigned i = 0; i < 10; i++) {
        TEST_ASSERT(ram1[i] == buf[i]);
    }
}

static void test_knx_memory_write__non_writeable(void)
{
    uint8_t buf[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    ssize_t result = knx_memory_write(&segments[1], 0x1000, 10, buf);

    TEST_ASSERT(result == -1);
}

static void test_knx_memory_write__null(void)
{
    ssize_t result = knx_memory_write(&segments[0], 0x0000, 10, NULL);

    TEST_ASSERT(result == -2);
}

Test *tests_knx_memory(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_knx_memory_find__existing_partial),
        new_TestFixture(test_knx_memory_find__existing_full),
        new_TestFixture(test_knx_memory_find__existing_too_large),
        new_TestFixture(test_knx_memory_find__non_existing),
        new_TestFixture(test_knx_memory_read__partial),
        new_TestFixture(test_knx_memory_read__full),
        new_TestFixture(test_knx_memory_read__non_readable),
        new_TestFixture(test_knx_memory_read__null),
        new_TestFixture(test_knx_memory_write__partial),
        new_TestFixture(test_knx_memory_write__full),
        new_TestFixture(test_knx_memory_write__non_writeable),
        new_TestFixture(test_knx_memory_write__null),
    };

    EMB_UNIT_TESTCALLER(knx_memory_tests, NULL, NULL, fixtures);

    return (Test *)&knx_memory_tests;
}
