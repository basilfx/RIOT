/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for the TPUART transceiver
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "tpuart.h"
#include "tpuart_params.h"

static tpuart_t dev;

static void _cb(uint8_t event, void *data, void *arg)
{
    (void) arg;

    switch (event) {
        case TPUART_EVENT_TELEGRAM:
            puts("Telegram received");
            break;
        case TPUART_EVENT_TELEGRAM_INCOMPLETE:
            puts("Incomplete telegram received");
            break;
        case TPUART_EVENT_STATE:
            printf("State is 0x%02x\n", *(uint8_t *) data);
            break;
        case TPUART_EVENT_SAVE:
            puts("Bus voltage dropping");
            break;
    }
}

int main(void)
{
    int result;

    dev.cb = _cb;

    puts("This test invokes several requests and checks for the responses.\n"
         "It should finish with 'Tests succeeded'.");

    /* test init */
    result = tpuart_init(&dev, &tpuart_params[0]);

    if (result != TPUART_OK) {
        printf("Unable to initialize driver: %d\n", result);
        return 1;
    }

    /* test reset */
    result = tpuart_reset(&dev);

    if (result != TPUART_OK) {
        printf("Unable to reset: %d\n", result);
        return 1;
    }

    /* test product id */
    result = tpuart_get_product_id(&dev);

    if (result < TPUART_OK) {
        printf("Unable to retrieve product ID: %d\n", result);
        return 1;
    }

    printf("Product ID is %d\n", result);

    /* test state */
    result = tpuart_get_state(&dev);

    if (result < TPUART_OK) {
        printf("Unable to retrieve state: %d\n", result);
        return 1;
    }

    printf("State is 0x%02x\n", result);

    /* activate CRC */
    result = tpuart_activate_crc(&dev);

    if (result != TPUART_OK) {
        printf("Unable to activate CRC mode: %d\n", result);
        return 1;
    }

    /* set address */
    result = tpuart_set_resend_count(&dev, 3, 3);

    if (result != TPUART_OK) {
        printf("Unable to set resend count: %d\n", result);
        return 1;
    }

    /* set address */
    result = tpuart_set_address(&dev, 0x1108);

    if (result != TPUART_OK) {
        printf("Unable to set address: %d\n", result);
        return 1;
    }

    /* set busy mode */
    result = tpuart_activate_busy_mode(&dev);

    if (result != TPUART_OK) {
        printf("Unable to set busy mode: %d\n", result);
        return 1;
    }

    /* reset busy mode */
    result = tpuart_reset_busy_mode(&dev);

    if (result != TPUART_OK) {
        printf("Unable to reset busy mode: %d\n", result);
        return 1;
    }

    /* send a telegram */
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };
    iolist_t iolist = {
        .iol_next = NULL,
        .iol_base = &telegram,
        .iol_len = sizeof(telegram)
    };

    result = tpuart_send(&dev, &iolist);

    if (result < TPUART_OK) {
        printf("Unable to send telegram: %d\n", result);
        return 1;
    }

    /* send an extended telegram */
    uint8_t telegram_extended[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x36, 0x00, 0xb0, 0x30, 0x00,
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30, 0x00, 0x00,
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30, 0x00, 0x00,
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30, 0x00, 0x00,
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30, 0x00, 0x00,
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x48
    };
    iolist_t iolist_extended = {
        .iol_next = NULL,
        .iol_base = &telegram_extended,
        .iol_len = sizeof(telegram_extended)
    };

    result = tpuart_send(&dev, &iolist_extended);

    if (result < TPUART_OK) {
        printf("Unable to send extended telegram: %d\n", result);
        return 1;
    }

    puts("Tests succeeded.");

    return 0;
}
