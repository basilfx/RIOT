/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>

#include "common/sensors.h"

#include "print.h"
#include "memory.h"

#include "mpu.h"
#include "net/knx.h"

#ifdef MODULE_RIOTBOOT
#include "riotboot/slot.h"
#endif

#include "em_rmu.h"

void print_hardware_info(void)
{
    puts("Hardware:");

    /* print board version */
#ifdef BOARD_VERSION_110
    printf("%15s: %s\n", "Version", "1.1.0");
#endif
#ifdef BOARD_VERSION_120
    printf("%15s: %s\n", "Version", "1.2.0");
#endif

#ifdef PART_NUMBER
    /* print CPU name */
    printf("%15s: %s\n", "MCU", PART_NUMBER);
#endif

    /* print MPU status */
    printf("%15s: %s\n", "MPU enabled", mpu_enabled() ? "YES" : "NO");

#ifdef RMU_PRESENT
    /* print reset reason */
    uint32_t reset = RMU_ResetCauseGet();
    RMU_ResetCauseClear();

    char *reason = "unexpected";

    if (reset & RMU_RSTCAUSE_SYSREQRST) {
        reason = "software reset";
    }
    else if (reset & RMU_RSTCAUSE_PORST) {
        reason = "power on reset";
    }
    else if (reset & RMU_RSTCAUSE_EXTRST) {
        reason = "external pin";
    }

    printf("%15s: %s\n", "Last reset", reason);
#endif
    puts("");
}

void print_firmware_info(void)
{
    puts("Firmware:");

    /* print firmware version */
    printf("%15s: %d\n", "Version", APP_VER);

#ifdef MODULE_RIOTBOOT
    int slot = riotboot_slot_current();
    const riotboot_hdr_t *header = riotboot_slot_get_hdr(slot);

    /* print slot information */
    printf("%15s: %d\n", "Slot", slot);

    /* print checksum */
    printf("%15s: %04lx\n", "Checksum", header->chksum);
#endif
    puts("");
}

void print_device_info(knx_device_t *device)
{
    puts("KNX device:");

    /* print mask version */
    printf("%15s: %04x\n", "Mask version", device->mask_version);

    /* print device address */
    char buf[KNX_ADDR_MAX_STR_LEN];

    knx_addr_individual_to_str(buf, &(device->address), sizeof(buf));

    printf("%15s: %s\n", "Address", buf);

    /* print device properties */
    printf("%15s: %02x%02x:%02x%02x%02x%02x\n", "Serial",
           device->info->serial[0], device->info->serial[1],
           device->info->serial[2], device->info->serial[3],
           device->info->serial[4], device->info->serial[5]);

    /* print table status */
    printf("%15s: %d\n", "Associations", device->associations->count);
    puts("");
}

void print_settings(knx_device_t *device)
{
    (void)device;

    memory_settings_t *settings = device->segments[MEMORY_SEGMENT_SETTINGS].ptr;

    /* print general settings */
    puts("General:");
    printf("%20s: %d ms\n", "Startup Delay",
           byteorder_ntohs(settings->general.startup_delay));
    printf("%20s: %s\n", "Randomize Startup",
           settings->general.randomize_startup_delay ? "YES" : "NO");
    puts("");

    /* print channel settings */
    for (unsigned i = 0; i < 6; i++) {
        printf("Channel %u:\n", i);
        printf("%15s: %s\n", "Enabled",
               settings->channels[i].enabled ? "YES" : "NO");
        printf("%15s: %d\n", "Function",
               settings->channels[i].function);
        printf("%15s: %s\n", "Invert",
               settings->channels[i].invert ? "YES" : "NO");
        printf("%15s: %s\n", "Pull",
               settings->channels[i].pull ? "YES" : "NO");
        puts("");
    }

    /* print timings */
    puts("Timings:");
    printf("%15s: %d ms\n", "Debounce",
           byteorder_ntohs(settings->timings.debounce));
    printf("%15s: %d ms\n", "Press",
           byteorder_ntohs(settings->timings.press));
    printf("%15s: %d ms\n", "Long press",
           byteorder_ntohs(settings->timings.long_press));
    printf("%15s: %d ms\n", "Longer press",
           byteorder_ntohs(settings->timings.longer_press));
    puts("");

    /* print sensor settings */
    for (unsigned i = 0; i < 4; i++) {
        printf("Sensor %u:\n", i);
        printf("%15s: %s\n", "Enabled",
               settings->sensors[i].enabled ? "YES" : "NO");
        printf("%15s: %s\n", "Smart update",
               settings->sensors[i].smart_update ? "YES" : "NO");
        printf("%15s: %d %%\n", "Difference",
               settings->sensors[i].difference);
        printf("%15s: %d s\n", "Update time",
               byteorder_ntohs(settings->sensors[i].update_time));
        printf("%15s: %d s\n", "Update time max",
               byteorder_ntohs(settings->sensors[i].update_time_max));
        printf("%15s: %s\n", "Source",
               settings->sensors[i].source);
        printf("%15s: %s\n", "Smoothing",
               sensors_smoothing_algorithm_to_str(settings->sensors[i].smoothing));
        printf("%15s: %d %%\n", "EMA alpha",
               settings->sensors[i].smoothing_ema_alpha);
        printf("%15s: %d\n", "MA samples",
               settings->sensors[i].smoothing_ma_samples);
        puts("");
    }
}
