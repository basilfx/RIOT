#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
# SPDX-License-Identifier: LGPL-2.1-only

"""
Build-matrix test for a dual-stack (IPv4 and IPv6) GNRC build.

This is a compile and boot smoke test, not a functional networking test: it
has no network interface set up and sends no packets. Its only job is to
confirm that a build with both `gnrc_ipv4` and `gnrc_ipv6_default` (plus the
family-aware upper layers on top of them) compiles, links and starts both
network-layer threads, since nothing else in the test suite builds this
combination.
"""

import sys

from testrunner import run

KERNEL_PID_UNDEF = 0


def testfunc(child):
    child.expect(r'gnrc_ipv4_pid: (-?\d+)')
    assert int(child.match.group(1)) != KERNEL_PID_UNDEF

    child.expect(r'gnrc_ipv6_pid: (-?\d+)')
    assert int(child.match.group(1)) != KERNEL_PID_UNDEF

    child.expect_exact('SUCCESS')


if __name__ == "__main__":
    sys.exit(run(testfunc))
