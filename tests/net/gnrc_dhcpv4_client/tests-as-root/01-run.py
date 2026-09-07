#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
# SPDX-License-Identifier: LGPL-2.1-only

import os
import re
import subprocess
import sys
import time

from scapy.all import AsyncSniffer, BOOTP, DHCP, Ether, IP, UDP, sendp
from testrunner import run

SERVER_IP = "192.168.10.1"
LEASE_ADDR = "192.168.10.2"
PREFIX_LEN = 24
LEASE_TIME = 3600

# CONFIG_DHCPV4_CLIENT_INIT_DELAY_MS is overridden in ../Makefile to keep
# this suite fast without giving up the jitter behaviour entirely; keep
# this in sync with that override.
INIT_DELAY_S = 0.3
# generous margin over the jittered restart delay above, to absorb
# scheduling jitter without making the suite unnecessarily slow
RESTART_MARGIN_S = 2


def pktbuf_empty(child):
    child.sendline("pktbuf")
    child.expect(r"packet buffer: first byte: (?P<first_byte>0x[0-9a-fA-F]+), "
                 r"last byte: 0x[0-9a-fA-F]+ \(size: (?P<size>\d+)\)")
    first_byte = child.match.group("first_byte")
    size = child.match.group("size")
    child.expect(
            r"~ unused: {} \(next: (\(nil\)|0), size: {}\) ~".format(
                first_byte, size))


def _dhcp_option(pkt, name):
    for opt in pkt[DHCP].options:
        if isinstance(opt, tuple) and opt[0] == name:
            return opt[1]
    return None


def _is_dhcp(pkt, msg_type):
    return (pkt.haslayer(BOOTP) and pkt.haslayer(DHCP) and
           (pkt[BOOTP].op == 1) and
           (_dhcp_option(pkt, "message-type") == msg_type))


def _discover_filter():
    return lambda pkt: _is_dhcp(pkt, 1)


def _request_filter():
    return lambda pkt: _is_dhcp(pkt, 3)


def _start_sniffer(iface, lfilter):
    # AsyncSniffer, rather than a plain blocking sniff() called after
    # sending: the device can reply before a post-hoc sniff() call even
    # starts listening. Starting the sniffer first and giving it a moment
    # to actually start listening avoids the race.
    sniffer = AsyncSniffer(iface=iface, lfilter=lfilter)
    sniffer.start()
    time.sleep(0.2)
    return sniffer


def _stop_sniffer(sniffer, wait):
    time.sleep(wait)
    sniffer.stop()
    return sniffer.results[0] if sniffer.results else None


