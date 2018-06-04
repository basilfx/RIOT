/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <string.h>

#include "riotboot/slot.h"
#include "riotboot/flashwrite.h"

#include "heatshrink_decoder.h"

#include "properties.h"
#include "memory.h"
#include "ota.h"

#define ENABLE_DEBUG 0
#include "debug.h"

static riotboot_flashwrite_t _writer;
static heatshrink_decoder _decoder;
static size_t _decompressed_size = 0;
static uint8_t _buffer[2048];
static size_t _buffer_size = 0;

static inline void _set_slot(knx_device_t *device, uint8_t slot)
{
    device->objects[PROPERTY_OBJECT_USER_OTA].properties[2].content.value[0] = slot;
}

static inline void _set_checksum(knx_device_t *device, uint32_t checksum)
{
    be_uint32_t tmp = byteorder_htonl(checksum);

    memcpy(device->objects[PROPERTY_OBJECT_USER_OTA].properties[3].content.value, &tmp, sizeof(be_uint32_t));
}

static inline void _set_state(knx_device_t *device, ota_state_t state)
{
    device->objects[PROPERTY_OBJECT_USER_OTA].properties[5].content.value[0] = (uint8_t) state;
}

static void _write_data(knx_device_t *device, uint8_t *buffer, size_t size, size_t total_size)
{
    int res;

    /* the first block contains the RIOTBOOT_MAGIC, which must be skipped,
       since it will be written by the riotboot_flashwrite_finish method */
    size_t skip = 0;

    if (_writer.offset == 4) {
        size = size - RIOTBOOT_FLASHWRITE_SKIPLEN;
        skip = RIOTBOOT_FLASHWRITE_SKIPLEN;
    }

    bool more = _writer.offset + size != total_size;

    DEBUG("[ota] _write_data: writing data offset=%u size=%u total_size=%u more=%d\n",
          _writer.offset, size, total_size, more);

    res = riotboot_flashwrite_putbytes(&_writer, &(buffer[skip]), size, more);

    if (res != 0) {
        _set_state(device, OTA_STATE_ERROR);
        DEBUG("[ota] _write_data: write failed\n");

        return;
    }

    if (!more) {
        res = riotboot_flashwrite_flush(&_writer);

        if (res != 0) {
            _set_state(device, OTA_STATE_ERROR);
            DEBUG("[ota] _write_data: flushing failed (res=%d)\n", res);

            return;
        }

        res = riotboot_flashwrite_finish(&_writer);

        if (res != 0) {
            _set_state(device, OTA_STATE_ERROR);
            DEBUG("[ota] _write_data: finishing failed (res=%d)\n", res);

            return;
        }

        const riotboot_hdr_t *hdr = riotboot_slot_get_hdr(riotboot_slot_other());

        res = riotboot_hdr_validate(hdr);

        if (res != 0) {
            _set_state(device, OTA_STATE_ERROR);
            DEBUG("[ota] _write_data: header not valid\n");

            return;
        }

        _set_state(device, OTA_STATE_FINISHED);
        DEBUG("[ota] _write_data: finished\n");
    }
}

void ota_write(knx_device_t *device, ota_event_t *ota_event)
{
    uint32_t offset = byteorder_ntohl(ota_event->offset);
    uint32_t size = byteorder_ntohl(ota_event->size);
    uint32_t total_size = byteorder_ntohl(ota_event->total_size);
    uint8_t expected_checksum = ota_event->checksum;
    uint8_t flags = ota_event->flags;

    bool compressed = flags & OTA_FLAG_COMPRESSED;

    DEBUG("[ota] ota_write: OTA offset=%" PRIu32 " size=%" PRIu32 " "
          "total_size=%" PRIu32 " checksum=%02x flags=%02x\n",
          offset,
          size,
          total_size,
          expected_checksum,
          flags);

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

    if (offset == 0) {
        _set_state(device, OTA_STATE_BUSY);
        DEBUG("[ota] ota_write: initializing\n");

        if (riotboot_flashwrite_init(&_writer, riotboot_slot_other()) != 0) {
            _set_state(device, OTA_STATE_ERROR);
            DEBUG("[ota] ota_write: initialization failed\n");

            return;
        }

        heatshrink_decoder_reset(&_decoder);
    }

    if (compressed) {
        size_t skip = 0;
        size_t actual_sink = 0;
        size_t total_sink = 0;
        size_t actual_poll;
        size_t total_poll;

        /* skip first four bytes when this is the first block, as it contains
           the decompressed size */
        if (offset == 0) {
            memcpy(&_decompressed_size, ota, sizeof(_decompressed_size));

            if (_decompressed_size < total_size) {
                DEBUG("[ota] ota_write: decompressed size smaller than total size\n");
                return;
            }

            DEBUG("[ota] ota_write: decompressed size=%u\n", _decompressed_size);

            skip = 4;
            size = size - skip;
        }

        while (1) {
            int res = heatshrink_decoder_sink(&_decoder, &ota[total_sink + skip], size - total_sink, &actual_sink);


            if (res != HSDR_SINK_OK) {
                DEBUG("[ota] ota_write: heatshrink decoder sink failed (res=%d)\n", res);
                return;
            }

            DEBUG("[ota] ota_write: heatshrink decoder sink res=%d actual_sink=%u\n", res, actual_sink);

            if (actual_sink == 0) {
                break;
            }

            total_sink += actual_sink;

            while (1) {
                if (_buffer_size == sizeof(_buffer)) {
                    DEBUG("[ota] ota_write: flushing buffer\n");

                    _write_data(device, _buffer, _buffer_size, _decompressed_size);
                    _buffer_size = 0;
                }

                DEBUG("[ota] ota_write: buffer_size=%u\n", _buffer_size);

                res = heatshrink_decoder_poll(&_decoder, &_buffer[_buffer_size], sizeof(_buffer) - _buffer_size, &actual_poll);

                DEBUG("[ota] ota_write: heatshrink decoder poll res=%d actual_poll=%u\n", res, actual_poll);

                if (res == HSDR_POLL_MORE) {
                    total_poll += actual_poll;
                    _buffer_size += actual_poll;
                    continue;
                }
                else if (res == HSDR_POLL_EMPTY) {
                    total_poll += actual_poll;
                    _buffer_size += actual_poll;
                    break;
                }
                else {
                    DEBUG("[ota] ota_write: heatshrink decoder poll failed (res=%d)\n", res);
                    return;
                }
            }
        }

        bool more = (offset + size) != total_size;

        if (!more) {
            int res = heatshrink_decoder_finish(&_decoder);

            if (res != HSDR_FINISH_DONE) {
                DEBUG("[ota] ota_write: heatshrink decoder finish failed (res=%d)\n", res);
                return;
            }

            DEBUG("[ota] ota_write: heatshrink decoder finished, total_sink=%u total_poll=%u\n", total_sink, total_poll);

            if (_buffer_size > 0) {
                DEBUG("[ota] ota_write: flushing remaining buffer\n");

                _write_data(device, _buffer, _buffer_size, _decompressed_size);
                _buffer_size = 0;
            }
        }
    }
    else {
        _write_data(device, ota, size, total_size);
    }
}

void ota_init(knx_device_t *device)
{
    int slot = riotboot_slot_current();
    const riotboot_hdr_t *header = riotboot_slot_get_hdr(slot);

    /* load the OTA settings */
    _set_slot(device, slot);
    _set_checksum(device, header->chksum);
    _set_state(device, OTA_STATE_READY);
}
