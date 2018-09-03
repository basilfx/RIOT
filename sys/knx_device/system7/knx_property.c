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
 * @brief       KNX device property implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <string.h>

#include "knx_device/system7/property.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Mapping from type to (maximum) size in bytes
 */
static uint8_t sizes[] = {
    [KNX_PROPERTY_TYPE_CONTROL] = 1,
    [KNX_PROPERTY_TYPE_CHAR] = 1,
    [KNX_PROPERTY_TYPE_UNSIGNED_CHAR] = 1,
    [KNX_PROPERTY_TYPE_INT] = 2,
    [KNX_PROPERTY_TYPE_UNSIGNED_INT] = 2,
    [KNX_PROPERTY_TYPE_KNX_FLOAT] = 2,
    [KNX_PROPERTY_TYPE_DATE] = 3,
    [KNX_PROPERTY_TYPE_TIME] = 3,
    [KNX_PROPERTY_TYPE_LONG] = 4,
    [KNX_PROPERTY_TYPE_UNSIGNED_LONG] = 4,
    [KNX_PROPERTY_TYPE_FLOAT] = 4,
    [KNX_PROPERTY_TYPE_DOUBLE] = 8,
    [KNX_PROPERTY_TYPE_CHAR_BLOCK] = 10,
    [KNX_PROPERTY_TYPE_POLL_GROUP_SETTINGS] = 3,
    [KNX_PROPERTY_TYPE_SHORT_CHAR_BLOCK] = 5,
    [KNX_PROPERTY_TYPE_DATE_TIME] = 8,
    [KNX_PROPERTY_TYPE_VARIABLE_LENGTH] = 255,
    [KNX_PROPERTY_TYPE_GENERIC1] = 1,
    [KNX_PROPERTY_TYPE_GENERIC2] = 2,
    [KNX_PROPERTY_TYPE_GENERIC3] = 3,
    [KNX_PROPERTY_TYPE_GENERIC4] = 4,
    [KNX_PROPERTY_TYPE_GENERIC5] = 5,
    [KNX_PROPERTY_TYPE_GENERIC6] = 6,
    [KNX_PROPERTY_TYPE_GENERIC7] = 7,
    [KNX_PROPERTY_TYPE_GENERIC8] = 8,
    [KNX_PROPERTY_TYPE_GENERIC9] = 9,
    [KNX_PROPERTY_TYPE_GENERIC10] = 10,
    [KNX_PROPERTY_TYPE_GENERIC11] = 11,
    [KNX_PROPERTY_TYPE_GENERIC12] = 12,
    [KNX_PROPERTY_TYPE_GENERIC13] = 13,
    [KNX_PROPERTY_TYPE_GENERIC14] = 14,
    [KNX_PROPERTY_TYPE_GENERIC15] = 15,
    [KNX_PROPERTY_TYPE_GENERIC16] = 16,
    [KNX_PROPERTY_TYPE_GENERIC17] = 17,
    [KNX_PROPERTY_TYPE_GENERIC18] = 18,
    [KNX_PROPERTY_TYPE_GENERIC19] = 19,
    [KNX_PROPERTY_TYPE_GENERIC20] = 20,
    [KNX_PROPERTY_TYPE_UTF8] = 255,
    [KNX_PROPERTY_TYPE_VERSION] = 2,
    [KNX_PROPERTY_TYPE_ALARM_INFO] = 6,
    [KNX_PROPERTY_TYPE_BINARY_INFORMATION] = 1,
    [KNX_PROPERTY_TYPE_BITSET8] = 1,
    [KNX_PROPERTY_TYPE_BITSET16] = 2,
    [KNX_PROPERTY_TYPE_ENUM8] = 1,
    [KNX_PROPERTY_TYPE_SCALING] = 1,
    [KNX_PROPERTY_TYPE_NE_VL] = 255,
    [KNX_PROPERTY_TYPE_NE_FL] = 255,
    [KNX_PROPERTY_TYPE_FUNCTION] = 255,
    [KNX_PROPERTY_TYPE_ESCAPE] = 255,
};

knx_property_t *knx_property_find_by_id(const knx_property_object_t *objects, uint8_t object, uint8_t id)
{
    for (unsigned i = 0; objects[i].properties != NULL; i++) {
        if (i == object) {
            for (unsigned j = 0; j < objects[i].count; j++) {
                if (objects[i].properties[j].id == id) {
                    return &(objects[i].properties[j]);
                }
            }
        }
    }

    return NULL;
}

knx_property_t *knx_property_find_by_index(const knx_property_object_t *objects, uint8_t object, uint8_t index)
{
    for (unsigned i = 0; objects[i].properties != NULL; i++) {
        if (i == object) {
            /* index cannot exceed count */
            if (index >= objects[i].count) {
                return NULL;
            }

            return &(objects[i].properties[index]);
        }
    }

    return NULL;
}

size_t knx_property_size(const knx_property_t *property)
{
    assert(property != NULL);
    assert(property->type < sizeof(sizes));

    return sizes[property->type];
}

size_t knx_property_elements(const knx_property_t *property)
{
    assert(property != NULL);

    /* if not an array, then there is only one element */
    if (property->flags & KNX_PROPERTY_FLAG_ARRAY) {
        size_t property_size = knx_property_size(property);

        /* sanity check */
        assert(property->size % property_size == 0);

        return property->size / property_size;
    }
    else {
        return 1;
    }
}

ssize_t knx_property_read(const knx_property_t *property, uint8_t count, uint8_t start, uint8_t *buf, size_t len)
{
    assert(property != NULL);

    if (buf == NULL) {
        return -1;
    }

    /* ensure buffer is large enough */
    uint8_t size = knx_property_size(property);

    if (len < (count * size)) {
        return -2;
    }

    /* get pointer to value */
    const void *ptr = NULL;

    if (property->flags & KNX_PROPERTY_FLAG_POINTER) {
        ptr = property->content.ptr;

        /* sanity check */
        assert(ptr != NULL);
    }
    else {
        ptr = property->content.value;

        /* sanity check */
        assert(size <= KNX_PROPERTY_VALUE_SIZE);
    }

    /* copy value from pointer */
    DEBUG("[knx_property] knx_property_read: id=%d count=%d start=%d\n", property->id, count, start);

    memcpy(buf, ((const uint8_t *)ptr) + ((start - 1) * size), count * size);

    return count * size;
}

ssize_t knx_property_write(knx_property_t *property, uint8_t count, uint8_t start, const uint8_t *buf, size_t len)
{
    assert(property != NULL);

    if (buf == NULL) {
        return -1;
    }

    /* ensure buffer is large enough */
    uint8_t size = knx_property_size(property);

    if (len < (count * size)) {
        return -2;
    }

    /* ensure property is writeable */
    if ((property->flags & KNX_PROPERTY_FLAG_WRITABLE) == 0) {
        return -3;
    }

    /* control properties must be handled via events */
    if (property->type == KNX_PROPERTY_TYPE_CONTROL) {
        return 0;
    }

    /* get pointer to value */
    void *ptr = NULL;

    if (property->flags & KNX_PROPERTY_FLAG_POINTER) {
        ptr = property->content.ptr;

        /* sanity check */
        assert(ptr != NULL);
    }
    else {
        ptr = &(property->content.value);

        /* sanity check */
        assert(size <= sizeof(property->content.value));
    }

    /* copy value to pointer */
    DEBUG("[knx_property] knx_property_write: id=%d count=%d start=%d\n", property->id, count, start);

    memcpy(((uint8_t *)ptr) + ((start - 1) * size), buf, count * size);

    return count * size;
}
