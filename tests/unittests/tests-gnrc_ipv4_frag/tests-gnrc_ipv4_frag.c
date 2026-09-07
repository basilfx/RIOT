/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 */
#include <string.h>

#include "embUnit.h"

#include "byteorder.h"
#include "net/gnrc/ipv4/frag.h"
#include "net/gnrc/pktbuf.h"
#include "net/ipv4/hdr.h"

#include "tests-gnrc_ipv4_frag.h"

#define TEST_PROTOCOL   (200U)     /* https://tools.ietf.org/html/rfc3692 */

static const ipv4_addr_t _src = { .u8 = { 10, 0, 0, 1 } };
static const ipv4_addr_t _dst = { .u8 = { 10, 0, 0, 2 } };

static void setUp(void)
{
    /* release any dangling reassembly (left over from a previous test)
     * *before* gnrc_pktbuf_init() invalidates the pktbuf pointers it
     * still holds */
    gnrc_ipv4_frag_reset();
    gnrc_pktbuf_init();
}

static gnrc_pktsnip_t *_build_fragment(uint16_t id, uint16_t offset_bytes,
                                       bool more_frags, const void *data,
                                       size_t len)
{
    gnrc_pktsnip_t *hdr_snip, *payload;
    ipv4_hdr_t *hdr;

    hdr_snip = gnrc_pktbuf_add(NULL, NULL, sizeof(ipv4_hdr_t), GNRC_NETTYPE_IPV4);
    if (hdr_snip == NULL) {
        return NULL;
    }
    hdr = hdr_snip->data;
    memset(hdr, 0, sizeof(ipv4_hdr_t));
    ipv4_hdr_set_version(hdr);
    ipv4_hdr_set_ihl(hdr, sizeof(ipv4_hdr_t));
    hdr->src = _src;
    hdr->dst = _dst;
    hdr->protocol = TEST_PROTOCOL;
    hdr->ttl = 64;
    hdr->id = byteorder_htons(id);
    ipv4_hdr_set_flags(hdr, more_frags ? IPV4_HDR_FLAGS_MF : 0);
    ipv4_hdr_set_fo(hdr, offset_bytes / 8U);
    hdr->tl = byteorder_htons(sizeof(ipv4_hdr_t) + len);

    /* payload snip becomes the head: [payload] -> [header], matching the
     * receive-order shape the IPv4 receive path hands to
     * gnrc_ipv4_frag_reass() */
    payload = gnrc_pktbuf_add(hdr_snip, data, len, GNRC_NETTYPE_UNDEF);
    if (payload == NULL) {
        gnrc_pktbuf_release(hdr_snip);
        return NULL;
    }
    return payload;
}

static void _check_reassembled(gnrc_pktsnip_t *pkt, uint16_t expect_id,
                               const uint8_t *expect_data, size_t expect_len)
{
    gnrc_pktsnip_t *ipv4 = gnrc_pktsnip_search_type(pkt, GNRC_NETTYPE_IPV4);
    ipv4_hdr_t *hdr;

    TEST_ASSERT_NOT_NULL(ipv4);
    hdr = ipv4->data;
    TEST_ASSERT_EQUAL_INT(0, ipv4_hdr_get_fo(hdr));
    TEST_ASSERT_EQUAL_INT(0, ipv4_hdr_get_flags(hdr) & IPV4_HDR_FLAGS_MF);
    TEST_ASSERT_EQUAL_INT(expect_id, byteorder_ntohs(hdr->id));
    TEST_ASSERT_EQUAL_INT(TEST_PROTOCOL, hdr->protocol);
    TEST_ASSERT_EQUAL_INT(sizeof(ipv4_hdr_t) + expect_len,
                          byteorder_ntohs(hdr->tl));
    TEST_ASSERT_EQUAL_INT(expect_len, pkt->size);
    TEST_ASSERT_MESSAGE(memcmp(pkt->data, expect_data, expect_len) == 0,
                        "reassembled payload mismatch");
}

