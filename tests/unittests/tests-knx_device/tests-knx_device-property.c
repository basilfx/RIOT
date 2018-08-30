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
 * @brief       Test cases for the KNX device property implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <string.h>

#include "embUnit/embUnit.h"

#include "knx_device/system7/property.h"

#include "tests-knx_device.h"

static char text[] = "hello, world!";

static uint8_t dummy[8];

static knx_property_t properties_0[] = {
    {
        .id = 1,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = { 0x00, 0x00 }
    },
    {
        .id = 2,
        .type = KNX_PROPERTY_TYPE_GENERIC6,
        .flags = KNX_PROPERTY_FLAG_POINTER | KNX_PROPERTY_FLAG_WRITABLE,
        .content.ptr = dummy
    },
    {
        .id = 3,
        .type = KNX_PROPERTY_TYPE_GENERIC2,
        .flags = KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = dummy
    },
    {
        .id = 40,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_CHAR,
        .flags = KNX_PROPERTY_FLAG_NONE | KNX_PROPERTY_FLAG_WRITABLE,
        .content.value = { 0xaa }
    }
};

static knx_property_t properties_1[] = {
    {
        .id = 1,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = { 0x00, 0x07 }
    },
    {
        .id = 2,
        .type = KNX_PROPERTY_TYPE_CONTROL,
        .flags = KNX_PROPERTY_FLAG_WRITABLE,
        .content.value = { 0x00, 0x00 }
    }
};

static knx_property_t properties_2[] = {
    {
        .id = 1,
        .type = KNX_PROPERTY_TYPE_UNSIGNED_INT,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = { 0x00, 0x00 }
    }
};

static knx_property_t properties_3[] = {
    {
        .id = 1,
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_ARRAY,
        .size = sizeof(text),
        .content.ptr = text
    }
};

static knx_property_object_t objects[] = {
    {
        .properties = properties_0,
        .count = sizeof(properties_0) / sizeof(properties_0[0])
    },
    {
        .properties = properties_1,
        .count = sizeof(properties_1) / sizeof(properties_1[0])
    },
    {
        .properties = properties_2,
        .count = sizeof(properties_2) / sizeof(properties_2[0])
    },
    {
        .properties = properties_3,
        .count = sizeof(properties_3) / sizeof(properties_3[0])
    },
    KNX_PROPERTY_OBJECT_END
};

static void test_knx_property_find_by_id__existing(void)
{
    TEST_ASSERT(knx_property_find_by_id(objects, 0, 1) == &properties_0[0]);
    TEST_ASSERT(knx_property_find_by_id(objects, 0, 40) == &properties_0[3]);
    TEST_ASSERT(knx_property_find_by_id(objects, 2, 1) == &properties_2[0]);
}

static void test_knx_property_find_by_id__non_existing(void)
{
    TEST_ASSERT(knx_property_find_by_id(objects, 0, 5) == NULL);
    TEST_ASSERT(knx_property_find_by_id(objects, 0, 255) == NULL);
    TEST_ASSERT(knx_property_find_by_id(objects, 255, 255) == NULL);
}

static void test_knx_property_find_by_index__existing(void)
{
    TEST_ASSERT(knx_property_find_by_index(objects, 0, 0) == &properties_0[0]);
    TEST_ASSERT(knx_property_find_by_index(objects, 0, 1) == &properties_0[1]);
    TEST_ASSERT(knx_property_find_by_index(objects, 0, 2) == &properties_0[2]);
    TEST_ASSERT(knx_property_find_by_index(objects, 0, 3) == &properties_0[3]);
    TEST_ASSERT(knx_property_find_by_index(objects, 1, 0) == &properties_1[0]);
}

static void test_knx_property_find_by_index__non_existing_object(void)
{
    TEST_ASSERT(knx_property_find_by_index(objects, 255, 255) == NULL);
}

static void test_knx_property_find_by_index__non_existing_property(void)
{
    TEST_ASSERT(knx_property_find_by_index(objects, 0, 255) == NULL);
}

