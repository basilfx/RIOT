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
 * @brief       Test cases for the KNX telegram implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdint.h>
#include <stdio.h>

#include "embUnit/embUnit.h"

#include "net/knx/telegram.h"

#include "tests-knx.h"

static void test_knx_telegram_is__valid(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_is(telegram, sizeof(telegram)));
}

static void test_knx_telegram_is__null(void)
{
    TEST_ASSERT(!knx_telegram_is(NULL, 0));
}

static void test_knx_telegram_is__unknown(void)
{
    uint8_t telegram[] = {
        0x00, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(!knx_telegram_is(telegram, sizeof(telegram)));
}

static void test_knx_telegram_is__too_long(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(!knx_telegram_is(telegram, KNX_TELEGRAM_MAX_LEN + 1));
}

static void test_knx_telegram_is__too_long_for_standard(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(!knx_telegram_is(telegram, KNX_TELEGRAM_TYPE_STANDARD_MAX_LEN + 1));
}

static void test_knx_telegram_is__too_short(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(!knx_telegram_is(telegram, KNX_TELEGRAM_MIN_LEN - 1));
}

static void test_knx_telegram_is__too_short_for_extended(void)
{
    uint8_t telegram[] = {
        0x3c, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(!knx_telegram_is(telegram, KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN - 1));
}

static void test_knx_telegram_checksum__correct(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x00
    };

    TEST_ASSERT(knx_telegram_checksum(telegram, sizeof(telegram)) == 0x30);
}

static void test_knx_telegram_get_checksum__correct(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_get_checksum(telegram, sizeof(telegram)) == 0x30);
}

static void test_knx_telegram_set_checksum__correct(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x00
    };

    knx_telegram_set_checksum(telegram, sizeof(telegram), 0x30);

    TEST_ASSERT(telegram[sizeof(telegram) - 1] == 0x30);
}

static void test_knx_telegram_update_checksum__correct(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x00
    };

    knx_telegram_update_checksum(telegram, sizeof(telegram));

    TEST_ASSERT(telegram[sizeof(telegram) - 1] == 0x30);
}

static void test_knx_telegram_is_checksum_valid__correct(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_is_checksum_valid(telegram, sizeof(telegram)));
}

static void test_knx_telegram_is_checksum_valid__incorrect(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x00
    };

    TEST_ASSERT(!knx_telegram_is_checksum_valid(telegram, sizeof(telegram)));
}

static void test_knx_telegram_get_type__standard(void)
{
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_get_type(telegram) == KNX_TELEGRAM_TYPE_STANDARD);
}

static void test_knx_telegram_get_type__extended(void)
{
    uint8_t telegram[] = {
        0x3c, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_get_type(telegram) == KNX_TELEGRAM_TYPE_EXTENDED);
}

static void test_knx_telegram_get_type__poll(void)
{
    uint8_t telegram[] = {
        0xf0, 0x11, 0x03, 0x01, 0x01, 0xE1
    };

    TEST_ASSERT(knx_telegram_get_type(telegram) == KNX_TELEGRAM_TYPE_POLL);
}

static void test_knx_telegram_get_type__unknown(void)
{
    uint8_t telegram[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    TEST_ASSERT(knx_telegram_get_type(telegram) == KNX_TELEGRAM_TYPE_UNKNOWN);
}

static void test_knx_telegram_set_type__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);

    TEST_ASSERT(telegram[0] == 0x90);
}

static void test_knx_telegram_set_type__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);

    TEST_ASSERT(telegram[0] == 0x10);
}

static void test_knx_telegram_set_type__poll(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_POLL_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_POLL);

    TEST_ASSERT(telegram[0] == 0xf0);
}

static void test_knx_telegram_set_type__unknown(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_UNKNOWN);

    TEST_ASSERT(telegram[0] == 0x00);
}

static void test_knx_telegram_get_src_addr__standard(void)
{
    knx_telegram_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };
    knx_addr_t tmp;

    TEST_ASSERT(knx_telegram_get_src_addr(telegram, &tmp) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0x11);
    TEST_ASSERT(tmp.u8[1] == 0x03);
}

