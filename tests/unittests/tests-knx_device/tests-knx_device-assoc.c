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
 * @brief       Test cases for the KNX device associations implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "embUnit/embUnit.h"

#include "knx_device/system7/assoc.h"

#include "tests-knx_device.h"

static knx_com_object_t objects[255];

static knx_table_addr_t addresses = {
    .count = 3,
    .addrs = { { 0x0001 }, { 0x0009 }, { 0x0003 } }
};

/* note that this is a memory table per KNX specification, which uses 
   1-based indexing for the address, and 0-based indexing for the com object */
static knx_table_assoc_t table = {
    .count = 3,
    .associations = {
        {
            .addr_index = 1,
            .com_object_index = 1
        },
        {
            .addr_index = 2,
            .com_object_index = 3
        },
        {
            .addr_index = 3,
            .com_object_index = 2
        },
        {
            .addr_index = 3,
            .com_object_index = 3
        }
    }
};

static knx_assoc_mapping_t mappings;

static knx_assoc_t associations = {
    .mappings = &mappings
};

static void test_knx_assoc_update__no_items(void)
{
    table.count = 0;

    knx_assoc_update(&associations, (knx_com_object_t *)&objects, &table, &addresses, 255);

    TEST_ASSERT(associations.count == 0);
}

static void test_knx_assoc_update__one_item(void)
{
    table.count = 1;

    knx_assoc_update(&associations, (knx_com_object_t *)&objects, &table, &addresses, 255);

    TEST_ASSERT(associations.count == 1);
    knx_addr_group_t addr_a = { { 0x01, 0x00 } };

    TEST_ASSERT(knx_addr_equal(&(associations.mappings[0].group_addr), &addr_a));
    TEST_ASSERT(associations.mappings[0].com_object == &objects[1]);
}

static void test_knx_assoc_update__three_items(void)
{
    table.count = 3;

    knx_assoc_update(&associations, (knx_com_object_t *)&objects, &table, &addresses, 255);

    TEST_ASSERT(associations.count == 3);
    knx_addr_group_t addr_a = { { 0x01, 0x00 } };
    TEST_ASSERT(knx_addr_equal(&(associations.mappings[0].group_addr), &addr_a));
    TEST_ASSERT(associations.mappings[0].com_object == &objects[1]);
    knx_addr_group_t addr_b = { { 0x03, 0x00 } };
    TEST_ASSERT(knx_addr_equal(&(associations.mappings[1].group_addr), &addr_b));
    TEST_ASSERT(associations.mappings[1].com_object == &objects[2]);
    knx_addr_group_t addr_c = { { 0x09, 0x00 } };
    TEST_ASSERT(knx_addr_equal(&(associations.mappings[2].group_addr), &addr_c));
    TEST_ASSERT(associations.mappings[2].com_object == &objects[3]);
}

static void test_knx_assoc_find_by_group_address__existing(void)
{
    table.count = 3;

    knx_assoc_update(&associations, (knx_com_object_t *)&objects, &table, &addresses, 255);

    knx_addr_group_t addr_a = { { 0x01, 0x00 } };
    TEST_ASSERT(knx_assoc_find_by_group_address(&associations, &addr_a) == 0);
    knx_addr_group_t addr_b = { { 0x03, 0x00 } };
    TEST_ASSERT(knx_assoc_find_by_group_address(&associations, &addr_b) == 1);
    knx_addr_group_t addr_c = { { 0x09, 0x00 } };
    TEST_ASSERT(knx_assoc_find_by_group_address(&associations, &addr_c) == 2);
}

static void test_knx_assoc_find_by_group_address__non_existing(void)
{
    table.count = 3;

    knx_assoc_update(&associations, (knx_com_object_t *)&objects, &table, &addresses, 255);

    knx_addr_group_t addr = { { 0xff, 0xff } };
    TEST_ASSERT(knx_assoc_find_by_group_address(&associations, &addr) == -1);
}

static void test_knx_assoc_iter_by_group_address__existing(void)
{
    table.count = 4;

    knx_assoc_update(&associations, (knx_com_object_t *)&objects, &table, &addresses, 255);

    knx_addr_group_t addr = { { 0x03, 0x00 } };
    knx_assoc_mapping_t *next = NULL;

    next = knx_assoc_iter_by_group_address(&associations, NULL, &addr);
    TEST_ASSERT(next->com_object == &(objects[3]));
    next = knx_assoc_iter_by_group_address(&associations, next, &addr);
    TEST_ASSERT(next->com_object == &(objects[2]));
    next = knx_assoc_iter_by_group_address(&associations, next, &addr);
    TEST_ASSERT(next == NULL);
}