def _reply(iface, req, msg_type, extra_options=()):
    bootp_req = req[BOOTP]
    reply = (
        Ether(dst="ff:ff:ff:ff:ff:ff") /
        IP(src=SERVER_IP, dst="255.255.255.255") /
        UDP(sport=67, dport=68) /
        BOOTP(op=2, yiaddr=LEASE_ADDR, siaddr=SERVER_IP, xid=bootp_req.xid,
              flags=bootp_req.flags, chaddr=bootp_req.chaddr) /
        DHCP(options=[
            ("message-type", msg_type),
            ("server_id", SERVER_IP),
            ("subnet_mask", "255.255.255.0"),
            ("router", SERVER_IP),
            ("lease_time", LEASE_TIME),
            ("renewal_time", LEASE_TIME // 2),
            ("rebinding_time", (LEASE_TIME * 7) // 8),
            *extra_options,
            "end",
        ])
    )
    sendp(reply, iface=iface, verbose=0)
    return reply


def test_nak_on_request_restarts_discovery(child, iface):
    # RFC 2131, section 3.1, step 4: a DHCPNAK to the client's REQUEST must
    # send it back to INIT, where it discovers again with a fresh
    # transaction ID (dhcpv4_client.c's _request() posts init_event on
    # DHCPV4_MSG_NAK).
    sniffer = _start_sniffer(iface, _discover_filter())
    first_discover = _stop_sniffer(sniffer, wait=INIT_DELAY_S + RESTART_MARGIN_S)
    assert first_discover is not None, "no initial DISCOVER observed"
    first_xid = first_discover[BOOTP].xid

    sniffer = _start_sniffer(iface, _request_filter())
    _reply(iface, first_discover, "offer")
    req = _stop_sniffer(sniffer, wait=5)
    assert req is not None, "no REQUEST observed after OFFER"
    assert _dhcp_option(req, "requested_addr") == LEASE_ADDR
    assert _dhcp_option(req, "server_id") == SERVER_IP

    sniffer = _start_sniffer(iface, _discover_filter())
    _reply(iface, req, "nak")
    second_discover = _stop_sniffer(
            sniffer, wait=INIT_DELAY_S + RESTART_MARGIN_S)

    assert second_discover is not None, \
        "no fresh DISCOVER observed after NAK"
    assert second_discover[BOOTP].xid != first_xid, \
        "restart after NAK reused the old transaction ID"
    pktbuf_empty(child)


def test_dora_configures_lease(child, iface):
    # the full DORA exchange (DISCOVER, OFFER, REQUEST, ACK) must configure
    # the offered address on the interface, flagged as DHCP-assigned
    # (GNRC_NETIF_IPV4_ADDRS_FLAGS_STATE_DHCP,
    # gnrc_dhcpv4_client's dhcpv4_client_conf_lease()), with the prefix
    # length derived from the subnet mask option.
    #
    # A classless static route (option 121, RFC 3442) is included in the
    # ACK purely to exercise gnrc_dhcpv4/client.c's parsing path -- there is
    # no shell command to introspect gnrc_ipv4_ft's forwarding table, so
    # this cannot be asserted on directly here.
    sniffer = _start_sniffer(iface, _discover_filter())
    discover = _stop_sniffer(sniffer, wait=INIT_DELAY_S + RESTART_MARGIN_S)
    assert discover is not None, "no DISCOVER observed"

    sniffer = _start_sniffer(iface, _request_filter())
    _reply(iface, discover, "offer")
    req = _stop_sniffer(sniffer, wait=5)
    assert req is not None, "no REQUEST observed after OFFER"
    assert _dhcp_option(req, "requested_addr") == LEASE_ADDR
    assert _dhcp_option(req, "server_id") == SERVER_IP

    # option 121 (RFC 3442): one route, 10.0.0.0/16 via SERVER_IP, encoded
    # as (16, 10, 0, <router 4 bytes>) -- use the raw option code rather
    # than a scapy name, since not every scapy version defines one for it
    classless_route = bytes([16, 10, 0]) + bytes(
            int(o) for o in SERVER_IP.split("."))
    _reply(iface, req, "ack", extra_options=[(121, classless_route)])

    for _ in range(20):
        child.sendline("ifconfig")
        child.expect(r"HWaddr")
        idx = child.expect([
                r"inet addr:\s*{}/{}\s*DHCP".format(
                    LEASE_ADDR.replace(".", r"\."), PREFIX_LEN),
                r">",
            ])
        if idx == 0:
            break
        time.sleep(0.5)
    else:
        raise AssertionError("leased address never appeared in ifconfig")
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


def testfunc(child):
    iface = get_bridge(os.environ["TAP"])

    child.sendline("ifconfig")
    child.expect(r"Iface\s+(?P<iface>\S+)")

    def run_scenario(func):
        if child.logfile == sys.stdout:
            func(child, iface)
        else:
            try:
                func(child, iface)
                print(".", end="", flush=True)
            except Exception as e:
                print("FAILED")
                raise e

    # the NAK scenario is run first since it depends on catching the
    # device's very first, auto_init-triggered DISCOVER; the DORA success
    # scenario reuses the fresh DISCOVER that scenario leaves behind.
    run_scenario(test_nak_on_request_restarts_discovery)
    run_scenario(test_dora_configures_lease)

    print("SUCCESS")


if __name__ == "__main__":
    if os.geteuid() != 0:
        print("\x1b[1;31mThis test requires root privileges.\n"
              "It's constructing and sending Ethernet frames.\x1b[0m\n",
              file=sys.stderr)
        sys.exit(1)
    sys.exit(run(testfunc, timeout=1, echo=False))
