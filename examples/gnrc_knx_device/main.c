/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
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
#include "net/gnrc/knx_l7.h"

#include "shell.h"
#include "shell_commands.h"

#include "periph/gpio.h"
#include "periph/pm.h"

#define COM_OBJECT_LED 0
#define COM_OBJECT_BUTTON 1

/**
 * @brief   List of communication objects (objects which can be interacted
 *          with)
 */
static knx_com_object_t com_objects[] = {
    [COM_OBJECT_LED] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .access = KNX_COM_OBJECT_ACCESS_ENABLED | KNX_COM_OBJECT_ACCESS_WRITE,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.value = { false }
    },
    [COM_OBJECT_BUTTON] = {
        .type = KNX_COM_OBJECT_TYPE_BIT1,
        .access = KNX_COM_OBJECT_ACCESS_ENABLED | KNX_COM_OBJECT_ACCESS_TRANSMIT,
        .priority = KNX_TELEGRAM_PRIORITY_LOW,
        .content.value = { false }
    },
    KNX_COM_OBJECT_END
};

/**
 * @brief   Mapping between a group address and a communication object (there
 *          can be multiple mappings per group address or communication object)
 *
 * Normally, you would calculate this using `knx_assoc_update` method, given a
 * association table uploaded to a device. This must be an ordered list of
 * mappings.
 */
static knx_assoc_mapping_t mappings[] = {
    {
        .group_addr = { { 0x00, 0x01 } },
        .com_object = &(com_objects[COM_OBJECT_LED])
    },
    {
        .group_addr = { { 0x00, 0x02 } },
        .com_object = &(com_objects[COM_OBJECT_BUTTON])
    }
};

/**
 * @brief   Wrapper for the mappings (contains information about the number of
 *          mappings)
 */
static knx_assoc_t associations = {
    .mappings = mappings,
    .count = ARRAY_SIZE(mappings)
};

/**
 * @brief   Device meta data definition
 */
static knx_table_device_t device_table = {
    .manufacturer_id = { .u8 = { 0x3A, 0x01 } },
    .serial = { 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 },
};

/**
 * @brief   Device descriptor definition
 */
knx_device_t device = {
    .mask_version = KNX_DEVICE_MASK_0705,
    .address = { { 0x32, 0x01 } },
    .info = &device_table,
    .segments = NULL,
    .objects = NULL,
    .associations = &associations,
    .com_objects = com_objects,
};

void _com_object_write(knx_device_t *dev, knx_event_com_object_write_t *event)
{
    (void)dev;

    LOG_DEBUG("[main] _com_object_write: object id=%p\n", event->com_object);

    if (event->com_object == &(com_objects[COM_OBJECT_LED])) {
        /* write the new value of the LED to the GPIO port */
        gpio_write(LED0_PIN, com_objects[COM_OBJECT_LED].content.value[0]);
    }
    else {
        LOG_DEBUG("[main] _com_object_write: unhandled\n");
    }
}

static void _cb_pin(void *arg)
{
    (void)arg;

    com_objects[COM_OBJECT_BUTTON].content.value[0] = !com_objects[COM_OBJECT_BUTTON].content.value[0];

    /* send the updates */
    gnrc_knx_l7_update_com_object(&device, &(com_objects[COM_OBJECT_BUTTON]));
}

static void _cb_knx(knx_device_t *dev, knx_device_event_t event, void *args)
{
    LOG_DEBUG("[main] _cb_knx: received event %d with args %p\n", event, args);

    switch (event) {
        case KNX_DEVICE_EVENT_RESTART:
            LOG_INFO("[main] _cb_knx: reboot requested\n");
            break;
        case KNX_DEVICE_EVENT_COM_OBJECT_WRITE:
            _com_object_write(dev, (knx_event_com_object_write_t *)args);
            break;
        default:
            LOG_DEBUG("[main] _cb_knx: event not handled\n");
    }
}

int main(void)
{
    /* retrieve interface */
    netif_t *iface = NULL;

    while (1) {
        iface = netif_iter(iface);

        /* stop on last/no iface */
        if (iface == NULL) {
            LOG_ERROR("[main] main: unable to find a KNX iface\n");
            return 1;
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
        LOG_ERROR("[main] main: unable to set addr\n");
        return 1;
    }

    /* enable checksumming */
    netopt_enable_t checksums = NETOPT_ENABLE;

    if (netif_set_opt(iface, NETOPT_CHECKSUM, 0, &checksums, sizeof(checksums)) < 0) {
        LOG_ERROR("[main] main: unable to enable checksums\n");
        return 1;
    }

    /* initialize device */
    device.event_callback = _cb_knx;
    device.iface = iface;

    /* add pin callback */
    gpio_init_int(PB0_PIN, GPIO_IN_PU, GPIO_RISING, _cb_pin, NULL);

    /* initialize application layer */
    gnrc_knx_l7_init(&device);

    /* prepare shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
