/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef OTA_H
#define OTA_H

#include "knx_device.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OTA_STATE_READY,
    OTA_STATE_BUSY,
    OTA_STATE_ERROR,
    OTA_STATE_FINISHED
} ota_state_t;

typedef struct __attribute__((packed)) {
    network_uint32_t offset;
    network_uint32_t size;
    network_uint32_t total_size;
    uint8_t checksum;
} ota_event_t;

void ota_write(knx_device_t *device, ota_event_t *ota_event);
void ota_init(knx_device_t *device);

#ifdef __cplusplus
}
#endif

#endif /* OTA_H */
/** @} */
