/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Tests GNRC KNX Layer 3 stack implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "common.h"

#include "thread.h"
#include "embUnit.h"
#include "embUnit/embUnit.h"
#include "net/knx.h"
#include "net/gnrc/knx_l3.h"
#include "net/gnrc/pktbuf.h"

/**
 * @brief   General-purpose buffer for a telegram (the +1 is for the tests that
 *          exceeds the maximum length)
 */
static knx_telegram_t telegram[KNX_TELEGRAM_MAX_LEN + 1];

/**
 * @brief   Source address definition
 */
static knx_addr_physical_t src = { { 0xff, 0x00 } };

/**
 * @brief   Destination address definition
 */
static knx_addr_physical_t dst = { { 0x00, 0xff } };

/**
 * @brief   Group destination address definition
 */
static knx_addr_physical_t group_dst = { { 0x77, 0x77 } };

static void set_up(void)
{
    /* empty the packet buffer */
    gnrc_pktbuf_init();

    /* empty message queue */
    while (msg_avail()) {
        msg_t msg;
        msg_receive(&msg);
    }

    /* clear the telegram */
    memset(telegram, 0, sizeof(telegram));
}

static void send_telegram_type(uint8_t *data, size_t len, gnrc_nettype_t type)
{
    gnrc_netif_t *iface = gnrc_netif_iter(NULL);

    gnrc_netif_hdr_t netif_hdr;

    gnrc_netif_hdr_init(&netif_hdr, KNX_ADDR_LEN, KNX_ADDR_LEN);

    netif_hdr.if_pid = iface->pid;

    gnrc_pktsnip_t *netif = gnrc_pktbuf_add(NULL,
                                            &netif_hdr,
                                            sizeof(netif_hdr),
                                            GNRC_NETTYPE_NETIF);
    gnrc_pktsnip_t *pkt = gnrc_pktbuf_add(netif,
                                          data,
                                          len,
                                          type);

    gnrc_netapi_dispatch_receive(GNRC_NETTYPE_KNX_L3, GNRC_NETREG_DEMUX_CTX_ALL, pkt);
}

static void send_telegram(uint8_t *data, size_t len)
{
    send_telegram_type(data, len, GNRC_NETTYPE_KNX_L3);
}

static void test_gnrc_knx_l3__receive__not_knx_l3(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);

    send_telegram_type(telegram,
                       KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN,
                       GNRC_NETTYPE_UNDEF);

    TEST_ASSERT_EQUAL_INT(0, msg_avail());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__not_a_telegram__empty(void)
{
    send_telegram(telegram, 0);

    TEST_ASSERT_EQUAL_INT(0, msg_avail());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__not_a_telegram__garbage(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_UNKNOWN, &src, &dst, false);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(0, msg_avail());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__not_a_telegram__too_long__standard(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MAX_LEN + 1);

    TEST_ASSERT_EQUAL_INT(0, msg_avail());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__not_a_telegram__too_long__extended(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_EXTENDED, &src, &dst, false);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_EXTENDED_MAX_LEN + 1);

    TEST_ASSERT_EQUAL_INT(0, msg_avail());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__physical_addressed__standard(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(1, msg_avail());
    TEST_ASSERT(!gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__physical_addressed__extended(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_EXTENDED, &src, &dst, false);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(1, msg_avail());
    TEST_ASSERT(!gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__group_addressed__standard(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &group_dst, true);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(1, msg_avail());
    TEST_ASSERT(!gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__group_addressed__extended(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_EXTENDED, &src, &group_dst, true);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(1, msg_avail());
    TEST_ASSERT(!gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__physical_addressed__not_for_me__standard(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &dst, &src, false);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(0, msg_avail());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__physical_addressed__not_for_me__extended(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_EXTENDED, &dst, &src, false);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(0, msg_avail());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__physical_addressed_broadcast__standard(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &knx_addr_broadcast, false);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(1, msg_avail());
    TEST_ASSERT(!gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l3__receive__physical_addressed_broadcast__extended(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_EXTENDED, &src, &knx_addr_broadcast, false);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(1, msg_avail());
    TEST_ASSERT(!gnrc_pktbuf_is_empty());
}

static Test *tests_gnrc_knx_l3(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_gnrc_knx_l3__receive__not_knx_l3),
        new_TestFixture(test_gnrc_knx_l3__receive__not_a_telegram__empty),
        new_TestFixture(test_gnrc_knx_l3__receive__not_a_telegram__garbage),
        new_TestFixture(test_gnrc_knx_l3__receive__not_a_telegram__too_long__standard),
        new_TestFixture(test_gnrc_knx_l3__receive__not_a_telegram__too_long__extended),
        new_TestFixture(test_gnrc_knx_l3__receive__physical_addressed__standard),
        new_TestFixture(test_gnrc_knx_l3__receive__physical_addressed__extended),
        new_TestFixture(test_gnrc_knx_l3__receive__group_addressed__standard),
        new_TestFixture(test_gnrc_knx_l3__receive__group_addressed__extended),
        new_TestFixture(test_gnrc_knx_l3__receive__physical_addressed__not_for_me__standard),
        new_TestFixture(test_gnrc_knx_l3__receive__physical_addressed__not_for_me__extended),
        new_TestFixture(test_gnrc_knx_l3__receive__physical_addressed_broadcast__standard),
        new_TestFixture(test_gnrc_knx_l3__receive__physical_addressed_broadcast__extended),
    };

    EMB_UNIT_TESTCALLER(tests, set_up, NULL, fixtures);

    return (Test *)&tests;
}

int main(void)
{
    _tests_init();

    TESTS_START();
    TESTS_RUN(tests_gnrc_knx_l3());
    TESTS_END();

    return 0;
}
