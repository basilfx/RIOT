/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef USER_SHELL_H
#define USER_SHELL_H

#include "knx_device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)) {
    uint8_t size;
    uint8_t data[8];
} shell_event_t;

void shell_read(shell_event_t *shell_event);
void shell_write(shell_event_t *shell_event);

#ifdef __cplusplus
}
#endif

#endif /* USER_SHELL_H */
/** @} */