static void test_knx_telegram_get_src_addr__extended(void)
{
    knx_telegram_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x00, 0x00, 0xb0, 0x01
    };
    knx_addr_t tmp;

    TEST_ASSERT(knx_telegram_get_src_addr(telegram, &tmp) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0x11);
    TEST_ASSERT(tmp.u8[1] == 0x03);
}

static void test_knx_telegram_get_src_addr__poll(void)
{
    knx_telegram_t telegram[] = {
        0xf0, 0x11, 0x03, 0x01, 0x01, 0xaa, 0xb7
    };
    knx_addr_t tmp;

    TEST_ASSERT(knx_telegram_get_src_addr(telegram, &tmp) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0x11);
    TEST_ASSERT(tmp.u8[1] == 0x03);
}

static void test_knx_telegram_get_src_addr__unknown(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };
    knx_addr_t tmp;

    TEST_ASSERT(knx_telegram_get_src_addr(telegram, &tmp) == NULL);
}

static void test_knx_telegram_get_src_addr__null(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_telegram_get_src_addr(telegram, NULL) == NULL);
}

static void test_knx_telegram_set_src_addr__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };
    knx_addr_t addr = { { 0x11, 0x08 } };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_telegram_set_src_addr(telegram, &addr);

    TEST_ASSERT(telegram[1] == 0x11);
    TEST_ASSERT(telegram[2] == 0x08);
}

static void test_knx_telegram_set_src_addr__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };
    knx_addr_t addr = { { 0x11, 0x08 } };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_telegram_set_src_addr(telegram, &addr);

    TEST_ASSERT(telegram[2] == 0x11);
    TEST_ASSERT(telegram[3] == 0x08);
}

static void test_knx_telegram_set_src_addr__poll(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_POLL_MIN_LEN] = { 0x00 };
    knx_addr_t addr = { { 0x11, 0x08 } };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_POLL);
    knx_telegram_set_src_addr(telegram, &addr);

    TEST_ASSERT(telegram[1] == 0x11);
    TEST_ASSERT(telegram[2] == 0x08);
}

static void test_knx_telegram_set_src_addr__unknown(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };
    knx_addr_t addr = { { 0x11, 0x08 } };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_UNKNOWN);
    knx_telegram_set_src_addr(telegram, &addr);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

static void test_knx_telegram_set_src_addr__null(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_telegram_set_src_addr(telegram, NULL);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

static void test_knx_telegram_get_dst_addr__standard(void)
{
    knx_telegram_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };
    knx_addr_t tmp;

    TEST_ASSERT(knx_telegram_get_dst_addr(telegram, &tmp) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0x01);
    TEST_ASSERT(tmp.u8[1] == 0x01);
}

static void test_knx_telegram_get_dst_addr__extended(void)
{
    knx_telegram_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x00, 0x00, 0xb0, 0x01
    };
    knx_addr_t tmp;

    TEST_ASSERT(knx_telegram_get_dst_addr(telegram, &tmp) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0x01);
    TEST_ASSERT(tmp.u8[1] == 0x01);
}

static void test_knx_telegram_get_dst_addr__poll(void)
{
    knx_telegram_t telegram[] = {
        0xf0, 0x11, 0x03, 0x01, 0x01, 0xaa, 0xb7
    };
    knx_addr_t tmp;

    TEST_ASSERT(knx_telegram_get_dst_addr(telegram, &tmp) != NULL);

    TEST_ASSERT(tmp.u8[0] == 0x01);
    TEST_ASSERT(tmp.u8[1] == 0x01);
}

static void test_knx_telegram_get_dst_addr__unknown(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };
    knx_addr_t tmp;

    TEST_ASSERT(knx_telegram_get_dst_addr(telegram, &tmp) == NULL);
}

static void test_knx_telegram_get_dst_addr__null(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_telegram_get_dst_addr(telegram, NULL) == NULL);
}

static void test_knx_telegram_set_dst_addr__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };
    knx_addr_t addr = { { 0x02, 0x01 } };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_telegram_set_dst_addr(telegram, &addr);

    TEST_ASSERT(telegram[3] == 0x02);
    TEST_ASSERT(telegram[4] == 0x01);
}

static void test_knx_telegram_set_dst_addr__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };
    knx_addr_t addr = { { 0x02, 0x01 } };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_telegram_set_dst_addr(telegram, &addr);

    TEST_ASSERT(telegram[4] == 0x02);
    TEST_ASSERT(telegram[5] == 0x01);
}

