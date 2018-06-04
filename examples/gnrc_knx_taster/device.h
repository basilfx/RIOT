/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef DEVICE_H
#define DEVICE_H

#include "net/netif.h"

#include "knx_device.h"

#ifdef __cplusplus
extern "C" {
#endif

extern knx_device_t device;

#define MSG_DEVICE_PROGRAMMING_MODE 100
#define MSG_DEVICE_LOAD 101
#define MSG_DEVICE_COM_OBJECT 102

typedef enum {
    DEVICE_BUTTON_FUNCTION_SWITCH           = 0,
    DEVICE_BUTTON_FUNCTION_SWITCH_SLL       = 20,
    DEVICE_BUTTON_FUNCTION_INTELLIGENT_DIM  = 60,
    DEVICE_BUTTON_FUNCTION_LED_OUTPUT       = 100,
} device_button_function_t;

typedef struct {
    uint8_t channel;
    knx_com_object_t *switch_a;
    knx_com_object_t *switch_b;
    knx_com_object_t *switch_c;
    knx_com_object_t *dimming;
    knx_com_object_t *value;
} device_button_t;

typedef struct {
    uint8_t channel;
    knx_com_object_t *value;
} device_sensor_t;

void device_reset(void);
void device_init(netif_t *iface);
kernel_pid_t device_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_H */
/** @} */
