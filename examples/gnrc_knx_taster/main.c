/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example KNX Device
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <string.h>
#include <stdlib.h>

#include "board.h"
#include "log.h"

#include "net/netif.h"
#include "net/knx.h"
#include "net/gnrc/knx_l3.h"
#include "net/gnrc/knx_l4.h"
#include "net/gnrc/knx_l7.h"

#include "xtimer.h"

#include "shell.h"
#include "shell_commands.h"

#include "device.h"
#include "memory.h"
#include "storage.h"

#include "display.h"
#include "print.h"

#include "periph/gpio.h"
#include "periph/pm.h"

typedef enum {
    DEVICE_ERROR_STORAGE = 2,
    DEVICE_ERROR_INTERFACE = 3,
} device_error_t;

static int cmd_reset(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /* parse arguments */
    if (argc != 2) {
        printf("usage: %s <confirm>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "1") != 0) {
        puts("Confirm reset with '1'.");
        return 0;
    }

    storage_prepare();
    pm_reboot();

    return 0;
}

static int cmd_settings(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    print_settings(&device);

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "reset", "factory reset device", cmd_reset },
    { "settings", "print settings", cmd_settings },
    { NULL, NULL, NULL }
};

static netif_t* _knx_init(void)
{
    netif_t *iface = NULL;

    while (1) {
        iface = netif_iter(iface);

        /* stop on last/no iface */
        if (iface == NULL) {
            LOG_ERROR("[main] knx_init: unable to find a KNX iface\n");
            return NULL;
        }

        /* assert iface type and stop here if its a KNX iface */
        uint16_t type;

        if (netif_get_opt(iface, NETOPT_DEVICE_TYPE, 0, &type, sizeof(type)) < 0) {
            continue;
        }

        if (type == NETDEV_TYPE_KNX) {
            break;
        }
    }

    /* set address */
    if (netif_set_opt(iface, NETOPT_ADDRESS, 0, &(device.address), sizeof(device.address)) < 0) {
        LOG_ERROR("[main] knx_init: unable to set addr\n");
        return NULL;
    }

    /* enable checksumming */
    netopt_enable_t checksums = NETOPT_ENABLE;

    if (netif_set_opt(iface, NETOPT_CHECKSUM, 0, &checksums, sizeof(checksums)) < 0) {
        LOG_ERROR("[main] knx_init: unable to enable checksums\n");
        return NULL;
    }

    return iface;
}

static void _error(device_error_t error)
{
    while (1) {
        for (unsigned i = 0; i < (unsigned)error; i++) {
            gpio_set(LED0_PIN);
            xtimer_msleep(500);
            gpio_clear(LED0_PIN);
            xtimer_msleep(500);
        }

        xtimer_msleep(5000);
    }
}

int main(void)
{
    print_hardware_info();
    print_firmware_info();

    /* prepare storage */
    if (storage_init() < 0) {
        LOG_ERROR("[main] main: unable to initialize storage\n");
        _error(DEVICE_ERROR_STORAGE);

        return 1;
    }

    storage_read();

    /* retrieve interface */
    netif_t *iface = _knx_init();

    if (iface == NULL) {
        LOG_ERROR("[main]: main: interface not ready, aborting\n");
        _error(DEVICE_ERROR_INTERFACE);

        return 1;
    }

    /* setup KNX device */
    device_reset();
    device_init(iface);

    /* print more information */
    print_device_info(&device);

    /* initialize application layer */
    gnrc_knx_l7_init(&device);

    /* prepare shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
