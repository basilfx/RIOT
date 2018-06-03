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

/**
 * @brief   Mapping of telegram type to string representation (for dump)
 */
static char *telegram_type_mapping[] = {
    [KNX_TELEGRAM_TYPE_STANDARD] = "standard",
    [KNX_TELEGRAM_TYPE_EXTENDED] = "extended",
    [KNX_TELEGRAM_TYPE_POLL] = "poll",
    [KNX_TELEGRAM_TYPE_UNKNOWN] = "unknown"
};

void knx_telegram_print(const knx_telegram_t *telegram, size_t len)
{
    knx_addr_t addr;
    char addr_str[KNX_ADDR_MAX_STR_LEN];

    assert(telegram != NULL);

    /* print telegram type */
    knx_telegram_type_t type = knx_telegram_get_type(telegram);

    printf("telegram type: %s\n", telegram_type_mapping[type]);

    if (type == KNX_TELEGRAM_TYPE_UNKNOWN) {
        return;
    }

    /* print length and payload length */
    printf("length: %u byte(s)\n", (unsigned)len);
    printf("payload length: %u byte(s)\n",
           (unsigned)knx_telegram_get_payload_length(telegram));

    /* print source address */
    knx_telegram_get_src_addr(telegram, &addr);

    printf("source address: %s\n",
           knx_addr_physical_to_str(addr_str, &addr, sizeof(addr_str)));

    /* print destination address */
    knx_telegram_get_dst_addr(telegram, &addr);

    if (knx_telegram_is_group_addressed(telegram)) {
        printf("destination address: %s\n",
               knx_addr_group_to_str(addr_str, &addr, sizeof(addr_str)));
    }
    else {
        printf("destination address: %s\n",
               knx_addr_physical_to_str(addr_str, &addr, sizeof(addr_str)));
    }

    /* print checksum */
    uint8_t checksum = knx_telegram_get_checksum(telegram, len);

    if (knx_telegram_is_checksum_valid(telegram, len)) {
        printf("checksum: 0x%02x (valid)\n", checksum);
    }
    else {
        printf("checksum: 0x%02x (invalid)\n", checksum);
    }
}
