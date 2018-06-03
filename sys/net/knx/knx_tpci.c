/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_knx_tpci
 * @{
 *
 * @file
 * @brief       KNX TPCI services implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdio.h>

#include "net/knx/tpci.h"

#include "assert.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

knx_tpci_t knx_tpci_get(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            return ((telegram[6] & 0xc0) >> 6);
        case KNX_TELEGRAM_TYPE_EXTENDED:
            return ((telegram[7] & 0xc0) >> 6);
        default:
            return 0;
    }
}

void knx_tpci_set(knx_telegram_t *telegram, knx_tpci_t tpci)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            telegram[6] = (telegram[6] & 0x3f) | ((tpci & 0x03) << 6);
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            telegram[7] = (telegram[7] & 0x3f) | ((tpci & 0x03) << 6);
            break;
        default:
            return;
    }
}

knx_tpci_ucd_t knx_tpci_ucd_get(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            return (telegram[6] & 0x03);
        case KNX_TELEGRAM_TYPE_EXTENDED:
            return (telegram[7] & 0x03);
        default:
            return 0;
    }
}

void knx_tpci_ucd_set(knx_telegram_t *telegram, knx_tpci_ucd_t ucd)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            telegram[6] = (telegram[6] & 0xfc) | (ucd & 0x03);
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            telegram[7] = (telegram[7] & 0xfc) | (ucd & 0x03);
            break;
        default:
            return;
    }
}

knx_tpci_ncd_t knx_tpci_ncd_get(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            return (telegram[6] & 0x03);
        case KNX_TELEGRAM_TYPE_EXTENDED:
            return (telegram[7] & 0x03);
        default:
            return 0;
    }
}

void knx_tpci_ncd_set(knx_telegram_t *telegram, knx_tpci_ncd_t ncd)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            telegram[6] = (telegram[6] & 0xfc) | (ncd & 0x03);
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            telegram[7] = (telegram[7] & 0xfc) | (ncd & 0x03);
            break;
        default:
            return;
    }
}

uint8_t knx_tpci_get_seq_number(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            return ((telegram[6] & 0x3c) >> 2);
        case KNX_TELEGRAM_TYPE_EXTENDED:
            return ((telegram[7] & 0x3c) >> 2);
        default:
            return 0;
    }
}

void knx_tpci_set_seq_number(knx_telegram_t *telegram, uint8_t seq)
{
    assert(telegram != NULL);

    if (seq > 15) {
        return;
    }

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            telegram[6] = (telegram[6] & 0xc3) | ((seq & 0x0f) << 2);
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            telegram[7] = (telegram[7] & 0xc3) | ((seq & 0x0f) << 2);
            break;
        default:
            return;
    }
}
