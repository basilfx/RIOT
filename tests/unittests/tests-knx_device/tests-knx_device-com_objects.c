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
 * @brief       Test cases for the KNX device communication objects
 *              implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "embUnit/embUnit.h"

#include "knx_device/system7/com_object.h"

#include "tests-knx_device.h"

static knx_table_com_objects_t table = {
    .objects = {
        {
            .type = KNX_COM_OBJECT_TYPE_BIT7,
            .flags = KNX_COM_OBJECT_ACCESS_READ
        },
        {
            .type = KNX_COM_OBJECT_TYPE_BYTE1,
            .flags = KNX_COM_OBJECT_ACCESS_WRITE
        }
    }
};

static void tests_knx_com_object_update__multiple(void)
{
    knx_com_object_t objects[] = {
        {
            .type = 0,
            .flags = 0,
        },
        {
            .type = 0,
            .flags = 0,
        }
    };

    knx_com_object_update(objects, &table, 2);

    TEST_ASSERT(objects[0].type == KNX_COM_OBJECT_TYPE_BIT7);
    TEST_ASSERT(objects[0].access == KNX_COM_OBJECT_ACCESS_READ);
    TEST_ASSERT(objects[1].type == KNX_COM_OBJECT_TYPE_BYTE1);
    TEST_ASSERT(objects[1].access == KNX_COM_OBJECT_ACCESS_WRITE);
}

static void test_knx_com_object_size__bit1(void)
{
    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT1
    };

    TEST_ASSERT(knx_com_object_size(&com_object) == 0);
}

static void test_knx_com_object_size__bit7(void)
{
    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT7
    };

    TEST_ASSERT(knx_com_object_size(&com_object) == 1);
}

static void test_knx_com_object_size__vardata(void)
{
    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_VARDATA
    };

    TEST_ASSERT(knx_com_object_size(&com_object) == 15);
}

static void test_knx_com_object_read__pointer__bit1(void)
{
    uint8_t buf = 0;
    uint8_t value = 1;

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .content.ptr = &value
    };

    ssize_t result = knx_com_object_read(&com_object, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(buf == 1);
}

static void test_knx_com_object_read__pointer__bit7(void)
{
    uint8_t buf = 0;
    uint8_t value = 64;

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT7,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .content.ptr = &value
    };

    ssize_t result = knx_com_object_read(&com_object, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(buf == 64);
}

static void test_knx_com_object_read__pointer__vardata(void)
{
    uint8_t buf[15] = { 0 };
    uint8_t value[15] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_VARDATA,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .content.ptr = &value
    };

    ssize_t result = knx_com_object_read(&com_object, &buf, sizeof(buf));

    TEST_ASSERT(result == 15);
    for (unsigned i = 0; i < 15; i++) {
        TEST_ASSERT(buf[i] == value[i]);
    }
}

static void test_knx_com_object_read__value__bit1(void)
{
    uint8_t buf = 0;

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_NONE,
        .content.value = { 1 }
    };

    ssize_t result = knx_com_object_read(&com_object, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(buf == 1);
}

static void test_knx_com_object_read__value__bit7(void)
{
    uint8_t buf = 0;

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT7,
        .flags = KNX_COM_OBJECT_FLAG_NONE,
        .content.value = { 64 }
    };

    ssize_t result = knx_com_object_read(&com_object, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(buf == 64);
}

static void test_knx_com_object_read__null(void)
{
    uint8_t buf = 0;

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT7,
        .flags = KNX_COM_OBJECT_FLAG_NONE,
        .content.value = { 64 }
    };

    ssize_t result = knx_com_object_read(&com_object, NULL, sizeof(buf));

    TEST_ASSERT(result == -1);
}

static void test_knx_com_object_read__too_small(void)
{
    uint8_t buf[15] = { 0 };
    uint8_t value[15] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_VARDATA,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .content.ptr = &value
    };

    ssize_t result = knx_com_object_read(&com_object, &buf, 1);

    TEST_ASSERT(result == -2);
}