static void test_knx_property_size__control(void)
{
    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_CONTROL
    };

    TEST_ASSERT(knx_property_size(&property) == 1);
}

static void test_knx_property_size__escape(void)
{
    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_ESCAPE
    };

    TEST_ASSERT(knx_property_size(&property) == 255);
}

static void test_knx_property_elements__value(void)
{
    TEST_ASSERT(knx_property_elements(&properties_2[0]) == 1);
}

static void test_knx_property_elements__array(void)
{
    TEST_ASSERT(knx_property_elements(&properties_3[0]) == sizeof(text));
}

static void test_knx_property_read__pointer(void)
{
    uint8_t buf = 0;
    uint8_t value = 123;

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = &value
    };

    ssize_t result = knx_property_read(&property, 1, 1, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(buf == 123);
}

static void test_knx_property_read__value(void)
{
    uint8_t buf = 0;

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = { 123 }
    };

    ssize_t result = knx_property_read(&property, 1, 1, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(buf == 123);
}

static void test_knx_property_read__array(void)
{
    char buf[3];
    char tmp[] = "abc";

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_ARRAY | KNX_PROPERTY_FLAG_POINTER,
        .size = sizeof(tmp),
        .content.ptr = tmp
    };

    ssize_t result = knx_property_read(&property, 1, 1, (uint8_t *)&buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(memcmp(buf, "a", 1) == 0);

    result = knx_property_read(&property, 2, 2, (uint8_t *)&buf, sizeof(buf));

    TEST_ASSERT(result == 2);
    TEST_ASSERT(memcmp(buf, "bc", 2) == 0);

    result = knx_property_read(&property, 3, 1, (uint8_t *)&buf, sizeof(buf));

    TEST_ASSERT(result == 3);
    TEST_ASSERT(memcmp(buf, "abc", 3) == 0);
}

static void test_knx_property_read__block(void)
{
    uint8_t buf[6] = { 0 };
    uint8_t value[6] = { 0, 1, 2, 3, 4, 5 };

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_GENERIC6,
        .flags = KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = value
    };

    ssize_t result = knx_property_read(&property, 1, 1, buf, sizeof(buf));

    TEST_ASSERT(result == 6);
    for (unsigned i = 0; i < 6; i++) {
        TEST_ASSERT(buf[i] == value[i]);
    }
}

static void test_knx_property_read__null(void)
{
    uint8_t buf = 0;

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_NONE,
        .content.value = { 123 }
    };

    ssize_t result = knx_property_read(&property, 1, 1, NULL, sizeof(buf));

    TEST_ASSERT(result == -1);
}

static void test_knx_property_read__too_small(void)
{
    uint8_t buf[6] = { 0 };
    uint8_t value[6] = { 0, 1, 2, 3, 4, 5 };

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_GENERIC6,
        .flags = KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = value
    };

    ssize_t result = knx_property_read(&property, 1, 1, buf, 1);

    TEST_ASSERT(result == -2);
}

static void test_knx_property_write__non_writeable(void)
{
    uint8_t buf = 123;
    uint8_t value = 0;

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_POINTER,
        .content.ptr = &value
    };

    ssize_t result = knx_property_write(&property, 1, 1, &buf, sizeof(buf));

    TEST_ASSERT(result == -3);
}

static void test_knx_property_write__control(void)
{
    uint8_t buf = 123;
    uint8_t value = 0;

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_CONTROL,
        .flags = KNX_PROPERTY_FLAG_POINTER | KNX_PROPERTY_FLAG_WRITABLE,
        .content.ptr = &value
    };

    ssize_t result = knx_property_write(&property, 1, 1, &buf, sizeof(buf));

    TEST_ASSERT(result == 0);
}

static void test_knx_property_write__pointer(void)
{
    uint8_t buf = 123;
    uint8_t value = 0;

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_POINTER | KNX_PROPERTY_FLAG_WRITABLE,
        .content.ptr = &value
    };

    ssize_t result = knx_property_write(&property, 1, 1, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(value == 123);
}

