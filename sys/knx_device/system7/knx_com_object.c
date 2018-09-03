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
 * @brief       KNX device communication object association interface
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <string.h>
#include <stdlib.h>

#include "knx_device/system7/com_object.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Mapping from communication object to size in bytes
 */
static const uint8_t sizes[] = {
    [KNX_COM_OBJECT_TYPE_BIT1] = 0,
    [KNX_COM_OBJECT_TYPE_BIT2] = 0,
    [KNX_COM_OBJECT_TYPE_BIT3] = 0,
    [KNX_COM_OBJECT_TYPE_BIT4] = 0,
    [KNX_COM_OBJECT_TYPE_BIT5] = 0,
    [KNX_COM_OBJECT_TYPE_BIT6] = 0,
    [KNX_COM_OBJECT_TYPE_BIT7] = 1,
    [KNX_COM_OBJECT_TYPE_BYTE1] = 1,
    [KNX_COM_OBJECT_TYPE_BYTE2] = 2,
    [KNX_COM_OBJECT_TYPE_BYTE3] = 3,
    [KNX_COM_OBJECT_TYPE_BYTE4] = 4,
    [KNX_COM_OBJECT_TYPE_DATA6] = 6,
    [KNX_COM_OBJECT_TYPE_DATA8] = 8,
    [KNX_COM_OBJECT_TYPE_DATA10] = 10,
    [KNX_COM_OBJECT_TYPE_MAXDATA] = 14,
    [KNX_COM_OBJECT_TYPE_VARDATA] = 15
};

void knx_com_object_update(knx_com_object_t *objects, const knx_table_com_objects_t *obj_table, int count)
{
    assert(objects != NULL);

    for (int i = 0; i < count; i++) {
        objects[i].access = obj_table->objects[i].flags & 0xfc;
        objects[i].priority = obj_table->objects[i].flags & 0x03;
        objects[i].type = obj_table->objects[i].type;
    }
}

size_t knx_com_object_size(const knx_com_object_t *com_object)
{
    assert(com_object != NULL);
    assert(com_object->type < sizeof(sizes));

    return sizes[com_object->type];
}

ssize_t knx_com_object_read(const knx_com_object_t *com_object, void *buf, size_t len)
{
    assert(com_object != NULL);

    if (buf == NULL) {
        return -1;
    }

    size_t size = knx_com_object_size(com_object);

    /* bit-sized values will take up a byte */
    if (size < 1) {
        size = 1;
    }

    if (len < size) {
        return -2;
    }

    /* get pointer to value */
    const void *ptr = NULL;

    if (com_object->flags & KNX_COM_OBJECT_FLAG_POINTER) {
        ptr = com_object->content.ptr;

        /* sanity check */
        assert(ptr != NULL);
    }
    else {
        ptr = com_object->content.value;

        /* sanity check */
        assert(size <= KNX_COM_OBJECT_VALUE_SIZE);
    }

    /* read into buf */
    DEBUG("[knx_com_object] knx_com_object_read: reading %d bytes\n", (unsigned)size);

    memcpy(buf, ptr, size);

    return size;
}

ssize_t knx_com_object_write(knx_com_object_t *com_object, const void *buf, size_t len)
{
    assert(com_object != NULL);

    if (buf == NULL) {
        return -1;
    }

    size_t size = knx_com_object_size(com_object);

    /* bit-sized values will take up a byte */
    if (size < 1) {
        size = 1;
    }

    if (len < size) {
        return -2;
    }

    /* get pointer to value */
    void *ptr = NULL;

    if (com_object->flags & KNX_COM_OBJECT_FLAG_POINTER) {
        ptr = com_object->content.ptr;

        /* sanity check */
        assert(ptr != NULL);
    }
    else {
        ptr = &(com_object->content.value);

        /* sanity check */
        assert(size <= sizeof(com_object->content.value));
    }

    /* write from buf */
    DEBUG("[knx_com_object] knx_com_object_write: writing %d bytes\n", (unsigned)size);

    memcpy(ptr, buf, size);

    return size;
}
