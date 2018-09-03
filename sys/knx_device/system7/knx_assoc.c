/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_knx_device_system7
 * @{
 *
 * @file
 * @brief       KNX device association implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <string.h>
#include <stdlib.h>

#include "net/knx.h"
#include "knx_device/system7/assoc.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static int _bin_search(const knx_assoc_t *target, const knx_addr_group_t *group_addr)
{
    uint8_t l = 0;
    uint8_t r = target->count;

    while (l < r) {
        uint8_t m = (l + r) / 2;

        if (knx_addr_compare(&(target->mappings[m].group_addr), group_addr) < 0) {
            l = m + 1;
        }
        else {
            r = m;
        }
    }

    return l;
}

void knx_assoc_update(knx_assoc_t *target, knx_com_object_t *objects, knx_table_assoc_t *assoc_table, const knx_table_addr_t *addr_table, uint8_t limit)
{
    assert(target != NULL);

    /* reset the number of associations prior to updating them */
    target->count = 0;

    /* iterate over every association, and insert them into target, ordered by
       address */
    limit = limit < assoc_table->count ? limit : assoc_table->count;

    for (int i = 0; i < limit; i++) {
        uint8_t address_index = assoc_table->associations[i].addr_index;
        uint8_t com_object_index = assoc_table->associations[i].com_object_index;

        /* store association (note that group address in memory is stored in
           network order, so it is converted) */
        knx_assoc_mapping_t new = {
            .group_addr = { .u16 = { addr_table->addrs[address_index - 1].u16 } },
            .com_object = &(objects[com_object_index])
        };

        /* add to the list by shifting all associations that are ordered after
           this address */
        uint8_t index = _bin_search(target, &(new.group_addr));

#if ENABLE_DEBUG
        char addr_str[KNX_ADDR_MAX_STR_LEN];

        DEBUG("[knx_assoc] knx_assoc_update: insert index for %s is %d, count is %d\n",
              knx_addr_group_to_str(addr_str, &(new.group_addr), KNX_ADDR_MAX_STR_LEN),
              index, target->count);
#endif

        memmove(&(target->mappings[index + 1]), &(target->mappings[index]), sizeof(knx_assoc_mapping_t) * (target->count - index));
        memcpy(&(target->mappings[index]), &new, sizeof(knx_assoc_mapping_t));

        /* advance to next assocation */
        target->count++;
    }

    DEBUG("[knx_assoc] knx_assoc_update: %d associations updated\n", target->count);
}

int knx_assoc_find_by_group_address(const knx_assoc_t *target, const knx_addr_group_t *group_addr)
{
    assert(target != NULL);
    assert(group_addr != NULL);

    /* the index will refer to the mapping equal to or just after group_addr */
    uint8_t index = _bin_search(target, group_addr);

    /* check for exact match */
    if (!knx_addr_equal(&(target->mappings[index].group_addr), group_addr)) {
        return -1;
    }

    return index;
}

knx_assoc_mapping_t *knx_assoc_iter_by_group_address(const knx_assoc_t *target, knx_assoc_mapping_t *prev, const knx_addr_group_t *group_addr)
{
    assert(target != NULL);
    assert(group_addr != NULL);

    /* start case, whenever prev is NULL */
    if (prev == NULL) {
        int index = knx_assoc_find_by_group_address(target, group_addr);

        /* return here if not found */
        if (index == -1) {
            return NULL;
        }

        return &(target->mappings[index]);
    }

    /* next case, whenever prev is the result of the previous invocation, and
       since all mappings are sorted by group address, we can return the next
       element until the address changed */
    assert(prev >= target->mappings);
    assert(prev < &(target->mappings[target->count]));

    knx_assoc_mapping_t *next = ++prev;

    /* out of bounds check */
    if (next >= &(target->mappings[target->count])) {
        return NULL;
    }

    /* next must have same address */
    if (!knx_addr_equal(&(next->group_addr), group_addr)) {
        return NULL;
    }

    return next;
}

int knx_assoc_find_by_com_object(const knx_assoc_t *target, const knx_com_object_t *com_object)
{
    assert(target != NULL);
    assert(com_object != NULL);

    /* use sequential search to find the first com object that matches */
    for (unsigned i = 0; i < target->count; i++) {
        if (target->mappings[i].com_object == com_object) {
            return i;
        }
    }

    return -1;
}

knx_assoc_mapping_t *knx_assoc_iter_by_com_object(const knx_assoc_t *target, knx_assoc_mapping_t *prev, const knx_com_object_t *com_object)
{
    assert(target != NULL);
    assert(com_object != NULL);

    /* start case, whenever prev is NULL */
    if (prev == NULL) {
        int index = knx_assoc_find_by_com_object(target, com_object);

        /* return here if not found */
        if (index == -1) {
            return NULL;
        }

        return &(target->mappings[index]);
    }

    /* next case, whenever prev is the result of the previous invocation, but
       because the mappings are not sorted by com object, we have to use
       sequential search again */
    assert(prev >= target->mappings);
    assert(prev < &(target->mappings[target->count]));

    while (1) {
        knx_assoc_mapping_t *next = ++prev;

        /* out of bounds check */
        if (next >= &(target->mappings[target->count])) {
            return NULL;
        }

        if (next->com_object == com_object) {
            return next;
        }
    }

    return NULL;
}
