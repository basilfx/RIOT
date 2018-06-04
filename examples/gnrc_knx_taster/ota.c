/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "riotboot/slot.h"
#include "riotboot/flashwrite.h"

#include "memory.h"
#include "ota.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

static riotboot_flashwrite_t _writer;

static inline void _set_slot(knx_device_t *device, uint8_t slot)
{
    device->objects[5].properties[4].content.value[0] = slot;
}

static inline void _set_state(knx_device_t *device, ota_state_t state)
{
    device->objects[5].properties[4].content.value[0] = (uint8_t) state;
}

void ota_write(knx_device_t *device, ota_event_t *ota_event)
{
    uint32_t offset = byteorder_ntohl(ota_event->offset);
    uint32_t size = byteorder_ntohl(ota_event->size);
    uint32_t total_size = byteorder_ntohl(ota_event->total_size);
    uint8_t expected_checksum = ota_event->checksum;

    DEBUG("[ota] ota_write: OTA offset=%" PRIu32 " size=%" PRIu32 " "
          "total_size=%" PRIu32 " checksum=%02x\n",
          offset,
          size,
          total_size,
          expected_checksum);

    if (!device->info->programming_mode) {
        DEBUG("[ota] ota_write: device not in programming mode\n");
        return;
    }

    uint8_t actual_checksum = 0;

    if ((size > total_size) || (offset > total_size) || (size > sizeof(ota))) {
        DEBUG("[ota] ota_write: block size mistmatch\n");
        return;
    }

    for (unsigned i = 0; i < size; i++) {
        actual_checksum = actual_checksum ^ ota[i];
    }

    if (actual_checksum != expected_checksum) {
        DEBUG("[ota] ota_write: block checksum mismatch\n");
        return;
    }

    bool more = (offset + size) != total_size;
    size_t skip = 0;

    if (offset == 0) {
        _set_state(device, OTA_STATE_BUSY);
        DEBUG("[ota] ota_write: initializing\n");

        if (riotboot_flashwrite_init(&_writer, riotboot_slot_other()) != 0) {
            _set_state(device, OTA_STATE_ERROR);
            DEBUG("[ota] ota_write: initialization failed\n");

            return;
        }

        /* the first block contains the RIOTBOOT_MAGIC, which must be skipped,
           since it will be written by the riotboot_flashwrite_finish method */
        size = size - RIOTBOOT_FLASHWRITE_SKIPLEN;
        skip = RIOTBOOT_FLASHWRITE_SKIPLEN;
    }

    if (riotboot_flashwrite_putbytes(&_writer, &(ota[skip]), size, more) != 0) {
        _set_state(device, OTA_STATE_ERROR);
        DEBUG("[ota] ota_write: write failed\n");

        return;
    }

    if (!more) {
        if (riotboot_flashwrite_finish(&_writer) != 0) {
            _set_state(device, OTA_STATE_ERROR);
            DEBUG("[ota] ota_write: finishing failed\n");

            return;
        }

        const riotboot_hdr_t *hdr = riotboot_slot_get_hdr(riotboot_slot_other());

        if (riotboot_hdr_validate(hdr) != 0) {
            _set_state(device, OTA_STATE_ERROR);
            DEBUG("[ota] ota_write: header not valid\n");

            return;
        }

        _set_state(device, OTA_STATE_FINISHED);
        DEBUG("[ota] ota_write: finished\n");
    }
}

void ota_init(knx_device_t *device)
{
    /* load the OTA settings */
    _set_slot(device, riotboot_slot_current());
    _set_state(device, OTA_STATE_READY);
}
