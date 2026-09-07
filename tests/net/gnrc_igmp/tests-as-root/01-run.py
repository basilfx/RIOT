#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
# SPDX-License-Identifier: LGPL-2.1-only

import os
import re
import subprocess
import sys
import time

from scapy.all import AsyncSniffer, Ether, IP, UDP, sendp
from scapy.contrib.igmp import IGMP
from testrunner import run

DEVICE_ADDR = "192.168.10.2"
PREFIX_LEN = 24
GROUP_PORT = 12345

GROUP_A = "239.1.2.3"
GROUP_B = "239.4.5.6"
GROUP_C = "239.7.8.9"

# a group join schedules a randomized repeat of its unsolicited report
# somewhere in [0, CONFIG_GNRC_IGMP_UNSOLICITED_REPORT_INTERVAL_MS] after the
# immediate one (RFC 2236, section 7.10). Several scenarios below rely on
# that repeat having already fired before they start asserting that *no*
# further, spontaneous report arrives for a group -- keep this in sync with
# the CFLAGS override of CONFIG_GNRC_IGMP_UNSOLICITED_REPORT_INTERVAL_MS in
# ../Makefile.
UNSOLICITED_REPORT_INTERVAL_S = 0.5

# a Query's Max Response Time is carried in units of 1/10 second (RFC 2236,
# section 2); mrcode=20 below (scapy.contrib.igmp.IGMP's field for this is
# named "mrcode", not "mrtime") means at most 2.0s until the device's own,
# randomly-delayed, response.
QUERY_MRTIME = 20
QUERY_MAX_DELAY_S = QUERY_MRTIME / 10
# generous margin over the query's own max delay, to absorb scheduling
# jitter without making the suite unnecessarily slow
SNIFF_MARGIN_S = 0.5


def pktbuf_empty(child):
    child.sendline("pktbuf")
    child.expect(r"packet buffer: first byte: (?P<first_byte>0x[0-9a-fA-F]+), "
                 r"last byte: 0x[0-9a-fA-F]+ \(size: (?P<size>\d+)\)")
    first_byte = child.match.group("first_byte")
    size = child.match.group("size")
    child.expect(
            r"~ unused: {} \(next: (\(nil\)|0), size: {}\) ~".format(
                first_byte, size))


def _join_group(child, iface, addr):
    child.sendline("ifconfig {} add {}".format(iface, addr))
    child.expect("success: added")
    # let the group's randomized repeated unsolicited report (if any) fire
    # now, rather than risk it landing inside a later scenario's sniff
    # window for the same group
    time.sleep(UNSOLICITED_REPORT_INTERVAL_S + 0.3)


def _leave_group(child, iface, addr):
    child.sendline("ifconfig {} del {}".format(iface, addr))
    child.expect("success: removed")


def _igmp_report_filter(ip_dst):
    return lambda pkt: (pkt.haslayer(IGMP) and pkt.haslayer(IP) and
                        (pkt[IP].src == ip_dst) and
                        (pkt[IGMP].type == 0x16))


def _igmp_leave_filter(ip_dst):
    return lambda pkt: (pkt.haslayer(IGMP) and pkt.haslayer(IP) and
                        (pkt[IP].src == ip_dst) and
                        (pkt[IGMP].type == 0x17))


def _start_sniffer(iface, lfilter):
    # AsyncSniffer, rather than a plain blocking sniff() called after
    # sending: the device can reply before a post-hoc sniff() call even
    # starts listening, and srp1()'s answer-matching heuristics don't
    # reliably correlate a bare IP fragment "request" (no recognizable
    # protocol layer of its own) with a reply either -- both risk a false
    # pass (no answer matched) even when the device did in fact reply.
    # Starting the sniffer first and sending only once it is up avoids
    # the race.
    sniffer = AsyncSniffer(iface=iface, lfilter=lfilter)
    sniffer.start()
    time.sleep(0.2)  # let the sniffer thread actually start listening
    return sniffer


def _stop_sniffer(sniffer, wait):
    time.sleep(wait)
    sniffer.stop()
    return sniffer.results[0] if sniffer.results else None


