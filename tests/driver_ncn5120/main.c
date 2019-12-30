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
 * @brief       Test application for the NCN5120 transceiver
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "ncn5120.h"
#include "ncn5120_params.h"

static ncn5120_t dev;

static void _cb(uint8_t event, void *data, void *arg)
{
    (void)arg;

    switch (event) {
        case NCN5120_EVENT_TELEGRAM:
            puts("Telegram received");
            break;
        case NCN5120_EVENT_TELEGRAM_INCOMPLETE:
            puts("Incomplete telegram received");
            break;
        case NCN5120_EVENT_STATE:
            printf("State is 0x%02x\n", *(uint8_t *)data);
            break;
        case NCN5120_EVENT_SYSTEM_STATE:
            printf("System state is 0x%02x\n", *(uint8_t *)data);
            break;
        case NCN5120_EVENT_SAVE:
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
    result = ncn5120_init(&dev, &ncn5120_params[0]);

    if (result != NCN5120_OK) {
        printf("Unable to initialize driver: %d\n", result);
        return 1;
    }

    /* test reset */
    result = ncn5120_reset(&dev);

    if (result != NCN5120_OK) {
        printf("Unable to reset: %d\n", result);
        return 1;
    }

    /* test state */
    result = ncn5120_get_state(&dev);

    if (result < NCN5120_OK) {
        printf("Unable to retrieve state: %d\n", result);
        return 1;
    }

    printf("State is 0x%02x\n", result);

    /* test system state */
    result = ncn5120_get_system_state(&dev);

    if (result < NCN5120_OK) {
        printf("Unable to retrieve system state: %d\n", result);
        return 1;
    }

    printf("System state is 0x%02x\n", result);

    /* activate CRC */
    result = ncn5120_configure(&dev, NCN5120_CONFIGURE_CRC_CTIT);

    if (result < NCN5120_OK) {
        printf("Unable to configure: %d\n", result);
        return 1;
    }

    /* set address */
    result = ncn5120_set_repetition(&dev, 3, 3);

    if (result != NCN5120_OK) {
        printf("Unable to set resend count: %d\n", result);
        return 1;
    }

    /* set address */
    result = ncn5120_set_address(&dev, 0x1103);

    if (result != NCN5120_OK) {
        printf("Unable to set address: %d\n", result);
        return 1;
    }

    /* set busy mode */
    result = ncn5120_set_busy_mode(&dev);

    if (result != NCN5120_OK) {
        printf("Unable to set busy mode: %d\n", result);
        return 1;
    }

    /* reset busy mode */
    result = ncn5120_quit_busy(&dev);

    if (result != NCN5120_OK) {
        printf("Unable to quit busy: %d\n", result);
        return 1;
    }

    /* enter stop mode */
    result = ncn5120_stop_mode(&dev);

    if (result != NCN5120_OK) {
        printf("Unable to set busy mode: %d\n", result);
        return 1;
    }

    /* exit stop mode */
    result = ncn5120_exit_stop_mode(&dev);

    if (result != NCN5120_OK) {
        printf("Unable to set busy mode: %d\n", result);
        return 1;
    }

    /* write to register */
    result = ncn5120_reg_write(&dev, NCN5120_REG_EXT_WATCHDOG_CTRL, 0x00);

    if (result != NCN5120_OK) {
        printf("Unable to write register: %d\n", result);
        return 1;
    }

    /* read from register */
    result = ncn5120_reg_read(&dev, NCN5120_REG_EXT_WATCHDOG_CTRL);

    if (result < NCN5120_OK) {
        printf("Unable to read register: %d\n", result);
        return 1;
    }

    printf("Register value is 0x%02x\n", result);

    /* send a telegram */
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };
    iolist_t iolist = {
        .iol_next = NULL,
        .iol_base = &telegram,
        .iol_len = sizeof(telegram)
    };

    result = ncn5120_send(&dev, &iolist);

    if (result < NCN5120_OK) {
        printf("Unable to send telegram: %d\n", result);
        return 1;
    }

    /* send an extended telegram (>63 bytes, so this will trigger a data offset
       request) */
    uint8_t telegram_extended[] = {
        0x3c, 0x60, 0x11, 0x03, 0x01, 0x01, 0x39, 0x00, 0xb0, 0x30, 0x00,
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30, 0x00, 0x00,
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30, 0x00, 0x00,
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30, 0x00, 0x00,
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30, 0x00, 0x00,
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30, 0x00, 0xf7
    };
    iolist_t iolist_extended = {
        .iol_next = NULL,
        .iol_base = &telegram_extended,
        .iol_len = sizeof(telegram_extended)
    };

    result = ncn5120_send(&dev, &iolist_extended);

    if (result < NCN5120_OK) {
        printf("Unable to send extended telegram: %d\n", result);
        return 1;
    }

    puts("Tests succeeded.");

    return 0;
}
