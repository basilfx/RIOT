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
 * @brief       KNX Device communication object association interface
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef KNX_DEVICE_SYSTEM7_COM_OBJECT_H
#define KNX_DEVICE_SYSTEM7_COM_OBJECT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "net/knx.h"
#include "knx_device/system7/tables.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   The type of a communication object
 */
typedef enum {
    KNX_COM_OBJECT_TYPE_BIT1    = 0,    /**< 1 bit */
    KNX_COM_OBJECT_TYPE_BIT2    = 1,    /**< 2 bit */
    KNX_COM_OBJECT_TYPE_BIT3    = 2,    /**< 3 bit */
    KNX_COM_OBJECT_TYPE_BIT4    = 3,    /**< 4 bit */
    KNX_COM_OBJECT_TYPE_BIT5    = 4,    /**< 5 bit */
    KNX_COM_OBJECT_TYPE_BIT6    = 5,    /**< 6 bit */
    KNX_COM_OBJECT_TYPE_BIT7    = 6,    /**< 7 bit */
    KNX_COM_OBJECT_TYPE_BYTE1   = 7,    /**< 1 byte */
    KNX_COM_OBJECT_TYPE_BYTE2   = 8,    /**< 2 bytes */
    KNX_COM_OBJECT_TYPE_BYTE3   = 9,    /**< 3 bytes */
    KNX_COM_OBJECT_TYPE_BYTE4   = 10,   /**< 4 bytes or a float */
    KNX_COM_OBJECT_TYPE_FLOAT   = 10,   /**< 4 bytes or a float */
    KNX_COM_OBJECT_TYPE_DATA6   = 11,   /**< 6 bytes */
    KNX_COM_OBJECT_TYPE_DATA8   = 12,   /**< 8 bytes or a double */
    KNX_COM_OBJECT_TYPE_DOUBLE  = 12,   /**< 8 bytes or a double */
    KNX_COM_OBJECT_TYPE_DATA10  = 13,   /**< 10 bytes */
    KNX_COM_OBJECT_TYPE_MAXDATA = 14,   /**< 14 bytes */
    KNX_COM_OBJECT_TYPE_VARDATA = 15    /**< variable length 1-14 bytes */
} knx_com_object_type_t;

/**
 * @brief   Communication object access flags
 */
enum {
    KNX_COM_OBJECT_ACCESS_ENABLED   = 0x04,     /**< communication enabled */
    KNX_COM_OBJECT_ACCESS_READ      = 0x08,     /**< read enabled */
    KNX_COM_OBJECT_ACCESS_WRITE     = 0x10,     /**< write enabled */
    KNX_COM_OBJECT_ACCESS_TRANSMIT  = 0x40,     /**< transmit enabled */
    KNX_COM_OBJECT_ACCESS_UPDATE    = 0x80      /**< update enabled */
};

/**
 * @brief   Flags for a communication object
 */
enum {
    KNX_COM_OBJECT_FLAG_NONE        = 0x00,     /**< no flags set */
    KNX_COM_OBJECT_FLAG_MODIFIED    = 0x10,     /**< communication object modified */
    KNX_COM_OBJECT_FLAG_POINTER     = 0x40      /**< communication object uses a pointer */
};

/**
 * @brief   Indicates the last property in a list of communication objects
 */
#define KNX_COM_OBJECT_END { .flags = 0xff }

/**
 * @brief   Size of the value field of a property, for internal storing of a
 *          property size
 */
#ifndef KNX_COM_OBJECT_VALUE_SIZE
#define KNX_COM_OBJECT_VALUE_SIZE (4)
#endif

/**
 * @brief   Definition of a communication object
 */
typedef struct {
    knx_com_object_type_t type;
    knx_telegram_priority_t priority;
    uint8_t access;
    uint8_t flags;
    union {
        void *ptr;
        uint8_t value[KNX_COM_OBJECT_VALUE_SIZE];
    } content;
} knx_com_object_t;

/**
 * @brief   Update the communication objects using data from the communication
 *          tables
 *
 * @param[inout] objects    Reference to an array of communication objects (must
 *                          not be @p NULL)
 * @param[in] obj_table     Communication objects table (from memory)
 * @param[in] count         Number of elements in @p objects
 */
void knx_com_object_update(knx_com_object_t *objects, const knx_table_com_objects_t *obj_table, int count);

/**
 * @brief   Get the size of a communication object
 *
 * @param[in] com_object    Reference to a communication object (must not be
 *                          @p NULL)
 *
 * @return  size of object (in bytes)
 */
size_t knx_com_object_size(const knx_com_object_t *com_object);

/**
 * @brief   Read the contents of a communication object into a buffer
 *
 * The buffer must be large enough to fit the requested data.
 *
 * @param[in] com_object    Reference to a communication object (must not be
 *                          @p NULL)
 * @param[out] buf          Destination buffer
 * @param[in] len           Size of the buffer
 *
 * @return -1 if @p buffer is NULL
 * @return -2 if @p len smaller than required
 * @return otherwise, the number of bytes copied
 */
ssize_t knx_com_object_read(const knx_com_object_t *com_object, void *buf, size_t len);

/**
 * @brief   Write the contents of a buffer to a communication object
 *
 * The buffer must be large enough to contain the data requested.
 *
 * @param[in] com_object    Reference to a communication object (must not be
 *                          @p NULL)
 * @param[out] buf          Destination buffer
 * @param[in] len           Size of the buffer
 *
 * @return -1 if @p buffer is NULL
 * @return -2 if @p len smaller than required
 * @return otherwise, the number of bytes copied
 */
ssize_t knx_com_object_write(knx_com_object_t *com_object, const void *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* KNX_DEVICE_SYSTEM7_COM_OBJECT_H */
/** @} */