static void test_frag_reass__in_order(void)
{
    static const uint8_t part0[64] = { [0] = 0xaa, [63] = 0xbb };
    static const uint8_t part1[30] = { [0] = 0xcc, [29] = 0xdd };
    uint8_t expect[94];
    gnrc_pktsnip_t *frag;
    gnrc_pktsnip_t *res;

    memcpy(expect, part0, sizeof(part0));
    memcpy(expect + sizeof(part0), part1, sizeof(part1));

    frag = _build_fragment(0x1001, 0, true, part0, sizeof(part0));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(1, gnrc_ipv4_frag_rbuf_used());

    frag = _build_fragment(0x1001, sizeof(part0), false, part1, sizeof(part1));
    TEST_ASSERT_NOT_NULL(frag);
    res = gnrc_ipv4_frag_reass(frag);
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_INT(0, gnrc_ipv4_frag_rbuf_used());

    _check_reassembled(res, 0x1001, expect, sizeof(expect));
    gnrc_pktbuf_release(res);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_frag_reass__out_of_order(void)
{
    static const uint8_t part0[64] = { [0] = 0x11, [63] = 0x22 };
    static const uint8_t part1[16] = { [0] = 0x33, [15] = 0x44 };
    uint8_t expect[80];
    gnrc_pktsnip_t *frag;
    gnrc_pktsnip_t *res;

    memcpy(expect, part0, sizeof(part0));
    memcpy(expect + sizeof(part0), part1, sizeof(part1));

    /* last fragment arrives first */
    frag = _build_fragment(0x1002, sizeof(part0), false, part1, sizeof(part1));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(1, gnrc_ipv4_frag_rbuf_used());

    frag = _build_fragment(0x1002, 0, true, part0, sizeof(part0));
    TEST_ASSERT_NOT_NULL(frag);
    res = gnrc_ipv4_frag_reass(frag);
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_INT(0, gnrc_ipv4_frag_rbuf_used());

    _check_reassembled(res, 0x1002, expect, sizeof(expect));
    gnrc_pktbuf_release(res);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_frag_reass__duplicate_is_ignored(void)
{
    static const uint8_t part0[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    static const uint8_t part1[8] = { 8, 7, 6, 5, 4, 3, 2, 1 };
    uint8_t expect[16];
    gnrc_pktsnip_t *frag;
    gnrc_pktsnip_t *res;

    memcpy(expect, part0, sizeof(part0));
    memcpy(expect + sizeof(part0), part1, sizeof(part1));

    frag = _build_fragment(0x1003, 0, true, part0, sizeof(part0));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));

    /* byte-identical resend of the same fragment must not disturb the
     * in-progress reassembly */
    frag = _build_fragment(0x1003, 0, true, part0, sizeof(part0));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(1, gnrc_ipv4_frag_rbuf_used());

    frag = _build_fragment(0x1003, sizeof(part0), false, part1, sizeof(part1));
    TEST_ASSERT_NOT_NULL(frag);
    res = gnrc_ipv4_frag_reass(frag);
    TEST_ASSERT_NOT_NULL(res);

    _check_reassembled(res, 0x1003, expect, sizeof(expect));
    gnrc_pktbuf_release(res);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_frag_reass__overlap_discards_datagram(void)
{
    static const uint8_t part0[16] = { 0 };
    static const uint8_t overlap[16] = { 0 };
    static const uint8_t part1[8] = { 0 };
    gnrc_pktsnip_t *frag;

    frag = _build_fragment(0x1004, 0, true, part0, sizeof(part0));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(1, gnrc_ipv4_frag_rbuf_used());

    /* [8, 24) overlaps the already-received [0, 16) but is not identical
     * to any interval on file: RFC 791 security posture is to discard the
     * *entire* in-progress reassembly, not attempt to reconcile it */
    frag = _build_fragment(0x1004, 8, true, overlap, sizeof(overlap));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(0, gnrc_ipv4_frag_rbuf_used());

    /* the datagram is gone: a fragment that would have completed the
     * original reassembly must not silently do so */
    frag = _build_fragment(0x1004, 16, false, part1, sizeof(part1));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(1, gnrc_ipv4_frag_rbuf_used());

    /* fill the gap left by the discarded reassembly (fresh, unrelated,
     * "more fragments" so it does not itself conflict with the last
     * fragment already on file for this id) to leave a clean packet
     * buffer for subsequent tests */
    frag = _build_fragment(0x1004, 0, true, part0, sizeof(part0));
    TEST_ASSERT_NOT_NULL(frag);
    gnrc_pktbuf_release(gnrc_ipv4_frag_reass(frag));

    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_frag_reass__exceeds_established_length(void)
{
    static const uint8_t part0[8] = { 0 };
    static const uint8_t part1[8] = { 0 };
    static const uint8_t bogus[16] = { 0 };
    gnrc_pktsnip_t *frag;

    /* a non-last first fragment, so the datagram is not yet complete */
    frag = _build_fragment(0x1005, 0, true, part0, sizeof(part0));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));

    /* the last fragment, arriving out of order and leaving a gap at
     * [8, 16): establishes datagram_len == 24 without completing */
    frag = _build_fragment(0x1005, 16, false, part1, sizeof(part1));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(1, gnrc_ipv4_frag_rbuf_used());

    /* a fragment claiming to extend past the already-known datagram
     * length is malformed (or an attack): discard */
    frag = _build_fragment(0x1005, 20, false, bogus, sizeof(bogus));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(0, gnrc_ipv4_frag_rbuf_used());

    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_frag_reass__rbuf_full_evicts_oldest(void)
{
    static const uint8_t part[8] = { 0 };
    gnrc_pktsnip_t *frag;
    gnrc_pktsnip_t *res;

    /* fill both reassembly buffer slots (CONFIG_GNRC_IPV4_FRAG_RBUF_SIZE
     * == 2 for this test binary) with incomplete datagrams */
    frag = _build_fragment(0x2001, 0, true, part, sizeof(part));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));

    frag = _build_fragment(0x2002, 0, true, part, sizeof(part));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(2, gnrc_ipv4_frag_rbuf_used());

    /* a third, different datagram must evict one of the (both still
     * incomplete) existing ones rather than being dropped itself */
    frag = _build_fragment(0x2003, 0, true, part, sizeof(part));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(2, gnrc_ipv4_frag_rbuf_used());

    /* the newly added datagram must be one of the two survivors: complete
     * it to prove it was not the one dropped */
    frag = _build_fragment(0x2003, sizeof(part), false, part, sizeof(part));
    TEST_ASSERT_NOT_NULL(frag);
    res = gnrc_ipv4_frag_reass(frag);
    TEST_ASSERT_NOT_NULL(res);
    gnrc_pktbuf_release(res);

    /* whichever of 0x2001/0x2002 survived the eviction is left dangling
     * (incomplete, never timed out) on purpose here -- gnrc_ipv4_frag_reset()
     * in setUp() discards it before the next test, exactly like a real
     * timeout eventually would */
}