static void test_knx_assoc_iter_by_group_address__non_existing(void)
{
    table.count = 4;

    knx_assoc_update(&associations, (knx_com_object_t *)&objects, &table, &addresses, 255);

    knx_addr_group_t addr = { { 0xff, 0xff } };
    TEST_ASSERT(knx_assoc_iter_by_group_address(&associations, NULL, &addr) == NULL);
}

static void test_knx_assoc_iter_by_group_address__out_of_bounds(void)
{
    table.count = 4;

    knx_assoc_update(&associations, (knx_com_object_t *)&objects, &table, &addresses, 255);

    knx_addr_group_t addr = { { 0x03, 0x00 } };
    knx_assoc_mapping_t *next = &(associations.mappings[3]);

    /* next points to the fourth (and last) association, so next++ is out of 
       bounds */
    next = knx_assoc_iter_by_group_address(&associations, next, &addr);
    TEST_ASSERT(next == NULL);
}

static void test_knx_assoc_find_by_com_object__existing(void)
{
    table.count = 3;

    /* note that objects[0] is not used in any association */
    TEST_ASSERT(knx_assoc_find_by_com_object(&associations, &(objects[0])) == -1);
    TEST_ASSERT(knx_assoc_find_by_com_object(&associations, &(objects[1])) == 0);
    TEST_ASSERT(knx_assoc_find_by_com_object(&associations, &(objects[2])) == 2);
    TEST_ASSERT(knx_assoc_find_by_com_object(&associations, &(objects[3])) == 1);
}

static void test_knx_assoc_find_by_com_object__non_existing(void)
{
    table.count = 3;

    knx_assoc_update(&associations, (knx_com_object_t *)&objects, &table, &addresses, 255);

    TEST_ASSERT(knx_assoc_find_by_com_object(&associations, &(objects[5])) == -1);
}

static void test_knx_assoc_iter_by_com_object__existing(void)
{
    table.count = 4;

    knx_assoc_update(&associations, (knx_com_object_t *)&objects, &table, &addresses, 255);

    knx_assoc_mapping_t *next = NULL;

    next = knx_assoc_iter_by_com_object(&associations, NULL, &(objects[3]));
    knx_addr_group_t addr_a = { { 0x03, 0x00 } };
    TEST_ASSERT(knx_addr_equal(&(next->group_addr), &addr_a));
    next = knx_assoc_iter_by_com_object(&associations, next, &(objects[3]));
    knx_addr_group_t addr_b = { { 0x09, 0x00 } };
    TEST_ASSERT(knx_addr_equal(&(next->group_addr), &addr_b));
    next = knx_assoc_iter_by_com_object(&associations, next, &(objects[3]));
    TEST_ASSERT(next == NULL);
}

static void test_knx_assoc_iter_by_com_object__non_existing(void)
{
    table.count = 4;

    knx_assoc_update(&associations, (knx_com_object_t *)&objects, &table, &addresses, 255);

    /* note that objects[0] is not used in any association */
    TEST_ASSERT(knx_assoc_iter_by_com_object(&associations, NULL, &(objects[0])) == NULL);
}

Test *tests_knx_assoc(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_knx_assoc_update__no_items),
        new_TestFixture(test_knx_assoc_update__one_item),
        new_TestFixture(test_knx_assoc_update__three_items),
        new_TestFixture(test_knx_assoc_find_by_group_address__existing),
        new_TestFixture(test_knx_assoc_find_by_group_address__non_existing),
        new_TestFixture(test_knx_assoc_iter_by_group_address__existing),
        new_TestFixture(test_knx_assoc_iter_by_group_address__non_existing),
        new_TestFixture(test_knx_assoc_iter_by_group_address__out_of_bounds),
        new_TestFixture(test_knx_assoc_find_by_com_object__existing),
        new_TestFixture(test_knx_assoc_find_by_com_object__non_existing),
        new_TestFixture(test_knx_assoc_iter_by_com_object__existing),
        new_TestFixture(test_knx_assoc_iter_by_com_object__non_existing)
    };

    EMB_UNIT_TESTCALLER(knx_assoc_tests, NULL, NULL, fixtures);

    return (Test *)&knx_assoc_tests;
}
