/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_knx_device_system7
 *
 * @{
 *
 * @file
 * @brief       KNX device memory implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <string.h>

#include "knx_device/system7/memory.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

knx_memory_segment_t *knx_memory_find(knx_memory_segment_t *segments, uint16_t addr, uint16_t size)
{
    assert(segments != NULL);

    for (unsigned i = 0; segments[i].start != KNX_MEMORY_ADDRESS_MAX; i++) {
        if (segments[i].start <= addr && segments[i].size >= size && (addr + size) <= (segments[i].start + segments[i].size)) {
            return &(segments[i]);
        }
    }

    /* suitable memory segment not found */
    return NULL;
}

ssize_t knx_memory_read(const knx_memory_segment_t *segment, uint16_t addr, uint16_t size, uint8_t *buf)
{
    assert(segment != NULL);
    assert(addr >= segment->start);
    assert(size <= segment->size);

    /* ensure memory segment is readable */
    if ((segment->flags & KNX_MEMORY_FLAG_READABLE) == 0) {
        return -1;
    }

    /* ensure a buffer */
    if (buf == NULL) {
        return -2;
    }

    /* start reading */
    DEBUG("[knx_memory] knx_memory_read: reading address=%04x size=%d\n", addr, size);

    memcpy(buf, &((uint8_t *)segment->ptr)[addr - segment->start], size);

    return size;
}

ssize_t knx_memory_write(knx_memory_segment_t *segment, uint16_t addr, uint16_t size, const uint8_t *buf)
{
    assert(segment != NULL);
    assert(addr >= segment->start);
    assert(size <= segment->size);

    /* ensure memory segment is writeable */
    if ((segment->flags & KNX_MEMORY_FLAG_WRITABLE) == 0) {
        return -1;
    }

    /* ensure a buffer */
    if (buf == NULL) {
        return -2;
    }

    /* start writing */
    DEBUG("[knx_memory] knx_memory_writing: reading address=%04x size=%d\n", addr, size);

    memcpy(&((uint8_t *)segment->ptr)[addr - segment->start], buf, size);

    return size;
}