static void test_frag_reass__teardrop_nested_fragment_discards_datagram(void)
{
    static const uint8_t part0[16] = { 0 };
    static const uint8_t nested[4] = { 0 };
    gnrc_pktsnip_t *frag;

    frag = _build_fragment(0x1006, 0, true, part0, sizeof(part0));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(1, gnrc_ipv4_frag_rbuf_used());

    /* the classic Teardrop shape: [4, 8) sits entirely inside the
     * already-received [0, 16), which drove vulnerable stacks' signed
     * "gap length" arithmetic negative and corrupted the reassembly
     * buffer. This implementation must simply discard the datagram.
     * Marked as the last fragment so the length is only exercising the
     * overlap check, not the non-last-fragment multiple-of-8 rule */
    frag = _build_fragment(0x1006, 4, false, nested, sizeof(nested));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(0, gnrc_ipv4_frag_rbuf_used());

    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_frag_reass__non_last_fragment_length_not_multiple_of_8_rejected(void)
{
    /* a non-last fragment's payload length must be a multiple of 8 (the
     * fragment offset unit): a stack that skips this check can be driven
     * into misaligned reassembly by a single malformed fragment */
    static const uint8_t part[7] = { 0 };
    gnrc_pktsnip_t *frag;

    frag = _build_fragment(0x1007, 0, true, part, sizeof(part));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(0, gnrc_ipv4_frag_rbuf_used());

    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_frag_reass__ping_of_death_oversized_datagram_rejected(void)
{
    /* RFC 791's 16-bit total-length field caps a reassembled datagram at
     * 65535 bytes; a fragment set claiming to extend past that (the
     * original "Ping of Death") must be rejected outright, before any
     * reassembly state for it is even allocated */
    static const uint8_t tail[8] = { 0 };
    gnrc_pktsnip_t *frag;

    frag = _build_fragment(0x1008, 65512, false, tail, sizeof(tail));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(0, gnrc_ipv4_frag_rbuf_used());

    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_frag_reass__interval_pool_exhaustion_discards_gracefully(void)
{
    /* CONFIG_GNRC_IPV4_FRAG_LIMITS_POOL_SIZE == 8 for this test binary.
     * A flood of small, disjoint fragments for one datagram (a
     * bookkeeping-exhaustion DoS) must exhaust the pool and discard that
     * one datagram cleanly, rather than corrupting state or affecting
     * unrelated reassemblies */
    static const uint8_t part[8] = { 0 };
    gnrc_pktsnip_t *frag;

    for (uint16_t i = 0; i < 8; i++) {
        frag = _build_fragment(0x1009, i * sizeof(part), true, part, sizeof(part));
        TEST_ASSERT_NOT_NULL(frag);
        TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    }
    TEST_ASSERT_EQUAL_INT(1, gnrc_ipv4_frag_rbuf_used());

    /* the ninth disjoint fragment has nowhere left in the shared interval
     * pool: the datagram is discarded instead of silently dropping the
     * bookkeeping for an already-stored range */
    frag = _build_fragment(0x1009, 8 * sizeof(part), false, part, sizeof(part));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    TEST_ASSERT_EQUAL_INT(0, gnrc_ipv4_frag_rbuf_used());

    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

static void test_frag_reass__zero_length_fragments_do_not_exhaust_interval_pool(void)
{
    /* zero-length fragments carry no byte range and must not consume the
     * shared interval pool: a flood of them is a no-op, not a way to
     * starve bookkeeping for every other in-progress reassembly */
    static const uint8_t part0[8] = { 0xaa };
    static const uint8_t part1[8] = { 0xbb };
    uint8_t expect[16];
    gnrc_pktsnip_t *frag;
    gnrc_pktsnip_t *res;

    memcpy(expect, part0, sizeof(part0));
    memcpy(expect + sizeof(part0), part1, sizeof(part1));

    for (unsigned i = 0; i < 32; i++) {
        frag = _build_fragment(0x100a, 0, true, NULL, 0);
        TEST_ASSERT_NOT_NULL(frag);
        TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));
    }
    TEST_ASSERT_EQUAL_INT(1, gnrc_ipv4_frag_rbuf_used());

    frag = _build_fragment(0x100a, 0, true, part0, sizeof(part0));
    TEST_ASSERT_NOT_NULL(frag);
    TEST_ASSERT_NULL(gnrc_ipv4_frag_reass(frag));

    frag = _build_fragment(0x100a, sizeof(part0), false, part1, sizeof(part1));
    TEST_ASSERT_NOT_NULL(frag);
    res = gnrc_ipv4_frag_reass(frag);
    TEST_ASSERT_NOT_NULL(res);

    _check_reassembled(res, 0x100a, expect, sizeof(expect));
    gnrc_pktbuf_release(res);
    TEST_ASSERT(gnrc_pktbuf_is_sane());
    TEST_ASSERT(gnrc_pktbuf_is_empty());
}

Test *tests_gnrc_ipv4_frag_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_frag_reass__in_order),
        new_TestFixture(test_frag_reass__out_of_order),
        new_TestFixture(test_frag_reass__duplicate_is_ignored),
        new_TestFixture(test_frag_reass__overlap_discards_datagram),
        new_TestFixture(test_frag_reass__exceeds_established_length),
        new_TestFixture(test_frag_reass__rbuf_full_evicts_oldest),
        new_TestFixture(test_frag_reass__teardrop_nested_fragment_discards_datagram),
        new_TestFixture(test_frag_reass__non_last_fragment_length_not_multiple_of_8_rejected),
        new_TestFixture(test_frag_reass__ping_of_death_oversized_datagram_rejected),
        new_TestFixture(test_frag_reass__interval_pool_exhaustion_discards_gracefully),
        new_TestFixture(test_frag_reass__zero_length_fragments_do_not_exhaust_interval_pool),
    };

    EMB_UNIT_TESTCALLER(gnrc_ipv4_frag_tests, setUp, NULL, fixtures);

    return (Test *)&gnrc_ipv4_frag_tests;
}

void tests_gnrc_ipv4_frag(void)
{
    TESTS_RUN(tests_gnrc_ipv4_frag_tests());
}
/** @} */
