/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>
#include <string.h>

#include "board.h"
#include "storage.h"
#include "device.h"
#include "memory.h"

#include "mutex.h"

#include "fs/littlefs_fs.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief   Holds the magic value to identify valid flash data
 */
#define STORAGE_MAGIC { 0x55, 0xaa, 0x55, 0xaa }

/**
 * @brief   Global lock for mutual exclusive access to flash data
 */
static mutex_t lock = MUTEX_INIT;

/**
 * @brief   File system descriptor
 */
static littlefs_desc_t fs_desc = {
    .lock = MUTEX_INIT,
};

/**
 * @brief   File system mount point
 */
static vfs_mount_t flash_mount = {
    .fs = &littlefs_file_system,
    .mount_point = "/mtd0",
    .private_data = &fs_desc
};

static ssize_t _read_file(char *path, void *dest, size_t len)
{
    FILE *f = fopen(path, "rb");

    if (f == NULL) {
        DEBUG("[storage] _read_file: unable to read %s\n", path);
        return -1;
    }

    /* read the file size */
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < len) {
        DEBUG("[storage] _read_file: %d bytes expected from %s, got %d bytes\n", (unsigned)len, path, (unsigned)size);
        fclose(f);
        return -1;
    }

    /* read len bytes into buf */
    if (fread(dest, sizeof(uint8_t), len, f) != len) {
        DEBUG("[storage] _read_file: unable to read from file\n");
        fclose(f);
        return -1;
    }

    fclose(f);

    return len;
}

static ssize_t _write_file(char *path, const void *src, size_t len)
{
    FILE *f = fopen(path, "wb");

    if (f == NULL) {
        DEBUG("[storage] _write_file: unable to write %s\n", path);
        return -1;
    }

    if (fwrite(src, sizeof(uint8_t), len, f) != len) {
        DEBUG("[storage] _write_file: unable to write to file\n");
        fclose(f);
        return -1;
    }

    fclose(f);

    return len;
}

int storage_init(void)
{
    fs_desc.dev = MTD_0;

    /* mount file system */
    int res = vfs_mount(&flash_mount);

    /* try to format if mount not succeeded */
    if (res < 0) {
        res = storage_prepare();

        if (res < 0) {
            DEBUG("[storage] storage_init: unable to prepare\n");
            return -1;
        }
    }

    /* validate storage */
    res = storage_validate();

    if (res < 0) {
        res = storage_prepare();

        if (res < 0) {
            DEBUG("[storage] storage_init: unable to prepare\n");
            return -1;
        }
    }

    return 0;
}

int storage_validate(void)
{
    uint8_t expected_magic[] = STORAGE_MAGIC;
    uint8_t actual_magic[] = STORAGE_MAGIC;

    if (_read_file("/mtd0/magic", actual_magic, sizeof(actual_magic)) < 0) {
        DEBUG("[storage] storage_validate: unable to read magic\n");
        return -1;
    }

    if (memcmp(expected_magic, actual_magic, sizeof(expected_magic)) != 0) {
        DEBUG("[storage] storage_validate: magic mismatch\n");
        return -1;
    }

    uint8_t expected_version[] = { APP_VER };
    uint8_t actual_version[] = { APP_VER };

    if (_read_file("/mtd0/version", actual_version, sizeof(actual_version)) < 0) {
        DEBUG("[storage] storage_validate: unable to read version\n");
        return -1;
    }

    if (expected_version[0] != actual_version[0]) {
        DEBUG("[storage] storage_validate: version mismatch\n");
        return -1;
    }

    return 0;
}

int storage_prepare(void)
{
    int res;

    /* umount first (if needed) */
    vfs_umount(&flash_mount);

    /* format mount point */
    res = vfs_format(&flash_mount);

    if (res < 0) {
        DEBUG("[storage] storage_prepare: unable to format\n");
        return -1;
    }

    /* mount again */
    res = vfs_mount(&flash_mount);

    if (res < 0) {
        DEBUG("[storage] storage_prepare: unable to mount after format\n");
        return -1;
    }

    /* create necessary files */
    uint8_t magic[] = STORAGE_MAGIC;

    if (_write_file("/mtd0/magic", magic, sizeof(magic)) < 0) {
        DEBUG("[storage] storage_prepare: unable to write magic\n");
        return -1;
    }

    uint8_t version[] = { APP_VER };

    if (_write_file("/mtd0/version", version, sizeof(version)) < 0) {
        DEBUG("[storage] storage_prepare: unable to write version\n");
        return -1;
    }

    return 0;
}

int storage_read(void)
{
    char path[16];

    _read_file("/mtd0/address", &(device.address), sizeof(device.address));

    for (unsigned i = 0; segments[i].start != 0xffff; i++) {
        if (segments[i].type == KNX_MEMORY_TYPE_EEPROM) {
            snprintf(path, sizeof(path), "/mtd0/segment%d", i);
            _read_file(path, segments[i].ptr, segments[i].size);
        }
    }

    return 0;
}

int storage_write(void)
{
    char path[16];

    _write_file("/mtd0/address", &(device.address), sizeof(device.address));

    for (unsigned i = 0; segments[i].start != 0xffff; i++) {
        if (segments[i].type == KNX_MEMORY_TYPE_EEPROM) {
            if (segments[i].flags & KNX_MEMORY_FLAG_MODIFIED) {
                snprintf(path, sizeof(path), "/mtd0/segment%d", i);
                _write_file(path, segments[i].ptr, segments[i].size);

                /* mark as unmodified */
                segments[i].flags &= ~(KNX_MEMORY_FLAG_MODIFIED);
            }
        }
    }

    return 0;
}

void storage_acquire(void)
{
    mutex_lock(&lock);
}

void storage_release(void)
{
    mutex_unlock(&lock);
}
