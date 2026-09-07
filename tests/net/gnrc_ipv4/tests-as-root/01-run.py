#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
# SPDX-License-Identifier: LGPL-2.1-only

import os
import re
import subprocess
import sys

from scapy.all import Ether, IP, UDP, ICMP, sendp, srp1
from testrunner import run

DEVICE_ADDR = "192.168.10.2"
PREFIX_LEN = 24
UNBOUND_PORT = 12345


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

    print("SUCCESS")


if __name__ == "__main__":
    if os.geteuid() != 0:
        print("\x1b[1;31mThis test requires root privileges.\n"
              "It's constructing and sending Ethernet frames.\x1b[0m\n",
              file=sys.stderr)
        sys.exit(1)
    sys.exit(run(testfunc, timeout=1, echo=False))
