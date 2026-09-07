/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Tests ICMPv4 error message handling of the gnrc stack.
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 * @}
 */

#include "msg.h"
#include "shell.h"

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

static char line_buf[SHELL_DEFAULT_BUFSIZE];

int main(void)
{
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    return 0;
}
