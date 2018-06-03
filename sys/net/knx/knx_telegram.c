/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_knx_telegram
 * @{
 *
 * @file
 * @brief       KNX telegram implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdio.h>

#include "net/knx/telegram.h"
#include "net/knx/addr.h"

#include "assert.h"

bool knx_telegram_is(const uint8_t *telegram, size_t len)
{
    if (telegram == NULL) {
        return false;
    }

    /* assert general length */
    if (len < KNX_TELEGRAM_MIN_LEN || len > KNX_TELEGRAM_MAX_LEN) {
        return false;
    }

    /* assert minimal length per type */
    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            return (len >= KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN) &&
                   (len <= KNX_TELEGRAM_TYPE_STANDARD_MAX_LEN);
        case KNX_TELEGRAM_TYPE_POLL:
            return (len >= KNX_TELEGRAM_TYPE_POLL_MIN_LEN) &&
                   (len <= KNX_TELEGRAM_TYPE_POLL_MAX_LEN);
        case KNX_TELEGRAM_TYPE_EXTENDED:
            return (len >= KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN) &&
                   (len <= KNX_TELEGRAM_TYPE_EXTENDED_MAX_LEN);
        default:
            return false;
    }
}

uint8_t knx_telegram_checksum(const knx_telegram_t *telegram, size_t len)
{
    assert(telegram != NULL);

    uint8_t sum = 0xff;

    for (unsigned i = 0; i < len; ++i) {
        sum ^= telegram[i];
    }

    return sum;
}

inline uint8_t knx_telegram_get_checksum(const knx_telegram_t *telegram, size_t len)
{
    assert(telegram != NULL);

    return telegram[len - 1];
}

inline void knx_telegram_set_checksum(knx_telegram_t *telegram, size_t len, uint8_t checksum)
{
    assert(telegram != NULL);

    telegram[len - 1] = checksum;
}

inline void knx_telegram_update_checksum(knx_telegram_t *telegram, size_t len)
{
    assert(telegram != NULL);

    knx_telegram_set_checksum(telegram, len, knx_telegram_checksum(telegram, len - 1));
}

inline bool knx_telegram_is_checksum_valid(const knx_telegram_t *telegram, size_t len)
{
    assert(telegram != NULL);

    uint8_t actual = knx_telegram_get_checksum(telegram, len);
    uint8_t expected = knx_telegram_checksum(telegram, len - 1);

    return expected == actual;
}

knx_telegram_type_t knx_telegram_get_type(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    if (telegram[0] == KNX_TELEGRAM_CONTROL_POLL_REQUEST) {
        return KNX_TELEGRAM_TYPE_POLL;
    }
    else if ((telegram[0] & KNX_TELEGRAM_CONTROL_DATA_REQUEST) == KNX_TELEGRAM_CONTROL_DATA_REQUEST) {
        return KNX_TELEGRAM_TYPE_STANDARD;
    }
    else if ((telegram[0] & KNX_TELEGRAM_CONTROL_EDATA_REQUEST) == KNX_TELEGRAM_CONTROL_EDATA_REQUEST) {
        return KNX_TELEGRAM_TYPE_EXTENDED;
    }

    return KNX_TELEGRAM_TYPE_UNKNOWN;
}

void knx_telegram_set_type(knx_telegram_t *telegram, knx_telegram_type_t type)
{
    assert(telegram != NULL);

    switch (type) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            telegram[0] = (telegram[0] & 0x2f) | 0x90;
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            telegram[0] = (telegram[0] & 0x2f) | 0x10;
            break;
        case KNX_TELEGRAM_TYPE_POLL:
            telegram[0] = 0xf0;
            break;
        default:
            return;
    }
}

knx_addr_t *knx_telegram_get_src_addr(const knx_telegram_t *telegram, knx_addr_t *result)
{
    assert(telegram != NULL);

    if (result == NULL) {
        return NULL;
    }

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
        case KNX_TELEGRAM_TYPE_POLL:
            result->u16.u16 = *((uint16_t *)&(telegram[1]));
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            result->u16.u16 = *((uint16_t *)&(telegram[2]));
            break;
        default:
            return NULL;
    }

    return result;
}

void knx_telegram_set_src_addr(knx_telegram_t *telegram, const knx_addr_t *addr)
{
    assert(telegram != NULL);

    if (addr == NULL) {
        return;
    }

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
        case KNX_TELEGRAM_TYPE_POLL:
            *((uint16_t *)&(telegram[1])) = addr->u16.u16;
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            *((uint16_t *)&(telegram[2])) = addr->u16.u16;
            break;
        default:
            return;
    }
}

knx_addr_t *knx_telegram_get_dst_addr(const knx_telegram_t *telegram, knx_addr_t *result)
{
    assert(telegram != NULL);

    if (result == NULL) {
        return NULL;
    }

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
        case KNX_TELEGRAM_TYPE_POLL:
            result->u16.u16 = *((uint16_t *)&(telegram[3]));
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            result->u16.u16 = *((uint16_t *)&(telegram[4]));
            break;
        default:
            return NULL;
    }

    return result;
}

