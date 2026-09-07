#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
# SPDX-License-Identifier: LGPL-2.1-only

import os
import re
import subprocess
import sys
import time

from scapy.all import AsyncSniffer, Ether, IP, UDP, ICMP, fragment, sendp, \
    sniff, srp1
from testrunner import run

DEVICE_ADDR = "192.168.10.2"
PREFIX_LEN = 24
UNBOUND_PORT = 12345
# keep in sync with CONFIG_GNRC_IPV4_FRAG_RBUF_TIMEOUT_MS in ../Makefile
FRAG_TIMEOUT_S = 5


def pktbuf_empty(child):
    child.sendline("pktbuf")
    child.expect(r"packet buffer: first byte: (?P<first_byte>0x[0-9a-fA-F]+), "
                 r"last byte: 0x[0-9a-fA-F]+ \(size: (?P<size>\d+)\)")
    first_byte = child.match.group("first_byte")
    size = child.match.group("size")
    child.expect(
            r"~ unused: {} \(next: (\(nil\)|0), size: {}\) ~".format(
                first_byte, size))


def test_port_unreachable(child, iface, hw_dst, ip_dst, ip_src):
    # nothing is listening on UNBOUND_PORT, so the device is expected to
    # reply with an ICMPv4 destination unreachable (port unreachable)
    # message, quoting our UDP datagram
    p = srp1(Ether(dst=hw_dst) / IP(dst=ip_dst, src=ip_src) /
             UDP(sport=54321, dport=UNBOUND_PORT) / b"ABCD",
             iface=iface, timeout=1, verbose=0)
    assert p is not None
    assert ICMP in p
    assert p[ICMP].type == 3   # destination unreachable
    assert p[ICMP].code == 3   # port unreachable
    # RFC 792 quotes the IP header + 64 bits of the original datagram, so
    # parse the whole thing as an IP packet again rather than byte-comparing
    # against a standalone UDP() (whose checksum differs from the one
    # actually put on the wire, fixed up only once serialized as part of
    # the full Ether/IP/UDP chain)
    quoted = IP(bytes(p[ICMP].payload))
    assert UDP in quoted
    assert quoted[UDP].sport == 54321
    assert quoted[UDP].dport == UNBOUND_PORT
    assert bytes(quoted[UDP].payload) == b"ABCD"
    pktbuf_empty(child)


def test_ttl_exceeded(child, iface, hw_dst, ip_dst, ip_src):
    # a datagram that already arrived with ttl 0 must never be processed,
    # regardless of whether anyone is listening on the destination port
    p = srp1(Ether(dst=hw_dst) / IP(dst=ip_dst, src=ip_src, ttl=0) /
             UDP(sport=54321, dport=UNBOUND_PORT) / b"ABCD",
             iface=iface, timeout=1, verbose=0)
    assert p is not None
    assert ICMP in p
    assert p[ICMP].type == 11  # time exceeded
    assert p[ICMP].code == 0   # ttl exceeded in transit
    pktbuf_empty(child)


def _icmp_reply_filter(ip_dst, ip_src):
    return lambda pkt: (pkt.haslayer(ICMP) and pkt.haslayer(IP) and
                        (pkt[IP].src == ip_dst) and (pkt[IP].dst == ip_src))


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


def test_frag_in_order(child, iface, hw_dst, ip_dst, ip_src):
    # a UDP datagram split into properly-ordered, in-order fragments must
    # reassemble correctly: sent to an unbound port, the reassembled
    # datagram is expected to trigger the same ICMPv4 port-unreachable
    # path an unfragmented datagram would, proving the whole thing (not
    # just the first fragment) made it through
    payload = bytes(range(256)) * 2
    req = UDP(sport=54321, dport=UNBOUND_PORT) / payload
    frags = fragment(IP(dst=ip_dst, src=ip_src) / req, fragsize=256)
    assert len(frags) > 1
    sniffer = _start_sniffer(iface, _icmp_reply_filter(ip_dst, ip_src))
    for f in frags:
        sendp(Ether(dst=hw_dst) / f, iface=iface, verbose=0)
    p = _stop_sniffer(sniffer, wait=2)
    assert p is not None
    assert p[ICMP].type == 3
    assert p[ICMP].code == 3
    quoted = IP(bytes(p[ICMP].payload))
    assert UDP in quoted
    assert quoted[UDP].sport == 54321
    assert quoted[UDP].dport == UNBOUND_PORT
    pktbuf_empty(child)


def test_frag_out_of_order(child, iface, hw_dst, ip_dst, ip_src):
    # the same datagram, but with its fragments delivered in reverse
    # order (the offset-0 fragment, carrying the actual header, arrives
    # last): reassembly must not depend on arrival order
    payload = bytes(range(256))
    req = UDP(sport=54322, dport=UNBOUND_PORT) / payload
    frags = fragment(IP(dst=ip_dst, src=ip_src) / req, fragsize=64)
    assert len(frags) > 2
    sniffer = _start_sniffer(iface, _icmp_reply_filter(ip_dst, ip_src))
    for f in reversed(frags):
        sendp(Ether(dst=hw_dst) / f, iface=iface, verbose=0)
    p = _stop_sniffer(sniffer, wait=2)
    assert p is not None
    assert p[ICMP].type == 3
    assert p[ICMP].code == 3
    pktbuf_empty(child)


