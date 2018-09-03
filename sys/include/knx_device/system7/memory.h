/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_knx_device_system7
 *
 * @{
 *
 * @file
 * @brief       KNX Device memory interface
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef KNX_DEVICE_SYSTEM7_MEMORY_H
#define KNX_DEVICE_SYSTEM7_MEMORY_H

#include "knx_device/system7/property.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   The minimum memory address
 */
#define KNX_MEMORY_ADDRESS_MIN (0x0000)

/**
 * @brief   The maximum memory address
 */
#define KNX_MEMORY_ADDRESS_MAX (0xFFFF)

/**
 * @brief   Definition memory segment end marker
 */
#define KNX_MEMORY_SEGMENT_END { .start = KNX_MEMORY_ADDRESS_MAX }

/**
 * @brief   Memory types
 */
typedef enum {
    KNX_MEMORY_TYPE_EEPROM,
    KNX_MEMORY_TYPE_RAM,
    KNX_MEMORY_TYPE_FLASH
} knx_memory_type_t;

/**
 * @brief   Memory flags
 */
enum {
    KNX_MEMORY_FLAG_NONE = 0x00,        /**< No memory flags */
    KNX_MEMORY_FLAG_READABLE = 0x01,    /**< Memory segment readable */
    KNX_MEMORY_FLAG_WRITABLE = 0x02,    /**< Memory segment writeable */
    KNX_MEMORY_FLAG_MODIFIED = 0x10     /**< Memory segment was modified */
};

/**
 * @brief   Definition of a memory segment
 */
typedef struct {
    uint16_t start;             /**< Start address */
    uint16_t size;              /**< Size of the segment */
    knx_memory_type_t type;     /**< Memory segment type */
    uint8_t flags;              /**< Memory segment flags */
    void *ptr;                  /**< Pointer to actual memory */
} knx_memory_segment_t;

/**
 * @brief   Find a memory segment given an address and size
 *
 * The address + size must fit within the designated memory segment, otherwise
 * it will not be returned.
 *
 * Overlapping segements are not supported, neither are spanning segments.
 *
 * @param[in] segments      Array of memory segments (must not be @p NULL)
 * @param[in] addr          Start address of segment
 * @param[in] size          Size of the segment
 *
 * @return  reference to memory segment if found
 * @return  @p NULL if memory segment not found, or no suitable segment found
 */
knx_memory_segment_t *knx_memory_find(knx_memory_segment_t *segments, uint16_t addr, uint16_t size);

/**
 * @brief   Read the contents of a memory segment into a buffer
 *
 * @param[in] segment       Reference to a segment (must not be @p NULL)
 * @param[in] addr          Start address (must be within @p segment)
 * @param[in] size          Number of bytes to read (must fit @p segment)
 * @param[out] buf          Destination buffer (must at least be @p size bytes)
 *
 * @return -1 if @p segmet is not readable (missing KNX_MEMORY_FLAG_READABLE)
 * @return -2 if @p buf is @p NULL
 * @return otherwise, the number of bytes copied
 */
ssize_t knx_memory_read(const knx_memory_segment_t *segment, uint16_t addr, uint16_t size, uint8_t *buf);

/**
 * @brief   Write contents of a buffer into a memory segment.
 *
 * @param[in] segment       Reference to a segment (must not be @p NULL)
 * @param[in] addr          Start address (must be within @p segment)
 * @param[in] size          Number of bytes to write (must fit @p segment)
 * @param[out] buf          Source buffer (must at least be @p size bytes)
 *
 * @return -1 if @p segmet is not readable (missing KNX_MEMORY_FLAG_READABLE)
 * @return -2 if @p buf is @p NULL
 * @return otherwise, the number of bytes copied
 */
ssize_t knx_memory_write(knx_memory_segment_t *segment, uint16_t addr, uint16_t size, const uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif /* KNX_DEVICE_SYSTEM7_MEMORY_H */
/** @} */
