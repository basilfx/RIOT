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
 * @brief       Test cases for the KNX TPCI implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "embUnit/embUnit.h"

#include "net/knx/tpci.h"

#include "tests-knx.h"

static void tests_knx_tpci_get__standard(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0xc0, 0x80, 0x30
    };

    TEST_ASSERT(knx_tpci_get(telegram) == KNX_TPCI_NCD);
}

static void tests_knx_tpci_get__extended(void)
{
    uint8_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x02, 0xc0, 0xb0, 0x01
    };

    TEST_ASSERT(knx_tpci_get(telegram) == KNX_TPCI_NCD);
}

static void tests_knx_tpci_get__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_tpci_get(telegram) == 0);
}

static void tests_knx_tpci_set__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_tpci_set(telegram, KNX_TPCI_NCD);

    TEST_ASSERT(telegram[6] == 0xc0);
}

static void tests_knx_tpci_set__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_tpci_set(telegram, KNX_TPCI_NCD);

    TEST_ASSERT(telegram[7] == 0xc0);
}

static void tests_knx_tpci_set__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_tpci_set(telegram, KNX_TPCI_NCD);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

static void tests_knx_tpci_ucd_get__standard(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x81, 0x80, 0x30
    };

    TEST_ASSERT(knx_tpci_ucd_get(telegram) == KNX_TPCI_UCD_DISCONNECT);
}

static void tests_knx_tpci_ucd_get__extended(void)
{
    uint8_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x02, 0x81, 0xb0, 0x01
    };

    TEST_ASSERT(knx_tpci_ucd_get(telegram) == KNX_TPCI_UCD_DISCONNECT);
}

static void tests_knx_tpci_ucd_get__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_tpci_ucd_get(telegram) == 0);
}

static void tests_knx_tpci_ucd_set__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_DISCONNECT);

    TEST_ASSERT(telegram[6] == 0x81);
}

static void tests_knx_tpci_ucd_set__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_DISCONNECT);

    TEST_ASSERT(telegram[7] == 0x81);
}

static void tests_knx_tpci_ucd_set__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_DISCONNECT);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

static void tests_knx_tpci_ncd_get__standard(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0xc3, 0x80, 0x30
    };

    TEST_ASSERT(knx_tpci_ncd_get(telegram) == KNX_TPCI_NCD_NACK);
}

static void tests_knx_tpci_ncd_get__extended(void)
{
    uint8_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x02, 0xc3, 0xb0, 0x01
    };

    TEST_ASSERT(knx_tpci_ncd_get(telegram) == KNX_TPCI_NCD_NACK);
}

static void tests_knx_tpci_ncd_get__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_tpci_ncd_get(telegram) == 0);
}

static void tests_knx_tpci_ncd_set__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_tpci_set(telegram, KNX_TPCI_NCD);
    knx_tpci_ncd_set(telegram, KNX_TPCI_NCD_NACK);

    TEST_ASSERT(telegram[6] == 0xc3);
}

static void tests_knx_tpci_ncd_set__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_tpci_set(telegram, KNX_TPCI_NCD);
    knx_tpci_ncd_set(telegram, KNX_TPCI_NCD_NACK);

    TEST_ASSERT(telegram[7] == 0xc3);
}

static void tests_knx_tpci_ncd_set__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_tpci_ncd_set(telegram, KNX_TPCI_NCD_ACK);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

static void tests_knx_tpci_get_seq_number__standard(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x68, 0x80, 0x30
    };

    TEST_ASSERT(knx_tpci_get_seq_number(telegram) == 0x0a);
}

static void tests_knx_tpci_get_seq_number__extended(void)
{
    uint8_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x02, 0x68, 0xb0, 0x01
    };

    TEST_ASSERT(knx_tpci_get_seq_number(telegram) == 0x0a);
}

static void tests_knx_tpci_get_seq_number__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_tpci_get_seq_number(telegram) == 0);
}

static void tests_knx_tpci_set_seq_number__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_tpci_set(telegram, KNX_TPCI_NDP);
    knx_tpci_set_seq_number(telegram, 0x09);

    TEST_ASSERT(telegram[6] == 0x64);
}

static void tests_knx_tpci_set_seq_number__standard__too_long(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_tpci_set(telegram, KNX_TPCI_NDP);
    knx_tpci_set_seq_number(telegram, 0xff);

    TEST_ASSERT(telegram[6] == 0x40);
}

static void tests_knx_tpci_set_seq_number__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_tpci_set(telegram, KNX_TPCI_NDP);
    knx_tpci_set_seq_number(telegram, 0x09);

    TEST_ASSERT(telegram[7] == 0x64);
}

static void tests_knx_tpci_set_seq_number__extended__too_long(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_tpci_set(telegram, KNX_TPCI_NDP);
    knx_tpci_set_seq_number(telegram, 0xff);

    TEST_ASSERT(telegram[7] == 0x40);
}

static void tests_knx_tpci_set_seq_number__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_tpci_set_seq_number(telegram, 4);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

Test *tests_knx_tpci_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(tests_knx_tpci_get__standard),
        new_TestFixture(tests_knx_tpci_get__extended),
        new_TestFixture(tests_knx_tpci_get__other),
        new_TestFixture(tests_knx_tpci_set__standard),
        new_TestFixture(tests_knx_tpci_set__extended),
        new_TestFixture(tests_knx_tpci_set__other),
        new_TestFixture(tests_knx_tpci_ucd_get__standard),
        new_TestFixture(tests_knx_tpci_ucd_get__extended),
        new_TestFixture(tests_knx_tpci_ucd_get__other),
        new_TestFixture(tests_knx_tpci_ucd_set__standard),
        new_TestFixture(tests_knx_tpci_ucd_set__extended),
        new_TestFixture(tests_knx_tpci_ucd_set__other),
        new_TestFixture(tests_knx_tpci_ncd_get__standard),
        new_TestFixture(tests_knx_tpci_ncd_get__extended),
        new_TestFixture(tests_knx_tpci_ncd_get__other),
        new_TestFixture(tests_knx_tpci_ncd_set__standard),
        new_TestFixture(tests_knx_tpci_ncd_set__extended),
        new_TestFixture(tests_knx_tpci_ncd_set__other),
        new_TestFixture(tests_knx_tpci_get_seq_number__standard),
        new_TestFixture(tests_knx_tpci_get_seq_number__extended),
        new_TestFixture(tests_knx_tpci_get_seq_number__other),
        new_TestFixture(tests_knx_tpci_set_seq_number__standard),
        new_TestFixture(tests_knx_tpci_set_seq_number__standard__too_long),
        new_TestFixture(tests_knx_tpci_set_seq_number__extended__too_long),
        new_TestFixture(tests_knx_tpci_set_seq_number__extended),
        new_TestFixture(tests_knx_tpci_set_seq_number__other),
    };

    EMB_UNIT_TESTCALLER(knx_tpci_tests, NULL, NULL, fixtures);

    return (Test *)&knx_tpci_tests;
}