def test_join_sends_unsolicited_report(child, iface, hw_dst, ip_dst, ip_src):
    # a host must send an unsolicited Version 2 Membership Report
    # immediately upon joining a group (RFC 2236, section 7.10), well
    # before any Query is even received. gnrc_igmp_group_joined() sends it
    # to the group address itself, with TTL 1 (gnrc_igmp.c's _send_igmp()).
    sniffer = _start_sniffer(iface, _igmp_report_filter(ip_dst))
    _join_group(child, iface, GROUP_A)
    sniffer.stop()
    reports = [pkt for pkt in sniffer.results if pkt[IGMP].gaddr == GROUP_A]

    assert len(reports) >= 1, "no unsolicited membership report observed"
    report = reports[0]
    assert report[IP].dst == GROUP_A
    assert report[IP].ttl == 1
    assert report[IGMP].gaddr == GROUP_A
    # a correct checksum makes IGMP's own one's-complement self-check
    # return 0 once the field is cleared and recomputed
    recomputed = IGMP(bytes(report[IGMP]))
    recomputed.chksum = None
    assert bytes(recomputed)[2:4] == bytes(report[IGMP])[2:4]
    pktbuf_empty(child)


def test_general_query_triggers_report(child, iface, hw_dst, ip_dst, ip_src):
    # a General Query (group address 0.0.0.0) must make every group a host
    # currently holds membership in report back, after a random delay
    # bounded by the Query's Max Response Time (gnrc_igmp.c's
    # _handle_query() calls _schedule_report() for every matching cache
    # entry).
    sniffer = _start_sniffer(iface, _igmp_report_filter(ip_dst))
    query = Ether(dst=hw_dst) / IP(dst="224.0.0.1", src=ip_src, ttl=1) / \
        IGMP(type=0x11, mrcode=QUERY_MRTIME, gaddr="0.0.0.0")
    sendp(query, iface=iface, verbose=0)
    p = _stop_sniffer(sniffer, wait=QUERY_MAX_DELAY_S + SNIFF_MARGIN_S)

    assert p is not None, "no membership report observed after general query"
    assert p[IGMP].gaddr == GROUP_A
    pktbuf_empty(child)


def test_group_specific_query_only_answers_that_group(
        child, iface, hw_dst, ip_dst, ip_src):
    # a Group-Specific Query only asks for a report on the one group it
    # names; a host that is also a member of some other group must not
    # answer for that other group (gnrc_igmp.c's _handle_query() only
    # calls _schedule_report() for cache entries whose group matches the
    # query's, when the query's group address is not unspecified).
    _join_group(child, iface, GROUP_B)

    sniffer = _start_sniffer(iface, _igmp_report_filter(ip_dst))
    query = Ether(dst=hw_dst) / IP(dst=GROUP_A, src=ip_src, ttl=1) / \
        IGMP(type=0x11, mrcode=QUERY_MRTIME, gaddr=GROUP_A)
    sendp(query, iface=iface, verbose=0)
    time.sleep(QUERY_MAX_DELAY_S + SNIFF_MARGIN_S)
    sniffer.stop()
    groups = {pkt[IGMP].gaddr for pkt in sniffer.results}

    assert GROUP_A in groups, "no membership report observed for GROUP_A"
    assert GROUP_B not in groups, \
        "device answered a group-specific query for a different group"

    # leave GROUP_B again so the rest of the suite only ever has to reason
    # about GROUP_A's (and, later, GROUP_C's) state. GROUP_B's
    # last_reporter is still true at this point (nothing above cleared
    # it -- the group-specific query only touched GROUP_A), so this leave
    # genuinely triggers gnrc_igmp_group_left()'s asynchronous Leave Group
    # dispatch; give it the same margin every other scenario in this file
    # gives its own final send before checking the packet buffer.
    _leave_group(child, iface, GROUP_B)
    time.sleep(1)
    pktbuf_empty(child)


def test_report_from_another_host_suppresses_ours(
        child, iface, hw_dst, ip_dst, ip_src):
    # RFC 2236, section 3: a host cancels its own pending response to a
    # Query for a group as soon as it hears another host's report for that
    # same group, since the group's membership has already been reported
    # to the querier.
    #
    # Note: this only actually suppresses anything if the device's own
    # response is still *pending* at the moment the foreign report
    # arrives -- i.e. a Query has to be sent first to arm the delay timer
    # (_handle_query() in gnrc_igmp.c), immediately followed by the
    # foreign report (_handle_report() then cancels it and clears
    # last_reporter). Sending the foreign report *before* any Query is
    # outstanding would not suppress anything at all: _handle_query()
    # unconditionally (re-)arms a fresh response timer for every Query it
    # receives, regardless of any report already observed, so a Query sent
    # afterwards would still be answered. The foreign report's source IP
    # does not matter either way: _handle_report() only inspects the
    # IGMP payload's group address, never the IP source address.
    query = Ether(dst=hw_dst) / IP(dst="224.0.0.1", src=ip_src, ttl=1) / \
        IGMP(type=0x11, mrcode=QUERY_MRTIME, gaddr="0.0.0.0")
    foreign_report = Ether(dst=hw_dst) / \
        IP(dst=GROUP_A, src="192.168.10.99", ttl=1) / \
        IGMP(type=0x16, gaddr=GROUP_A)

    sniffer = _start_sniffer(iface, _igmp_report_filter(ip_dst))
    sendp(query, iface=iface, verbose=0)
    sendp(foreign_report, iface=iface, verbose=0)
    p = _stop_sniffer(sniffer, wait=QUERY_MAX_DELAY_S + SNIFF_MARGIN_S)

    assert p is None, \
        "device answered despite a foreign report suppressing it"
    pktbuf_empty(child)


