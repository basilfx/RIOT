/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     sys_shell_commands
 * @{
 *
 * @file
 * @brief       KNX Device commands
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "od.h"
#include "knx_device.h"

/**
 * @note    It is currently assumed that there is a knx_device_t somewhere.
 */
extern knx_device_t device;

static int _addr_get(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    char tmp[KNX_ADDR_MAX_STR_LEN];

    knx_addr_physical_to_str(tmp, &(device.address), sizeof(tmp));
    printf("KNX device address is %s\n", tmp);

    return 0;
}

static int _addr_set(int argc, char **argv)
{
    /* parse arguments */
    if (argc != 4) {
        printf("usage: %s %s set <addr>\n", argv[0], argv[1]);
        return 1;
    }

    /* interpret the address first */
    knx_addr_physical_t tmp;

    if (knx_addr_physical_from_str(&tmp, argv[3]) == NULL) {
        printf("error: unable to convert address from string (e.g. 1.2.3)\n");
        return 1;
    }

    /* set the address */
    return 0;
}

static int _addr(int argc, char **argv)
{
    /* parse arguments */
    if (argc < 3) {
        printf("usage: %s %s <get|set>\n", argv[0], argv[1]);
        return 1;
    }

    /* perform commands */
    if (strcmp(argv[2], "get") == 0) {
        return _addr_get(argc, argv);
    }
    else if (strcmp(argv[2], "set") == 0) {
        return _addr_set(argc, argv);
    }
    else {
        printf("error: unsupported operation: %s\n", argv[2]);
        return 1;
    }
}

static int _assoc_list(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /* check if associations are available */
    if (device.associations == NULL) {
        puts("error: no associations are available");
        return 1;
    }

    /* print all associations */
    char tmp[KNX_ADDR_MAX_STR_LEN];

    puts("Assocations:");

    for (unsigned i = 0; i < device.associations->count; i++) {
        knx_addr_group_to_str(tmp, &(device.associations->mappings[i].group_addr), sizeof(tmp));
        printf("- %s -> %p\n", tmp, device.associations->mappings[i].com_object);
    }

    return 0;
}

static int _assoc(int argc, char **argv)
{
    /* parse arguments */
    if (argc == 2) {
        printf("usage: %s %s <list>\n", argv[0], argv[1]);
        return 1;
    }

    /* perform commands */
    if (strcmp(argv[2], "list") == 0) {
        return _assoc_list(argc, argv);
    }
    else {
        printf("error: unsupported operation: %s\n", argv[2]);
        return 1;
    }
}

static int _com_read(int argc, char **argv)
{
    uint8_t index;
    uint8_t data[32];

    if (argc < 4) {
        printf("usage: %s %s read <index>\n", argv[0], argv[1]);
        return 1;
    }

    /* parse arguments */
    index = strtoul(argv[3], NULL, 10);

    /* find communication object */
    for (unsigned i = 0; device.com_objects[i].flags != 0xFF; i++) {
        if (i == index) {
            ssize_t result = knx_com_object_read(&(device.com_objects[i]), data, sizeof(data));

            if (result < 0) {
                puts("error: unable to read from communication object");
                return 1;
            }

            od_hex_dump(data, result, OD_WIDTH_DEFAULT);

            return 0;
        }
    }

    puts("error: communication object not found");
    return 1;
}

static int _com_write(int argc, char **argv)
{
    uint8_t index;
    uint8_t size = 0;
    uint8_t data[32];

    if (argc < 5) {
        printf("usage: %s %s write <index> <data-1:hex> ... <data-n:hex>\n", argv[0], argv[1]);
        return 1;
    }

    /* parse arguments */
    index = strtoul(argv[3], NULL, 10);
    size = 0;

    for (unsigned i = 4; i < (unsigned)argc; i++) {
        if ((i - 4) < sizeof(data)) {
            data[i - 4] = strtoul(argv[i], NULL, 16);
            size++;
        }
    }

    /* find communication object */
    for (unsigned i = 0; device.com_objects[i].flags != 0xFF; i++) {
        if (i == index) {
            ssize_t result = knx_com_object_write(&(device.com_objects[i]), data, size);

            if (result < 0) {
                puts("error: unable to write to communication object");
                return 1;
            }

            return 0;
        }
    }

    puts("error: communication object not found");
    return 1;
}

static int _com_list(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /* check if properties are available */
    if (device.com_objects == NULL) {
        puts("error: no communication objects available");
        return 1;
    }

    /* list all segments */
    puts("Communication objects:");

    for (unsigned i = 0; device.com_objects[i].flags != 0xFF; i++) {
        printf("- %i -> %p (type %d, flags %02x, %d bytes):\n",
               i,
               &(device.com_objects[i]),
               device.com_objects[i].type,
               device.com_objects[i].flags,
               (int)knx_com_object_size(&(device.com_objects[i])));
    }

    return 0;
}

static int _com(int argc, char **argv)
{
    /* parse arguments */
    if (argc == 2) {
        printf("usage: %s %s <read|write|list>\n", argv[0], argv[1]);
        return 1;
    }

    /* perform commands */
    if (strcmp(argv[2], "read") == 0) {
        return _com_read(argc, argv);
    }
    else if (strcmp(argv[2], "write") == 0) {
        return _com_write(argc, argv);
    }
    else if (strcmp(argv[2], "list") == 0) {
        return _com_list(argc, argv);
    }
    else {
        printf("error: unsupported operation: %s\n", argv[2]);
        return 1;
    }
}

static int _mem_read(int argc, char **argv)
{
    uint16_t start = 0;
    uint16_t size = 0;

    if (argc != 5 && argc != 4) {
        printf("usage: %s %s read <start:hex> [size:hex]\n", argv[0], argv[1]);
        return 1;
    }
    else if (argc == 4) {
        start = strtoul(argv[3], NULL, 16);
        size = 0xffff;
    }
    else if (argc == 5) {
        start = strtoul(argv[3], NULL, 16);
        size = strtoul(argv[4], NULL, 16);
    }

    /* find memory segment */
    knx_memory_segment_t *segment = knx_memory_find(device.segments, start, 1);

    if (segment == NULL) {
        printf("error: segment not mapped\n");
        return 1;
    }

    /* clip size to segment size */
    size = size > segment->size ? segment->size : size;

    /* dump contents */
    od_hex_dump(&((uint8_t *)segment->ptr)[start - segment->start], size, OD_WIDTH_DEFAULT);

    return 0;
}

static int _mem_write(int argc, char **argv)
{
    uint16_t start;
    uint16_t size = 0;
    uint8_t data[8];

    if (argc < 5) {
        printf("usage: %s %s write <start:hex> <data-1:hex> ... <data-n:hex>\n", argv[0], argv[1]);
        return 1;
    }
    else {
        start = strtoul(argv[3], NULL, 16);
        size = 0;

        for (unsigned i = 3; i < (unsigned)argc; i++) {
            if ((i - 3) < sizeof(data)) {
                data[i - 3] = strtoul(argv[i], NULL, 16);
                size++;
            }
        }
    }

    /* find memory segment */
    knx_memory_segment_t *segment = knx_memory_find(device.segments, start, 1);

    if (segment == NULL) {
        printf("error: segment not mapped\n");
        return 1;
    }

    ssize_t result = knx_memory_write(segment, start, size, data);

    if (result < 0) {
        printf("error: unable to write %d bytes to %04x\n", size, start);
        return 1;
    }

    return 0;
}

static int _mem_list(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /* check if segments are available */
    if (device.segments == NULL) {
        puts("error: no memory segments available");
        return 1;
    }

    /* list all segments */
    puts("Segments:");

    for (unsigned i = 0; device.segments[i].start != 0xffff; i++) {
        char *mode;

        if ((device.segments[i].flags & (KNX_MEMORY_FLAG_READABLE | KNX_MEMORY_FLAG_WRITABLE)) == (KNX_MEMORY_FLAG_READABLE | KNX_MEMORY_FLAG_WRITABLE)) {
            mode = "rw";
        }
        else if (device.segments[i].flags & KNX_MEMORY_FLAG_READABLE) {
            mode = "r";
        }
        else if (device.segments[i].flags & KNX_MEMORY_FLAG_WRITABLE) {
            mode = "w";
        }
        else {
            mode = "";
        }

        printf("- %04x -> %04x (%d bytes, %s)\n",
               device.segments[i].start,
               device.segments[i].start + device.segments[i].size,
               device.segments[i].size,
               mode);
    }

    return 0;
}

static int _mem(int argc, char **argv)
{
    /* parse arguments */
    if (argc == 2) {
        printf("usage: %s %s <read|write|list>\n", argv[0], argv[1]);
        return 1;
    }

    /* perform commands */
    if (strcmp(argv[2], "read") == 0) {
        return _mem_read(argc, argv);
    }
    else if (strcmp(argv[2], "write") == 0) {
        return _mem_write(argc, argv);
    }
    else if (strcmp(argv[2], "list") == 0) {
        return _mem_list(argc, argv);
    }
    else {
        printf("error: unsupported operation: %s\n", argv[2]);
        return 1;
    }
}

static int _prog(int argc, char **argv)
{
    /* parse arguments */
    if (argc < 3) {
        printf("usage: %s %s <status|on|off|toggle>\n", argv[0], argv[1]);
        return 1;
    }

    /* perform commands */
    if (strcmp(argv[2], "status") == 0) {
        if (device.info->programming_mode) {
            puts("Programming mode is ON");
        }
        else {
            puts("Programming mode is OFF");
        }
    }
    else if (strcmp(argv[2], "on") == 0) {
        device.info->programming_mode = true;
    }
    else if (strcmp(argv[2], "off") == 0) {
        device.info->programming_mode = false;
    }
    else if (strcmp(argv[2], "toggle") == 0) {
        device.info->programming_mode = !device.info->programming_mode;
    }
    else {
        printf("error: unsupported operation: %s\n", argv[2]);
        return 1;
    }

    return 0;
}

static int _prop_read(int argc, char **argv)
{
    uint8_t object;
    uint8_t property;
    uint8_t data[32];

    if (argc < 5) {
        printf("usage: %s %s read <object> <property>\n", argv[0], argv[1]);
        return 1;
    }

    /* parse arguments */
    object = strtoul(argv[3], NULL, 10);
    property = strtoul(argv[4], NULL, 10);

    /* find communication object */
    knx_property_t *prop = knx_property_find_by_id(device.objects,
                                                   object,
                                                   property);

    if (prop == NULL) {
        puts("error: property not found");
        return 1;
    }

    /* read from property */
    ssize_t result = knx_property_read(prop, 1, 1, data, sizeof(data));

    if (result < 0) {
        puts("error: unable to read from property");
        return 1;
    }

    od_hex_dump(data, result, OD_WIDTH_DEFAULT);

    return 0;
}

static int _prop_write(int argc, char **argv)
{
    uint8_t object;
    uint8_t property;
    uint8_t size;
    uint8_t data[32];

    if (argc < 6) {
        printf("usage: %s %s read <object> <property> <data-1:hex> ... <data-n:hex>\n", argv[0], argv[1]);
        return 1;
    }

    /* parse arguments */
    size = 0;

    object = strtoul(argv[3], NULL, 10);
    property = strtoul(argv[4], NULL, 10);

    for (unsigned i = 3; i < (unsigned)argc; i++) {
        if ((i - 3) < sizeof(data)) {
            data[i - 3] = strtoul(argv[i], NULL, 16);
            size++;
        }
    }

    /* find communication object */
    knx_property_t *prop = knx_property_find_by_id(device.objects,
                                                   object,
                                                   property);

    if (prop == NULL) {
        puts("error: property not found");
        return 1;
    }

    /* read from property */
    ssize_t result = knx_property_write(prop, 0, 0, data, sizeof(data));

    if (result < 0) {
        puts("error: unable to write to property");
        return 1;
    }

    return 0;
}

static int _prop_list(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    /* check if properties are available */
    if (device.objects == NULL) {
        puts("error: no properties available");
        return 1;
    }

    /* list all objects and their properties */
    puts("Objects:");

    for (unsigned i = 0; device.objects[i].properties != NULL; i++) {
        printf("Object %d:\n", i);

        for (unsigned j = 0; j < device.objects[i].count; j++) {
            printf("- %d -> %p (type %d, flags %02x, %d bytes)\n",
                   device.objects[i].properties[j].id,
                   &(device.objects[i].properties[j]),
                   device.objects[i].properties[j].type,
                   device.objects[i].properties[j].flags,
                   (int)knx_property_size(&(device.objects[i].properties[j])));
        }
    }

    return 0;
}

static int _prop(int argc, char **argv)
{
    /* parse arguments */
    if (argc == 2) {
        printf("usage: %s %s <list|read|write>\n", argv[0], argv[1]);
        return 1;
    }

    /* perform commands */
    if (strcmp(argv[2], "read") == 0) {
        return _prop_read(argc, argv);
    }
    else if (strcmp(argv[2], "write") == 0) {
        return _prop_write(argc, argv);
    }
    else if (strcmp(argv[2], "list") == 0) {
        return _prop_list(argc, argv);
    }
    else {
        printf("error: unsupported operation: %s\n", argv[2]);
        return 1;
    }
}

int _knx_device_handler(int argc, char **argv)
{
    if (strcmp(argv[1], "addr") == 0) {
        return _addr(argc, argv);
    }
    else if (strcmp(argv[1], "assoc") == 0) {
        return _assoc(argc, argv);
    }
    else if (strcmp(argv[1], "com") == 0) {
        return _com(argc, argv);
    }
    else if (strcmp(argv[1], "mem") == 0) {
        return _mem(argc, argv);
    }
    else if (strcmp(argv[1], "prog") == 0) {
        return _prog(argc, argv);
    }
    else if (strcmp(argv[1], "prop") == 0) {
        return _prop(argc, argv);
    }
    else {
        printf("usage: %s addr|assoc|com|mem|prog|prop\n", argv[0]);
        return 1;
    }

    return 0;
}