static void test_knx_com_object_write__pointer__bit1(void)
{
    uint8_t buf = 1;
    uint8_t value = 0;

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .content.ptr = &value
    };

    ssize_t result = knx_com_object_write(&com_object, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(value == 1);
}

static void test_knx_com_object_write__pointer__bit7(void)
{
    uint8_t buf = 64;
    uint8_t value = 0;

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT7,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .content.ptr = &value
    };

    ssize_t result = knx_com_object_write(&com_object, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(value == 64);
}

static void test_knx_com_object_write__pointer__vardata(void)
{
    uint8_t buf[15] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
    uint8_t value[15] = { 0 };

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_VARDATA,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .content.ptr = &value
    };

    ssize_t result = knx_com_object_write(&com_object, &buf, sizeof(buf));

    TEST_ASSERT(result == 15);
    for (unsigned i = 0; i < 15; i++) {
        TEST_ASSERT(value[i] == buf[i]);
    }
}

static void test_knx_com_object_write__value__bit1(void)
{
    uint8_t buf = 1;

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .flags = KNX_COM_OBJECT_FLAG_NONE,
        .content.value = { 0 }
    };

    ssize_t result = knx_com_object_write(&com_object, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(com_object.content.value[0] == 1);
}

static void test_knx_com_object_write__value__bit7(void)
{
    uint8_t buf = 64;

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT7,
        .flags = KNX_COM_OBJECT_FLAG_NONE,
        .content.value = { 0 }
    };

    ssize_t result = knx_com_object_write(&com_object, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(com_object.content.value[0] == 64);
}

static void test_knx_com_object_write__null(void)
{
    uint8_t buf = 64;

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_BIT7,
        .flags = KNX_COM_OBJECT_FLAG_NONE,
        .content.value = { 0 }
    };

    ssize_t result = knx_com_object_write(&com_object, NULL, sizeof(buf));

    TEST_ASSERT(result == -1);
}

static void test_knx_com_object_write__too_small(void)
{
    uint8_t buf[15] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
    uint8_t value[15] = { 0 };

    knx_com_object_t com_object = {
        .type = KNX_COM_OBJECT_TYPE_VARDATA,
        .flags = KNX_COM_OBJECT_FLAG_POINTER,
        .content.ptr = &value
    };

    ssize_t result = knx_com_object_write(&com_object, &buf, 1);

    TEST_ASSERT(result == -2);
}

Test *tests_knx_com_object(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(tests_knx_com_object_update__multiple),
        new_TestFixture(test_knx_com_object_size__bit1),
        new_TestFixture(test_knx_com_object_size__bit7),
        new_TestFixture(test_knx_com_object_size__vardata),
        new_TestFixture(test_knx_com_object_read__pointer__bit1),
        new_TestFixture(test_knx_com_object_read__pointer__bit7),
        new_TestFixture(test_knx_com_object_read__pointer__vardata),
        new_TestFixture(test_knx_com_object_read__value__bit1),
        new_TestFixture(test_knx_com_object_read__value__bit7),
        new_TestFixture(test_knx_com_object_read__null),
        new_TestFixture(test_knx_com_object_read__too_small),
        new_TestFixture(test_knx_com_object_write__pointer__bit1),
        new_TestFixture(test_knx_com_object_write__pointer__bit7),
        new_TestFixture(test_knx_com_object_write__pointer__vardata),
        new_TestFixture(test_knx_com_object_write__value__bit1),
        new_TestFixture(test_knx_com_object_write__value__bit7),
        new_TestFixture(test_knx_com_object_write__null),
        new_TestFixture(test_knx_com_object_write__too_small),
    };

    EMB_UNIT_TESTCALLER(knx_com_object, NULL, NULL, fixtures);

    return (Test *)&knx_com_object;
}