def test_leave_as_last_reporter_sends_leave(
        child, iface, hw_dst, ip_dst, ip_src):
    # a host must send a Leave Group message when leaving a group it was
    # the last host to report on -- but only in that case (RFC 2236,
    # section 6; gnrc_igmp_group_left() only sends it when
    # entry->last_reporter is still true).
    #
    # The previous scenario suppressed the device's own report, which
    # clears last_reporter as a side effect, so it can no longer be relied
    # on here. Re-establish it explicitly by sending a General Query and
    # waiting for the device's own, this time unsuppressed, response
    # before leaving.
    query = Ether(dst=hw_dst) / IP(dst="224.0.0.1", src=ip_src, ttl=1) / \
        IGMP(type=0x11, mrcode=QUERY_MRTIME, gaddr="0.0.0.0")
    sniffer = _start_sniffer(iface, _igmp_report_filter(ip_dst))
    sendp(query, iface=iface, verbose=0)
    p = _stop_sniffer(sniffer, wait=QUERY_MAX_DELAY_S + SNIFF_MARGIN_S)
    assert p is not None and p[IGMP].gaddr == GROUP_A, \
        "device did not report GROUP_A, cannot re-establish last-reporter"

    sniffer = _start_sniffer(iface, _igmp_leave_filter(ip_dst))
    _leave_group(child, iface, GROUP_A)
    p = _stop_sniffer(sniffer, wait=1)

    assert p is not None, "no leave group message observed"
    assert p[IP].dst == "224.0.0.2"
    assert p[IGMP].type == 0x17
    assert p[IGMP].gaddr == GROUP_A
    pktbuf_empty(child)


def test_leave_after_suppression_sends_no_leave(
        child, iface, hw_dst, ip_dst, ip_src):
    # unlike the previous scenario's suppression-of-a-pending-response,
    # _handle_report() in gnrc_igmp.c unconditionally clears a cache
    # entry's last_reporter flag whenever any report for that group
    # arrives, whether or not a Query is currently outstanding -- so a
    # bare foreign report, with no Query involved at all, is enough to
    # make sure no Leave Group message follows.
    _join_group(child, iface, GROUP_C)

    foreign_report = Ether(dst=hw_dst) / \
        IP(dst=GROUP_C, src="192.168.10.99", ttl=1) / \
        IGMP(type=0x16, gaddr=GROUP_C)
    sendp(foreign_report, iface=iface, verbose=0)
    time.sleep(0.2)

    sniffer = _start_sniffer(iface, _igmp_leave_filter(ip_dst))
    _leave_group(child, iface, GROUP_C)
    p = _stop_sniffer(sniffer, wait=1)

    assert p is None, "leave group message sent despite prior suppression"
    pktbuf_empty(child)


