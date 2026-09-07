/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 * @brief   Test application for GNRC's IGMPv2 implementation
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <stdio.h>

#include "msg.h"
#include "shell.h"

#define MAIN_MSG_QUEUE_SIZE (8)
static msg_t _main_msg_queue[MAIN_MSG_QUEUE_SIZE];

int main(void)
{
    /* every GNRC test app that registers at gnrc_netreg from its main/shell
     * thread needs a message queue on that thread before doing so, otherwise
     * it asserts under DEVELHELP the first time a registration-using shell
     * command actually runs */
    msg_init_queue(_main_msg_queue, MAIN_MSG_QUEUE_SIZE);

    puts("GNRC IGMP test application");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
/** @} */