static void test_knx_telegram_set_dst_addr__poll(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_POLL_MIN_LEN] = { 0x00 };
    knx_addr_t addr = { { 0x02, 0x01 } };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_POLL);
    knx_telegram_set_dst_addr(telegram, &addr);

    TEST_ASSERT(telegram[3] == 0x02);
    TEST_ASSERT(telegram[4] == 0x01);
}

static void test_knx_telegram_set_dst_addr__unknown(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };
    knx_addr_t addr = { { 0x02, 0x01 } };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_UNKNOWN);
    knx_telegram_set_dst_addr(telegram, &addr);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

static void test_knx_telegram_set_dst_addr__null(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_telegram_set_dst_addr(telegram, NULL);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

void test_knx_telegram_is_group_addressed__standard(void)
{
    knx_telegram_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_is_group_addressed(telegram));
}

void test_knx_telegram_is_group_addressed__extended(void)
{
    knx_telegram_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x00, 0x00, 0xb0, 0x01
    };

    TEST_ASSERT(!knx_telegram_is_group_addressed(telegram));
}

void test_knx_telegram_is_group_addressed__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(!knx_telegram_is_group_addressed(telegram));
}

void test_knx_telegram_set_group_addressed__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);

    knx_telegram_set_group_addressed(telegram, true);
    TEST_ASSERT((telegram[5] & 0x80) == 0x80);

    knx_telegram_set_group_addressed(telegram, false);
    TEST_ASSERT((telegram[5] & 0x80) == 0x00);
}

void test_knx_telegram_set_group_addressed__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);

    knx_telegram_set_group_addressed(telegram, true);
    TEST_ASSERT(telegram[1] & 0x80);

    knx_telegram_set_group_addressed(telegram, false);
    TEST_ASSERT((telegram[1] & 0x80) == 0x00);
}

void test_knx_telegram_set_group_addressed__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_telegram_set_group_addressed(telegram, true);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }

    knx_telegram_set_group_addressed(telegram, false);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

void test_knx_telegram_get_payload__standard__unmerged(void)
{
    knx_telegram_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_get_payload(telegram, false) == &(telegram[8]));
}

void test_knx_telegram_get_payload__standard__merged(void)
{
    knx_telegram_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_get_payload(telegram, true) == &(telegram[7]));
}

void test_knx_telegram_get_payload__extended__unmerged(void)
{
    knx_telegram_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x00, 0x00, 0xb0, 0x01
    };

    TEST_ASSERT(knx_telegram_get_payload(telegram, false) == &(telegram[9]));
}

void test_knx_telegram_get_payload__extended__merged(void)
{
    knx_telegram_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x00, 0x00, 0xb0, 0x01
    };

    TEST_ASSERT(knx_telegram_get_payload(telegram, true) == &(telegram[8]));
}

void test_knx_telegram_get_payload__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_telegram_get_payload(telegram, false) == NULL);
}

void test_knx_telegram_get_payload_length__standard(void)
{
    knx_telegram_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_get_payload_length(telegram) == 1);
}

void test_knx_telegram_get_payload_length__extended(void)
{
    knx_telegram_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x02, 0x00, 0xb0, 0x01
    };

    TEST_ASSERT(knx_telegram_get_payload_length(telegram) == 2);
}

void test_knx_telegram_get_payload_length__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_telegram_get_payload_length(telegram) == 0);
}

void test_knx_telegram_set_payload_length__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_telegram_set_payload_length(telegram, 0x0f);

    TEST_ASSERT(telegram[5] == 0x0f);
}

void test_knx_telegram_set_payload_length__standard__too_long(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_telegram_set_payload_length(telegram, 0xff);

    TEST_ASSERT(telegram[5] == 0x0f);
}

void test_knx_telegram_set_payload_length__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_telegram_set_payload_length(telegram, 0xaa);

    TEST_ASSERT(telegram[6] == 0xaa);
}

void test_knx_telegram_set_payload_length__extended__too_long(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_telegram_set_payload_length(telegram, 0xaabb);

    TEST_ASSERT(telegram[6] == 0xbb);
}

void test_knx_telegram_set_payload_length__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_telegram_set_payload_length(telegram, 0xff);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