void knx_telegram_set_dst_addr(knx_telegram_t *telegram, const knx_addr_t *addr)
{
    assert(telegram != NULL);

    if (addr == NULL) {
        return;
    }

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
        case KNX_TELEGRAM_TYPE_POLL:
            *((uint16_t *)&(telegram[3])) = addr->u16.u16;
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            *((uint16_t *)&(telegram[4])) = addr->u16.u16;
            break;
        default:
            return;
    }
}

bool knx_telegram_is_group_addressed(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            return (bool)(telegram[5] & 0x80);
        case KNX_TELEGRAM_TYPE_EXTENDED:
            return (bool)(telegram[1] & 0x80);
        default:
            return false;
    }
}

void knx_telegram_set_group_addressed(knx_telegram_t *telegram, bool group_addressed)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            if (group_addressed) {
                telegram[5] |= 0x80;
            }
            else {
                telegram[5] &= ~0x80;
            }
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            if (group_addressed) {
                telegram[1] |= 0x80;
            }
            else {
                telegram[1] &= ~0x80;
            }
            break;
        default:
            return;
    }
}

uint8_t *knx_telegram_get_payload(knx_telegram_t *telegram, bool merged)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            if (merged) {
                return &(telegram[7]);
            }
            else {
                return &(telegram[8]);
            }
        case KNX_TELEGRAM_TYPE_EXTENDED:
            if (merged) {
                return &(telegram[8]);
            }
            else {
                return &(telegram[9]);
            }
        default:
            return NULL;
    }
}

size_t knx_telegram_get_payload_length(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            return (telegram[5] & 0x0f);
        case KNX_TELEGRAM_TYPE_EXTENDED:
            return telegram[6];
        default:
            return 0;
    }
}

void knx_telegram_set_payload_length(knx_telegram_t *telegram, size_t len)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            telegram[5] = (telegram[5] & 0xf0) | (len & 0x0f);
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            telegram[6] = (len & 0xff);
        default:
            return;
    }
}

uint8_t knx_telegram_get_routing_count(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            return ((telegram[5] >> 4) & 0x07);
        case KNX_TELEGRAM_TYPE_EXTENDED:
            return ((telegram[1] >> 4) & 0x07);
        default:
            return 0;
    }
}

void knx_telegram_set_routing_count(knx_telegram_t *telegram, uint8_t count)
{
    assert(telegram != NULL);

    if (count > 7) {
        return;
    }

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            telegram[5] = (telegram[5] & 0x8f) | ((count & 0x07) << 4);
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            telegram[1] = (telegram[1] & 0x8f) | ((count & 0x07) << 4);
            break;
        default:
            return;
    }
}

knx_telegram_priority_t knx_telegram_get_priority(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
        case KNX_TELEGRAM_TYPE_EXTENDED:
            return (knx_telegram_priority_t)((telegram[0] >> 2) & 0x03);
        default:
            return KNX_TELEGRAM_PRIORITY_LOW;
    }
}

void knx_telegram_set_priority(knx_telegram_t *telegram, knx_telegram_priority_t priority)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
        case KNX_TELEGRAM_TYPE_EXTENDED:
            telegram[0] = (telegram[0] & 0xf3) | ((((uint8_t)priority) & 0x03) << 2);
            break;
        default:
            return;
    }
}

bool knx_telegram_is_repeated(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
        case KNX_TELEGRAM_TYPE_EXTENDED:
            /* flag is inverted in the control field */
            return (bool)((telegram[0] & 0x20) == 0x00);
        default:
            return false;
    }
}

void knx_telegram_set_repeated(knx_telegram_t *telegram, bool repeated)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
        case KNX_TELEGRAM_TYPE_EXTENDED:
            /* flag is inverted in the control field */
            if (repeated) {
                telegram[0] &= ~0x20;
            }
            else {
                telegram[0] |= 0x20;
            }
            break;
        default:
            return;
    }
}

void knx_telegram_build(knx_telegram_t *telegram, knx_telegram_type_t type, const knx_addr_t *src, const knx_addr_t *dest, bool group_addressed)
{
    assert(telegram != NULL);

    knx_telegram_set_type(telegram, type);
    knx_telegram_set_src_addr(telegram, src);
    knx_telegram_set_dst_addr(telegram, dest);

    switch (type) {
        case KNX_TELEGRAM_TYPE_STANDARD:
        case KNX_TELEGRAM_TYPE_EXTENDED:
            knx_telegram_set_priority(telegram, KNX_TELEGRAM_PRIORITY_LOW);
            knx_telegram_set_repeated(telegram, false);
            knx_telegram_set_routing_count(telegram, 6);
            knx_telegram_set_group_addressed(telegram, group_addressed);
            knx_telegram_set_payload_length(telegram, 0);
            break;
        default:
            return;
    }
}
