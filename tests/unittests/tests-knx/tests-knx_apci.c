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
 * @brief       Test cases for the KNX APCI implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "embUnit/embUnit.h"

#include "net/knx/apci.h"

#include "tests-knx.h"

static void test_knx_apci_get__standard(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_apci_get(telegram) == KNX_APCI_GROUP_VALUE_WRITE);
}

static void test_knx_apci_get__extended(void)
{
    uint8_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x02, 0x00, 0xb0, 0x01
    };

    TEST_ASSERT(knx_apci_get(telegram) == KNX_APCI_GROUP_VALUE_WRITE);
}

static void test_knx_apci_get__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_apci_get(telegram) == 0);
}

static void test_knx_apci_set__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_apci_set(telegram, KNX_APCI_ESCAPE);

    TEST_ASSERT(telegram[6] == 0x03);
    TEST_ASSERT(telegram[7] == 0xc0);
}

static void test_knx_apci_set__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_apci_set(telegram, KNX_APCI_ESCAPE);

    TEST_ASSERT(telegram[7] == 0x03);
    TEST_ASSERT(telegram[8] == 0xc0);
}

static void test_knx_apci_set__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_apci_set(telegram, KNX_APCI_MEMORY_WRITE);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

static void test_knx_apci_extended_get__standard(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x03, 0xd5, 0x30
    };

    TEST_ASSERT(knx_apci_extended_get(telegram) == KNX_APCI_EXTENDED_PROPERTY_VALUE_READ);
}

static void test_knx_apci_extended_get__extended(void)
{
    uint8_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x0f, 0xd3, 0xd5, 0x01
    };

    TEST_ASSERT(knx_apci_extended_get(telegram) == KNX_APCI_EXTENDED_PROPERTY_VALUE_READ);
}

static void test_knx_apci_extended_get__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_apci_extended_get(telegram) == 0);
}

static void test_knx_apci_extended_set__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_apci_extended_set(telegram, KNX_APCI_EXTENDED_AUTHORIZE_REQUEST);

    TEST_ASSERT(telegram[6] == 0x03);
    TEST_ASSERT(telegram[7] == 0xd1);
}

static void test_knx_apci_extended_set__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_apci_extended_set(telegram, KNX_APCI_EXTENDED_AUTHORIZE_REQUEST);

    TEST_ASSERT(telegram[7] == 0x03);
    TEST_ASSERT(telegram[8] == 0xd1);
}

static void test_knx_apci_extended_set__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_apci_extended_set(telegram, KNX_APCI_EXTENDED_PROPERTY_VALUE_RESPONSE);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

Test *tests_knx_apci_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_knx_apci_get__standard),
        new_TestFixture(test_knx_apci_get__extended),
        new_TestFixture(test_knx_apci_get__other),
        new_TestFixture(test_knx_apci_set__standard),
        new_TestFixture(test_knx_apci_set__extended),
        new_TestFixture(test_knx_apci_set__other),
        new_TestFixture(test_knx_apci_extended_get__standard),
        new_TestFixture(test_knx_apci_extended_get__extended),
        new_TestFixture(test_knx_apci_extended_get__other),
        new_TestFixture(test_knx_apci_extended_set__standard),
        new_TestFixture(test_knx_apci_extended_set__extended),
        new_TestFixture(test_knx_apci_extended_set__other),
    };

    EMB_UNIT_TESTCALLER(knx_apci_tests, NULL, NULL, fixtures);

    return (Test *)&knx_apci_tests;
}