void test_knx_telegram_get_routing_count__standard(void)
{
    knx_telegram_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_get_routing_count(telegram) == 6);
}

void test_knx_telegram_get_routing_count__extended(void)
{
    knx_telegram_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x00, 0x00, 0xb0, 0x01
    };

    TEST_ASSERT(knx_telegram_get_routing_count(telegram) == 6);
}

void test_knx_telegram_get_routing_count__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_telegram_get_routing_count(telegram) == 0);
}

void test_knx_telegram_set_routing_count__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_telegram_set_routing_count(telegram, 0x07);

    TEST_ASSERT(telegram[5] == 0x70);
}

void test_knx_telegram_set_routing_count__standard__too_long(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_telegram_set_routing_count(telegram, 0x77);

    TEST_ASSERT(telegram[5] == 0x00);
}

void test_knx_telegram_set_routing_count__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_telegram_set_routing_count(telegram, 0x07);

    TEST_ASSERT(telegram[1] == 0x70);
}

void test_knx_telegram_set_routing_count__extended__too_long(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_telegram_set_routing_count(telegram, 0x77);

    TEST_ASSERT(telegram[1] == 0x00);
}

void test_knx_telegram_set_routing_count__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_telegram_set_routing_count(telegram, 0x06);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

void test_knx_telegram_get_priority__standard(void)
{
    knx_telegram_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_get_priority(telegram) == KNX_TELEGRAM_PRIORITY_LOW);
}

void test_knx_telegram_get_priority__extended(void)
{
    knx_telegram_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x00, 0x00, 0xb0, 0x01
    };

    TEST_ASSERT(knx_telegram_get_priority(telegram) == KNX_TELEGRAM_PRIORITY_LOW);
}

void test_knx_telegram_get_priority__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    TEST_ASSERT(knx_telegram_get_priority(telegram) == KNX_TELEGRAM_PRIORITY_LOW);
}

void test_knx_telegram_set_priority__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);
    knx_telegram_set_priority(telegram, KNX_TELEGRAM_PRIORITY_SYSTEM);

    TEST_ASSERT(telegram[0] == 0x90);
}

void test_knx_telegram_set_priority__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);
    knx_telegram_set_priority(telegram, KNX_TELEGRAM_PRIORITY_ALARM);

    TEST_ASSERT(telegram[0] == 0x18);
}

void test_knx_telegram_set_priority__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_telegram_set_priority(telegram, KNX_TELEGRAM_PRIORITY_ALARM);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

void test_knx_telegram_is_repeated__standard(void)
{
    knx_telegram_t telegram[] = {
        0x9c, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };

    TEST_ASSERT(knx_telegram_is_repeated(telegram));
}

void test_knx_telegram_is_repeated__extended(void)
{
    knx_telegram_t telegram[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x00, 0x00, 0xb0, 0x01
    };

    TEST_ASSERT(!knx_telegram_is_repeated(telegram));
}

void test_knx_telegram_is_repeated__other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    TEST_ASSERT(!knx_telegram_is_repeated(telegram));
}

void test_knx_telegram_set_repeated__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_STANDARD);

    knx_telegram_set_repeated(telegram, true);
    TEST_ASSERT((telegram[0] & 0x20) == 0x00);

    knx_telegram_set_repeated(telegram, false);
    TEST_ASSERT((telegram[0] & 0x20) == 0x20);
}

void test_knx_telegram_set_repeated__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };

    knx_telegram_set_type(telegram, KNX_TELEGRAM_TYPE_EXTENDED);

    knx_telegram_set_repeated(telegram, true);
    TEST_ASSERT((telegram[0] & 0x20) == 0x00);

    knx_telegram_set_repeated(telegram, false);
    TEST_ASSERT((telegram[0] & 0x20) == 0x20);
}

void test_knx_telegram_set_repeated_other(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_MIN_LEN] = { 0x00 };

    knx_telegram_set_repeated(telegram, true);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }

    knx_telegram_set_repeated(telegram, false);

    for (unsigned i = 0; i < sizeof(telegram); i++) {
        TEST_ASSERT(telegram[i] == 0x00);
    }
}

