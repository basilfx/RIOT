/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Build-matrix test for a dual-stack (IPv4 and IPv6) GNRC build
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 * @}
 */

#include <stdio.h>

#include "net/gnrc/ipv4.h"
#include "net/gnrc/ipv6.h"

int main(void)
{
    /* gnrc_ipv4_init() and gnrc_ipv6_init() both run from auto_init before
     * main(), independently of any actual network interface. Printing the
     * resulting PIDs lets the test script confirm that both network-layer
     * threads started, not just that this configuration links. */
    printf("gnrc_ipv4_pid: %d\n", (int)gnrc_ipv4_pid);
    printf("gnrc_ipv6_pid: %d\n", (int)gnrc_ipv6_pid);

    puts("SUCCESS");

    return 0;
}
