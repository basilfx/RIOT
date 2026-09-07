/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 */
#include "embUnit/embUnit.h"

#include "byteorder.h"
#include "net/gnrc/igmp.h"
#include "net/gnrc/pktbuf.h"
#include "net/igmp.h"
#include "net/inet_csum.h"

#include "tests-gnrc_igmp.h"

static void setUp(void)
{
    gnrc_pktbuf_init();
}

static void test_gnrc_igmp_build__report_fields(void)
{
    ipv4_addr_t group = { .u8 = { 224, 1, 2, 3 } };
    gnrc_pktsnip_t *pkt = gnrc_igmp_build(IGMP_V2_MEMBERSHIP_REPORT, 0, &group);
    igmp_hdr_t *hdr;

    TEST_ASSERT_NOT_NULL(pkt);
    TEST_ASSERT_EQUAL_INT(sizeof(igmp_hdr_t), pkt->size);
    hdr = pkt->data;
    TEST_ASSERT_EQUAL_INT(IGMP_V2_MEMBERSHIP_REPORT, hdr->type);
    TEST_ASSERT_EQUAL_INT(0, hdr->max_resp_time);
    TEST_ASSERT(ipv4_addr_equal(&group, &hdr->group_addr));
    gnrc_pktbuf_release(pkt);
}

static void test_gnrc_igmp_build__checksum_is_valid(void)
{
    ipv4_addr_t group = { .u8 = { 224, 1, 2, 3 } };
    gnrc_pktsnip_t *pkt = gnrc_igmp_build(IGMP_MEMBERSHIP_QUERY, 100, &group);
    uint16_t csum;

    TEST_ASSERT_NOT_NULL(pkt);
    /* a valid IGMP checksum makes the 16-bit one's complement sum of the
     * whole message (checksum field included) equal to 0xffff, i.e.
     * inet_csum() over the whole thing, complemented, is 0 */
    csum = inet_csum(0, pkt->data, pkt->size);
    TEST_ASSERT_EQUAL_INT(0, (uint16_t) ~csum);
    gnrc_pktbuf_release(pkt);
}

static void test_gnrc_igmp_demux__drops_short_packet(void)
{
    gnrc_netif_t netif = { 0 };
    /* gnrc_igmp_demux() only asserts netif != NULL and never dereferences
     * it on this rejection path (the length check runs before any netif
     * access), so an otherwise-unregistered, zeroed netif is safe here */
    gnrc_pktsnip_t *pkt = gnrc_pktbuf_add(NULL, NULL, sizeof(igmp_hdr_t) - 1,
                                          GNRC_NETTYPE_UNDEF);

    TEST_ASSERT_NOT_NULL(pkt);
    /* must not crash on a too-short packet; consumes pkt */
    gnrc_igmp_demux(&netif, pkt);
}

static void test_gnrc_igmp_demux__drops_bad_checksum(void)
{
    gnrc_netif_t netif = { 0 };
    ipv4_addr_t group = { .u8 = { 224, 1, 2, 3 } };
    gnrc_pktsnip_t *pkt = gnrc_igmp_build(IGMP_V2_MEMBERSHIP_REPORT, 0, &group);
    igmp_hdr_t *hdr;

    TEST_ASSERT_NOT_NULL(pkt);
    hdr = pkt->data;
    /* flip every bit of an otherwise-valid checksum to corrupt it */
    hdr->csum.u16 ^= 0xffff;
    /* must not crash on a corrupted checksum; consumes pkt */
    gnrc_igmp_demux(&netif, pkt);
}

Test *tests_gnrc_igmp_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_gnrc_igmp_build__report_fields),
        new_TestFixture(test_gnrc_igmp_build__checksum_is_valid),
        new_TestFixture(test_gnrc_igmp_demux__drops_short_packet),
        new_TestFixture(test_gnrc_igmp_demux__drops_bad_checksum),
    };

    EMB_UNIT_TESTCALLER(gnrc_igmp_tests, setUp, NULL, fixtures);

    return (Test *)&gnrc_igmp_tests;
}

void tests_gnrc_igmp(void)
{
    TESTS_RUN(tests_gnrc_igmp_tests());
}
/** @} */