static void test_knx_telegram_build__standard(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN] = { 0x00 };
    knx_addr_t src = { { 0x11, 0x08 } };
    knx_addr_t dst = { { 0x08, 0x11 } };
    knx_addr_t tmp;

    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);

    TEST_ASSERT(knx_telegram_get_type(telegram) == KNX_TELEGRAM_TYPE_STANDARD);
    knx_telegram_get_src_addr(telegram, &tmp);
    TEST_ASSERT(tmp.u8[0] == 0x11);
    TEST_ASSERT(tmp.u8[1] == 0x08);

    knx_telegram_get_dst_addr(telegram, &tmp);
    TEST_ASSERT(tmp.u8[0] == 0x08);
    TEST_ASSERT(tmp.u8[1] == 0x11);
    TEST_ASSERT(!knx_telegram_is_group_addressed(telegram));
}

static void test_knx_telegram_build__extended(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN] = { 0x00 };
    knx_addr_t src = { { 0x11, 0x08 } };
    knx_addr_t dst = { { 0x08, 0x11 } };
    knx_addr_t tmp;

    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_EXTENDED, &src, &dst, true);

    TEST_ASSERT(knx_telegram_get_type(telegram) == KNX_TELEGRAM_TYPE_EXTENDED);
    knx_telegram_get_src_addr(telegram, &tmp);
    TEST_ASSERT(tmp.u8[0] == 0x11);
    TEST_ASSERT(tmp.u8[1] == 0x08);

    knx_telegram_get_dst_addr(telegram, &tmp);
    TEST_ASSERT(tmp.u8[0] == 0x08);
    TEST_ASSERT(tmp.u8[1] == 0x11);
    TEST_ASSERT(knx_telegram_is_group_addressed(telegram));
}

static void test_knx_telegram_build__poll(void)
{
    knx_telegram_t telegram[KNX_TELEGRAM_TYPE_POLL_MIN_LEN] = { 0x00 };
    knx_addr_t src = { { 0x11, 0x08 } };
    knx_addr_t dst = { { 0x08, 0x11 } };
    knx_addr_t tmp;

    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_POLL, &src, &dst, false);

    TEST_ASSERT(knx_telegram_get_type(telegram) == KNX_TELEGRAM_TYPE_POLL);
    knx_telegram_get_src_addr(telegram, &tmp);
    TEST_ASSERT(tmp.u8[0] == 0x11);
    TEST_ASSERT(tmp.u8[1] == 0x08);

    knx_telegram_get_dst_addr(telegram, &tmp);
    TEST_ASSERT(tmp.u8[0] == 0x08);
    TEST_ASSERT(tmp.u8[1] == 0x11);
}

