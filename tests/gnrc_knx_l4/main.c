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
 * @brief       Tests GNRC KNX Layer 4 stack implementation
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
#include "net/gnrc/knx_l4.h"
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
 * @brief   Reference to the connection variable in KNX L4 (used as a test
 *          probe to assert certain conditions)
 */
static knx_connection_t *connection;

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

    /* reset the connection status */
    connection = gnrc_knx_l4_get_connection();

    memset(connection, 0, sizeof(knx_connection_t));
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

    gnrc_netapi_dispatch_receive(GNRC_NETTYPE_KNX_L4, GNRC_NETREG_DEMUX_CTX_ALL, pkt);
}

static void send_telegram(uint8_t *data, size_t len)
{
    send_telegram_type(data, len, GNRC_NETTYPE_KNX_L4);
}

static void test_gnrc_knx_l4__receive__not_knx_l4(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);

    send_telegram_type(telegram,
                       KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN,
                       GNRC_NETTYPE_UNDEF);

    TEST_ASSERT_EQUAL_INT(0, msg_avail());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l4__receive__not_a_telegram__empty(void)
{
    send_telegram(telegram, 0);

    TEST_ASSERT_EQUAL_INT(0, msg_avail());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l4__receive__connect(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT(connection->connected == true);
    TEST_ASSERT(knx_addr_equal(&(connection->dest), &src));
}

static void test_gnrc_knx_l4__receive__connect_twice(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    uint64_t timestamp = connection->timestamp;

    /* send another connection request */
    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT(connection->timestamp == timestamp);
}

static void test_gnrc_knx_l4__receive__disconnect__from_same(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* send disconnect from same src */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_DISCONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT(connection->connected == false);
}

static void test_gnrc_knx_l4__receive__disconnect__from_other(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* send disconnect from another src */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &dst, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_DISCONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT(connection->connected == true);
    TEST_ASSERT(knx_addr_equal(&(connection->dest), &src));
}

static void test_gnrc_knx_l4__receive__disconnect_twice(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* send disconnect from same src */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_DISCONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* send another disconnect */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT(connection->connected == false);
}

static void test_gnrc_knx_l4__receive__ack__from_same(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* send an ACK for something (artificially) sent */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_NCD);
    knx_tpci_ncd_set(telegram, KNX_TPCI_NCD_ACK);
    knx_tpci_set_seq_number(telegram, 0);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT(connection->src_seq == 1);
}

static void test_gnrc_knx_l4__receive__ack__unexpected(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* send an ACK with an unexpected sequence number */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_NCD);
    knx_tpci_ncd_set(telegram, KNX_TPCI_NCD_ACK);
    knx_tpci_set_seq_number(telegram, 10);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT(connection->src_seq == 0);
}

static void test_gnrc_knx_l4__receive__ack__from_other(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* send disconnect from same src */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &dst, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_NCD);
    knx_tpci_ncd_set(telegram, KNX_TPCI_NCD_ACK);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT(connection->dest_seq == 0);
}

static void test_gnrc_knx_l4__receive__telegram__connectionless(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UDP);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(1, msg_avail());
    TEST_ASSERT(!gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l4__receive__telegram__connected(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_NDP);
    knx_tpci_set_seq_number(telegram, 0);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(2, msg_avail());
    TEST_ASSERT(!gnrc_pktbuf_is_empty());
}

static void test_gnrc_knx_l4__receive__telegram__wrong_sequence_number(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* send first telegram */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_NDP);
    knx_tpci_set_seq_number(telegram, 1);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* send second telegram */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_NDP);
    knx_tpci_set_seq_number(telegram, 10);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT(connection->connected == false);
}

static void test_gnrc_knx_l4__receive__telegram__timeout(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* send first telegram */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_NDP);
    knx_tpci_set_seq_number(telegram, 1);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* wait for timeout (changing the timestamp instead of sleeping) */
    connection->timestamp = 0;

    /* send second telegram */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_NDP);
    knx_tpci_set_seq_number(telegram, 2);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT(connection->connected == false);
}

static void test_gnrc_knx_l4__receive__telegram__from_other(void)
{
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &src, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_UCD);
    knx_tpci_ucd_set(telegram, KNX_TPCI_UCD_CONNECT);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    /* send a telegram from another source */
    knx_telegram_build(telegram, KNX_TELEGRAM_TYPE_STANDARD, &dst, &dst, false);
    knx_tpci_set(telegram, KNX_TPCI_NDP);

    send_telegram(telegram, KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN);

    TEST_ASSERT_EQUAL_INT(0, msg_avail());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static Test *tests_gnrc_knx_l4(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_gnrc_knx_l4__receive__not_knx_l4),
        new_TestFixture(test_gnrc_knx_l4__receive__not_a_telegram__empty),
        new_TestFixture(test_gnrc_knx_l4__receive__connect),
        new_TestFixture(test_gnrc_knx_l4__receive__disconnect__from_same),
        new_TestFixture(test_gnrc_knx_l4__receive__disconnect__from_other),
        new_TestFixture(test_gnrc_knx_l4__receive__connect_twice),
        new_TestFixture(test_gnrc_knx_l4__receive__disconnect_twice),
        new_TestFixture(test_gnrc_knx_l4__receive__ack__from_same),
        new_TestFixture(test_gnrc_knx_l4__receive__ack__unexpected),
        new_TestFixture(test_gnrc_knx_l4__receive__ack__from_other),
        new_TestFixture(test_gnrc_knx_l4__receive__telegram__connectionless),
        new_TestFixture(test_gnrc_knx_l4__receive__telegram__connected),
        new_TestFixture(test_gnrc_knx_l4__receive__telegram__wrong_sequence_number),
        new_TestFixture(test_gnrc_knx_l4__receive__telegram__timeout),
        new_TestFixture(test_gnrc_knx_l4__receive__telegram__from_other),
    };

    EMB_UNIT_TESTCALLER(tests, set_up, NULL, fixtures);

    return (Test *)&tests;
}

int main(void)
{
    _tests_init();

    TESTS_START();
    TESTS_RUN(tests_gnrc_knx_l4());
    TESTS_END();

    return 0;
}