def test_frag_duplicate(child, iface, hw_dst, ip_dst, ip_src):
    # a byte-identical resend of an already-received fragment must be
    # silently ignored, not disturb the in-progress reassembly
    payload = bytes(range(64))
    req = UDP(sport=54323, dport=UNBOUND_PORT) / payload
    frags = fragment(IP(dst=ip_dst, src=ip_src) / req, fragsize=32)
    assert len(frags) >= 2
    sniffer = _start_sniffer(iface, _icmp_reply_filter(ip_dst, ip_src))
    sendp(Ether(dst=hw_dst) / frags[0], iface=iface, verbose=0)
    sendp(Ether(dst=hw_dst) / frags[0], iface=iface, verbose=0)
    for f in frags[1:]:
        sendp(Ether(dst=hw_dst) / f, iface=iface, verbose=0)
    p = _stop_sniffer(sniffer, wait=2)
    assert p is not None
    assert p[ICMP].type == 3
    assert p[ICMP].code == 3
    pktbuf_empty(child)


def test_frag_overlap_discards(child, iface, hw_dst, ip_dst, ip_src):
    # a fragment that partially, but not identically, overlaps a range
    # already received for the same datagram is a well-known source of
    # security bugs (RFC 791 has no reconciliation rule for it): the
    # *entire* in-progress reassembly must be discarded, not silently
    # completed with mismatched, overlapping data
    ident = 0xbeef
    part_a = bytes([0xaa]) * 64
    part_b = bytes([0xbb]) * 8

    first = IP(dst=ip_dst, src=ip_src, id=ident, flags="MF", frag=0) / part_a
    # overlaps first's [0, 64) at [8, 72): same datagram, different
    # content, not identical to the interval already on file
    bogus = IP(dst=ip_dst, src=ip_src, id=ident, flags="MF", frag=1) / \
        (b"X" * 64)
    last = IP(dst=ip_dst, src=ip_src, id=ident, frag=8) / part_b

    sniffer = _start_sniffer(iface, _icmp_reply_filter(ip_dst, ip_src))
    sendp(Ether(dst=hw_dst) / first, iface=iface, verbose=0)
    sendp(Ether(dst=hw_dst) / bogus, iface=iface, verbose=0)
    sendp(Ether(dst=hw_dst) / last, iface=iface, verbose=0)
    p = _stop_sniffer(sniffer, wait=2)
    assert p is None, "datagram must not reassemble after an overlap"
    pktbuf_empty(child)


def test_frag_timeout(child, iface, hw_dst, ip_dst, ip_src):
    # a datagram that never completes must eventually be discarded, with
    # an ICMPv4 time exceeded (fragment reassembly time exceeded) message
    # sent back, quoting the (only) fragment received
    ident = 0xcafe
    # a non-last fragment's payload length must be a multiple of 8 bytes
    # (the fragment-offset field's unit): 8 (UDP header) + 8 (data) == 16
    first = IP(dst=ip_dst, src=ip_src, id=ident, flags="MF", frag=0) / \
        (UDP(sport=54325, dport=UNBOUND_PORT) / b"ABCDEFGH")

    sendp(Ether(dst=hw_dst) / first, iface=iface, verbose=0)

    def _is_time_exceeded(pkt):
        return pkt.haslayer(ICMP) and (pkt[ICMP].type == 11)

    got = sniff(iface=iface, timeout=FRAG_TIMEOUT_S + 5,
               lfilter=_is_time_exceeded, count=1)
    assert len(got) == 1, "no ICMPv4 time exceeded after reassembly timeout"
    assert got[0][ICMP].code == 1
    quoted = IP(bytes(got[0][ICMP].payload))
    assert UDP in quoted
    assert quoted[UDP].sport == 54325
    pktbuf_empty(child)


def test_oversized_transmit(child, iface, hw_dst, ip_dst, ip_src):
    # a locally-originated echo request bigger than the interface MTU
    # must be fragmented, not silently dropped or truncated
    size = 1600

    sniffer = _start_sniffer(
            iface,
            lambda pkt: pkt.haslayer(IP) and (pkt[IP].src == ip_dst) and
                        (pkt[IP].dst == ip_src)
        )
    child.sendline("ping4 -c 1 -s {} {}".format(size, ip_src))
    child.expect(r"1 packets transmitted")
    time.sleep(0.5)
    sniffer.stop()
    got = sniffer.results

    assert len(got) >= 2, \
        "a {} byte echo request must be fragmented".format(size)
    ids = {pkt[IP].id for pkt in got}
    assert len(ids) == 1, "all fragments must share one Identification value"

    frags = sorted(got, key=lambda p: p[IP].frag)
    offset = 0
    for i, pkt in enumerate(frags):
        assert (pkt[IP].frag * 8) == offset, \
            "gap or overlap between fragments"
        more = bool(pkt[IP].flags.MF)
        is_last = (i == len(frags) - 1)
        assert more != is_last, \
            "exactly the last fragment must not have the MF flag set"
        offset += len(bytes(pkt[IP].payload))

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

    run(test_port_unreachable)
    run(test_ttl_exceeded)
    run(test_frag_in_order)
    run(test_frag_out_of_order)
    run(test_frag_duplicate)
    run(test_frag_overlap_discards)
    run(test_frag_timeout)
    run(test_oversized_transmit)

    print("SUCCESS")


if __name__ == "__main__":
    if os.geteuid() != 0:
        print("\x1b[1;31mThis test requires root privileges.\n"
              "It's constructing and sending Ethernet frames.\x1b[0m\n",
              file=sys.stderr)
        sys.exit(1)
    sys.exit(run(testfunc, timeout=1, echo=False))
