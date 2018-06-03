/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_knx_apci
 * @{
 *
 * @file
 * @brief       KNX APCI services implementation
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdio.h>

#include "net/knx/apci.h"

#include "byteorder.h"
#include "assert.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

knx_apci_t knx_apci_get(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            return ((telegram[6] & 0x03) << 2) + ((telegram[7] & 0xc0) >> 6);
        case KNX_TELEGRAM_TYPE_EXTENDED:
            return ((telegram[7] & 0x03) << 2) + ((telegram[8] & 0xc0) >> 6);
        default:
            return 0;
    }
}

void knx_apci_set(knx_telegram_t *telegram, knx_apci_t apci)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            telegram[6] = (telegram[6] & 0xfc) | ((apci >> 2) & 0xff);
            telegram[7] = (telegram[7] & 0x3f) | ((apci & 0x03) << 6);
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            telegram[7] = (telegram[7] & 0xfc) | ((apci >> 2) & 0xff);
            telegram[8] = (telegram[8] & 0x3f) | ((apci & 0x03) << 6);
            break;
        default:
            return;
    }
}

knx_apci_extended_t knx_apci_extended_get(const knx_telegram_t *telegram)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            return ((telegram[6] & 0x03) << 8) | telegram[7];
        case KNX_TELEGRAM_TYPE_EXTENDED:
            return ((telegram[7] & 0x03) << 8) | telegram[8];
        default:
            return 0;
    }
}

void knx_apci_extended_set(knx_telegram_t *telegram, knx_apci_extended_t apci)
{
    assert(telegram != NULL);

    switch (knx_telegram_get_type(telegram)) {
        case KNX_TELEGRAM_TYPE_STANDARD:
            telegram[6] = (telegram[6] & 0xfc) | ((apci >> 8) & 0xff);
            telegram[7] = (apci & 0xff);
            break;
        case KNX_TELEGRAM_TYPE_EXTENDED:
            telegram[7] = (telegram[7] & 0xfc) | ((apci >> 8) & 0xff);
            telegram[8] = (apci & 0xff);
            break;
        default:
            return;
    }
}