Test *tests_knx_telegram_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_knx_telegram_is__valid),
        new_TestFixture(test_knx_telegram_is__null),
        new_TestFixture(test_knx_telegram_is__unknown),
        new_TestFixture(test_knx_telegram_is__too_long),
        new_TestFixture(test_knx_telegram_is__too_long_for_standard),
        new_TestFixture(test_knx_telegram_is__too_short),
        new_TestFixture(test_knx_telegram_is__too_short_for_extended),
        new_TestFixture(test_knx_telegram_checksum__correct),
        new_TestFixture(test_knx_telegram_get_checksum__correct),
        new_TestFixture(test_knx_telegram_set_checksum__correct),
        new_TestFixture(test_knx_telegram_update_checksum__correct),
        new_TestFixture(test_knx_telegram_is_checksum_valid__correct),
        new_TestFixture(test_knx_telegram_is_checksum_valid__incorrect),
        new_TestFixture(test_knx_telegram_get_type__standard),
        new_TestFixture(test_knx_telegram_get_type__extended),
        new_TestFixture(test_knx_telegram_get_type__poll),
        new_TestFixture(test_knx_telegram_get_type__unknown),
        new_TestFixture(test_knx_telegram_set_type__standard),
        new_TestFixture(test_knx_telegram_set_type__extended),
        new_TestFixture(test_knx_telegram_set_type__poll),
        new_TestFixture(test_knx_telegram_set_type__unknown),
        new_TestFixture(test_knx_telegram_get_src_addr__standard),
        new_TestFixture(test_knx_telegram_get_src_addr__extended),
        new_TestFixture(test_knx_telegram_get_src_addr__poll),
        new_TestFixture(test_knx_telegram_get_src_addr__unknown),
        new_TestFixture(test_knx_telegram_get_src_addr__null),
        new_TestFixture(test_knx_telegram_set_src_addr__standard),
        new_TestFixture(test_knx_telegram_set_src_addr__extended),
        new_TestFixture(test_knx_telegram_set_src_addr__poll),
        new_TestFixture(test_knx_telegram_set_src_addr__unknown),
        new_TestFixture(test_knx_telegram_set_src_addr__null),
        new_TestFixture(test_knx_telegram_get_dst_addr__standard),
        new_TestFixture(test_knx_telegram_get_dst_addr__extended),
        new_TestFixture(test_knx_telegram_get_dst_addr__poll),
        new_TestFixture(test_knx_telegram_get_dst_addr__unknown),
        new_TestFixture(test_knx_telegram_get_dst_addr__null),
        new_TestFixture(test_knx_telegram_set_dst_addr__standard),
        new_TestFixture(test_knx_telegram_set_dst_addr__extended),
        new_TestFixture(test_knx_telegram_set_dst_addr__poll),
        new_TestFixture(test_knx_telegram_set_dst_addr__unknown),
        new_TestFixture(test_knx_telegram_set_dst_addr__null),
        new_TestFixture(test_knx_telegram_is_group_addressed__standard),
        new_TestFixture(test_knx_telegram_is_group_addressed__extended),
        new_TestFixture(test_knx_telegram_is_group_addressed__other),
        new_TestFixture(test_knx_telegram_set_group_addressed__standard),
        new_TestFixture(test_knx_telegram_set_group_addressed__extended),
        new_TestFixture(test_knx_telegram_set_group_addressed__other),
        new_TestFixture(test_knx_telegram_get_payload__standard__unmerged),
        new_TestFixture(test_knx_telegram_get_payload__standard__merged),
        new_TestFixture(test_knx_telegram_get_payload__extended__unmerged),
        new_TestFixture(test_knx_telegram_get_payload__extended__merged),
        new_TestFixture(test_knx_telegram_get_payload__other),
        new_TestFixture(test_knx_telegram_get_payload_length__standard),
        new_TestFixture(test_knx_telegram_get_payload_length__extended),
        new_TestFixture(test_knx_telegram_get_payload_length__other),
        new_TestFixture(test_knx_telegram_set_payload_length__standard),
        new_TestFixture(test_knx_telegram_set_payload_length__standard__too_long),
        new_TestFixture(test_knx_telegram_set_payload_length__extended),
        new_TestFixture(test_knx_telegram_set_payload_length__extended__too_long),
        new_TestFixture(test_knx_telegram_set_payload_length__other),
        new_TestFixture(test_knx_telegram_get_routing_count__standard),
        new_TestFixture(test_knx_telegram_get_routing_count__extended),
        new_TestFixture(test_knx_telegram_get_routing_count__other),
        new_TestFixture(test_knx_telegram_set_routing_count__standard),
        new_TestFixture(test_knx_telegram_set_routing_count__standard__too_long),
        new_TestFixture(test_knx_telegram_set_routing_count__extended),
        new_TestFixture(test_knx_telegram_set_routing_count__extended__too_long),
        new_TestFixture(test_knx_telegram_set_routing_count__other),
        new_TestFixture(test_knx_telegram_get_priority__standard),
        new_TestFixture(test_knx_telegram_get_priority__extended),
        new_TestFixture(test_knx_telegram_get_priority__other),
        new_TestFixture(test_knx_telegram_set_priority__standard),
        new_TestFixture(test_knx_telegram_set_priority__extended),
        new_TestFixture(test_knx_telegram_set_priority__other),
        new_TestFixture(test_knx_telegram_is_repeated__standard),
        new_TestFixture(test_knx_telegram_is_repeated__extended),
        new_TestFixture(test_knx_telegram_is_repeated__other),
        new_TestFixture(test_knx_telegram_set_repeated__standard),
        new_TestFixture(test_knx_telegram_set_repeated__extended),
        new_TestFixture(test_knx_telegram_set_repeated_other),
        new_TestFixture(test_knx_telegram_build__standard),
        new_TestFixture(test_knx_telegram_build__extended),
        new_TestFixture(test_knx_telegram_build__poll),
    };

    EMB_UNIT_TESTCALLER(knx_telegram_tests, NULL, NULL, fixtures);

    return (Test *)&knx_telegram_tests;
}