static void test_knx_property_write__value(void)
{
    uint8_t buf = 123;

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_WRITABLE,
        .content.value = { 0 }
    };

    ssize_t result = knx_property_write(&property, 1, 1, &buf, sizeof(buf));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(property.content.value[0] == 123);
}

static void test_knx_property_write__array(void)
{
    char buf[] = "abc";
    char buf2[] = "d";
    char tmp[3];

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_ARRAY | KNX_PROPERTY_FLAG_WRITABLE | KNX_PROPERTY_FLAG_POINTER,
        .size = sizeof(tmp),
        .content.ptr = tmp
    };

    ssize_t result = knx_property_write(&property, 3, 1, (uint8_t *)buf, sizeof(buf));

    TEST_ASSERT(result == 3);
    TEST_ASSERT(memcmp(tmp, "abc", 3) == 0);

    result = knx_property_write(&property, 1, 2, (uint8_t *)buf2, sizeof(buf2));

    TEST_ASSERT(result == 1);
    TEST_ASSERT(memcmp(tmp, "adc", 3) == 0);
}

static void test_knx_property_write__block(void)
{
    uint8_t buf[6] = { 0, 1, 2, 3, 4, 5 };
    uint8_t value[6] = { 0 };

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_GENERIC6,
        .flags = KNX_PROPERTY_FLAG_POINTER | KNX_PROPERTY_FLAG_WRITABLE,
        .content.ptr = value
    };

    ssize_t result = knx_property_write(&property, 1, 1, buf, sizeof(buf));

    TEST_ASSERT(result == 6);
    for (unsigned i = 0; i < 6; i++) {
        TEST_ASSERT(value[i] == buf[i]);
    }
}

static void test_knx_property_write__null(void)
{
    uint8_t buf = 123;

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_CHAR,
        .flags = KNX_PROPERTY_FLAG_WRITABLE,
        .content.value = { 0 }
    };

    ssize_t result = knx_property_write(&property, 1, 1, NULL, sizeof(buf));

    TEST_ASSERT(result == -1);
}

static void test_knx_property_write__too_small(void)
{
    uint8_t buf[6] = { 0, 1, 2, 3, 4, 5 };
    uint8_t value[6] = { 0 };

    knx_property_t property = {
        .type = KNX_PROPERTY_TYPE_GENERIC6,
        .flags = KNX_PROPERTY_FLAG_POINTER | KNX_PROPERTY_FLAG_WRITABLE,
        .content.ptr = value
    };

    ssize_t result = knx_property_write(&property, 1, 1, buf, 1);

    TEST_ASSERT(result == -2);
}

Test *tests_knx_property_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_knx_property_find_by_id__existing),
        new_TestFixture(test_knx_property_find_by_id__non_existing),
        new_TestFixture(test_knx_property_find_by_index__existing),
        new_TestFixture(test_knx_property_find_by_index__non_existing_object),
        new_TestFixture(test_knx_property_find_by_index__non_existing_property),
        new_TestFixture(test_knx_property_size__control),
        new_TestFixture(test_knx_property_size__escape),
        new_TestFixture(test_knx_property_elements__value),
        new_TestFixture(test_knx_property_elements__array),
        new_TestFixture(test_knx_property_read__pointer),
        new_TestFixture(test_knx_property_read__value),
        new_TestFixture(test_knx_property_read__array),
        new_TestFixture(test_knx_property_read__block),
        new_TestFixture(test_knx_property_read__null),
        new_TestFixture(test_knx_property_read__too_small),
        new_TestFixture(test_knx_property_write__non_writeable),
        new_TestFixture(test_knx_property_write__control),
        new_TestFixture(test_knx_property_write__pointer),
        new_TestFixture(test_knx_property_write__value),
        new_TestFixture(test_knx_property_write__array),
        new_TestFixture(test_knx_property_write__block),
        new_TestFixture(test_knx_property_write__null),
        new_TestFixture(test_knx_property_write__too_small)
    };

    EMB_UNIT_TESTCALLER(knx_property_tests, NULL, NULL, fixtures);

    return (Test *)&knx_property_tests;
}
/** @} */
