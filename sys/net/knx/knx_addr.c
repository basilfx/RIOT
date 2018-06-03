/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_knx_addr
 * @{
 *
 * @file
 * @brief       KNX address implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <string.h>
#include <stdio.h>

#include "net/knx/addr.h"

const knx_addr_t knx_addr_broadcast = KNX_ADDR_BROADCAST;
const knx_addr_t knx_addr_undefined = KNX_ADDR_UNDEFINED;

inline bool knx_addr_equal(const knx_addr_t *a, const knx_addr_t *b)
{
    return (a->u16.u16 == b->u16.u16);
}

int knx_addr_compare(const knx_addr_t *a, const knx_addr_t *b)
{
    return (byteorder_ntohs(a->u16) - byteorder_ntohs(b->u16));
}

knx_addr_physical_t *knx_addr_physical(knx_addr_physical_t *result, uint8_t area, uint8_t line, uint8_t device)
{
    if (result == NULL) {
        return NULL;
    }

    if (area >= 16 || line >= 16) {
        return NULL;
    }

    result->u16 = byteorder_htons(((area & 0xf) << 12) + ((line & 0xf) << 8) + device);

    return result;
}

knx_addr_group_t *knx_addr_group(knx_addr_group_t *result, uint8_t main, uint8_t mid, uint8_t sub)
{
    if (result == NULL) {
        return NULL;
    }

    if (main >= 32 || mid >= 8) {
        return NULL;
    }

    result->u16 = byteorder_htons(((main & 0x1f) << 11) + ((mid & 0x7) << 8) + sub);

    return result;
}

knx_addr_group_t *knx_addr_group2(knx_addr_group_t *result, uint8_t main, uint16_t sub)
{
    if (result == NULL) {
        return NULL;
    }

    if (main >= 32 || sub >= 2048) {
        return NULL;
    }

    result->u16 = byteorder_htons(((main & 0x1f) << 11) + sub);

    return result;
}

knx_addr_physical_t *knx_addr_physical_from_str(knx_addr_physical_t *result, const char *addr)
{
    int area, line, device;

    if (addr == NULL) {
        return NULL;
    }

    if (sscanf(addr, "%d.%d.%d", &area, &line, &device) != 3) {
        return NULL;
    }

    return knx_addr_physical(result, area, line, device);
}

knx_addr_group_t *knx_addr_group_from_str(knx_addr_group_t *result, const char *addr)
{
    int main_, mid, sub;

    if (addr == NULL) {
        return NULL;
    }

    if (sscanf(addr, "%d/%d/%d", &main_, &mid, &sub) != 3) {
        return NULL;
    }

    return knx_addr_group(result, main_, mid, sub);
}

knx_addr_group_t *knx_addr_group2_from_str(knx_addr_group_t *result, const char *addr)
{
    int main_, sub;

    if (addr == NULL) {
        return NULL;
    }

    if (sscanf(addr, "%d/%d", &main_, &sub) != 2) {
        return NULL;
    }

    return knx_addr_group2(result, main_, sub);
}

char *knx_addr_physical_to_str(char *result, const knx_addr_physical_t *addr, uint8_t len)
{
    if (result == NULL) {
        return NULL;
    }

    if (addr == NULL) {
        return NULL;
    }

    if (len < KNX_ADDR_MAX_STR_LEN) {
        return NULL;
    }

    uint16_t tmp = byteorder_ntohs(addr->u16);

    uint8_t area = (tmp >> 12) & 0xf;
    uint8_t line = (tmp >> 8) & 0xf;
    uint8_t device = (tmp & 0xff);

    snprintf(result, len, "%d.%d.%d", area, line, device);

    return result;
}

char *knx_addr_group_to_str(char *result, const knx_addr_group_t *addr, uint8_t len)
{
    if (result == NULL) {
        return NULL;
    }

    if (addr == NULL) {
        return NULL;
    }

    if (len < KNX_ADDR_MAX_STR_LEN) {
        return NULL;
    }

    uint16_t tmp = byteorder_ntohs(addr->u16);

    uint8_t main_ = (tmp >> 12) & 0xf;
    uint8_t mid = (tmp >> 8) & 0xf;
    uint8_t sub = (tmp & 0xff);

    snprintf(result, len, "%d/%d/%d", main_, mid, sub);

    return result;
}

char *knx_addr_group2_to_str(char *result, const knx_addr_group_t *addr, uint8_t len)
{
    if (result == NULL) {
        return NULL;
    }

    if (addr == NULL) {
        return NULL;
    }

    if (len < KNX_ADDR_MAX_STR_LEN) {
        return NULL;
    }

    uint16_t tmp = byteorder_ntohs(addr->u16);

    uint8_t main_ = (tmp >> 11) & 0x1f;
    uint16_t sub = (tmp & 0x7ff);

    snprintf(result, len, "%d/%d", main_, sub);

    return result;
}