def test_group_reception(child, iface, hw_dst, ip_dst, ip_src):
    # a UDP datagram addressed to a joined multicast group must actually
    # reach the transport layer, not be silently dropped by
    # gnrc_ipv4.c's _receive_ipv4() group-membership admission check.
    #
    # An ICMPv4 error reply cannot be used to prove this: RFC 1122, section
    # 3.2.2 forbids generating any ICMPv4 error for a datagram whose own
    # destination is a multicast address, and
    # gnrc_icmpv4_error.c's _check_ipv4_hdr() correctly enforces exactly
    # that, so a destination-port-unreachable reply would never arrive here
    # even when reception works perfectly.
    #
    # Instead, prove reception directly from the device's own shell output:
    # testfunc() already started a UDP server on GROUP_PORT, whose receive
    # callback is gnrc_pktdump (see ../Makefile). gnrc_pktdump's _dump()
    # prints a "NETTYPE_UDP (<n>)" line for every UDP snip it dumps, so
    # observing that line on the device after sending the datagram is
    # sufficient proof the datagram reached the transport layer.
    #
    # The Ethernet destination is deliberately hw_dst (the device's own
    # unicast MAC), exactly like every other scenario in this suite,
    # rather than the RFC 1112 multicast MAC the group address maps to:
    # this isolates the IP-layer admission check under test here from the
    # driver's separate L2 multicast MAC filter (Task 1's concern, already
    # covered elsewhere), which would otherwise make this scenario's result
    # depend on the native tap driver's/host bridge's multicast forwarding
    # behaviour instead of only on gnrc_ipv4.c.
    _join_group(child, iface, GROUP_A)

    sendp(Ether(dst=hw_dst) / IP(dst=GROUP_A, src=ip_src) /
          UDP(sport=54321, dport=GROUP_PORT) / b"ABCD",
          iface=iface, verbose=0)
    child.expect(r"NETTYPE_UDP \(\d+\)")
    pktbuf_empty(child)


def check_and_search_output(cmd, pattern, res_group, *args, **kwargs):
    output = subprocess.check_output(cmd, *args, **kwargs).decode("utf-8")
    for line in output.splitlines():
        m = re.search(pattern, line)
        if m is not None:
            return m.group(res_group)
    return None


def get_bridge(tap):
    res = check_and_search_output(
            ["bridge", "link"],
            r"{}.+master\s+(?P<master>[^\s]+)".format(tap),
            "master"
        )
    return tap if res is None else res


def get_host_addr(tap):
    res = check_and_search_output(
            ["ip", "-4", "addr", "show", "dev", tap],
            r"inet (?P<addr>[0-9.]+)/\d+",
            "addr"
        )
    if res is None:
        raise AssertionError(
                "Can't find host IPv4 address on interface {}".format(tap)
            )
    return res


def testfunc(child):
    tap = get_bridge(os.environ["TAP"])
    ip_src = get_host_addr(tap)

    child.sendline("ifconfig")
    child.expect(r"Iface\s+(?P<iface>\S+)")
    iface = child.match.group("iface")
    child.expect(r"HWaddr: (?P<hwaddr>[A-Fa-f:0-9]+)\s")
    hwaddr_dst = child.match.group("hwaddr").lower()

    child.sendline("ifconfig {} add {}/{}".format(
            iface, DEVICE_ADDR, PREFIX_LEN
        ))
    child.expect("success: added")

    # start a UDP server on GROUP_PORT so test_group_reception() has
    # something to prove reception with: gnrc_pktdump (registered as the
    # server's callback) prints a distinctive line for every packet it
    # actually receives at the transport layer.
    child.sendline("udp server start {}".format(GROUP_PORT))
    child.expect("Success: started UDP server")

    def run(func):
        if child.logfile == sys.stdout:
            func(child, tap, hwaddr_dst, DEVICE_ADDR, ip_src)
        else:
            try:
                func(child, tap, hwaddr_dst, DEVICE_ADDR, ip_src)
                print(".", end="", flush=True)
            except Exception as e:
                print("FAILED")
                raise e

    # ordering rationale: scenario 3 leaves GROUP_B again at its end, so
    # every later scenario only ever has to reason about GROUP_A's (and,
    # later, GROUP_C's) state; scenario 4 clears GROUP_A's last-reporter
    # flag by design, so scenario 5 explicitly re-establishes it via its
    # own General Query rather than assuming it is still set from
    # scenario 1; scenario 6 uses GROUP_C throughout, so it is entirely
    # independent of GROUP_A's state; scenario 7 re-joins GROUP_A since
    # scenario 5 already left it.
    run(test_join_sends_unsolicited_report)
    run(test_general_query_triggers_report)
    run(test_group_specific_query_only_answers_that_group)
    run(test_report_from_another_host_suppresses_ours)
    run(test_leave_as_last_reporter_sends_leave)
    run(test_leave_after_suppression_sends_no_leave)
    run(test_group_reception)

    print("SUCCESS")


if __name__ == "__main__":
    if os.geteuid() != 0:
        print("\x1b[1;31mThis test requires root privileges.\n"
              "It's constructing and sending Ethernet frames.\x1b[0m\n",
              file=sys.stderr)
        sys.exit(1)
    sys.exit(run(testfunc, timeout=1, echo=False))
