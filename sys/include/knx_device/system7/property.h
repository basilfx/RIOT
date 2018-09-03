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
 * @brief       KNX device property interface
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef KNX_DEVICE_SYSTEM7_PROPERTY_H
#define KNX_DEVICE_SYSTEM7_PROPERTY_H

#include "kernel_types.h"
#include "byteorder.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Definition of property types
 */
typedef enum {
    KNX_PROPERTY_TYPE_CONTROL = 0,
    KNX_PROPERTY_TYPE_CHAR = 1,
    KNX_PROPERTY_TYPE_UNSIGNED_CHAR = 2,
    KNX_PROPERTY_TYPE_INT = 3,
    KNX_PROPERTY_TYPE_UNSIGNED_INT = 4,
    KNX_PROPERTY_TYPE_KNX_FLOAT = 5,
    KNX_PROPERTY_TYPE_DATE = 6,
    KNX_PROPERTY_TYPE_TIME = 7,
    KNX_PROPERTY_TYPE_LONG = 8,
    KNX_PROPERTY_TYPE_UNSIGNED_LONG = 9,
    KNX_PROPERTY_TYPE_FLOAT = 10,
    KNX_PROPERTY_TYPE_DOUBLE = 11,
    KNX_PROPERTY_TYPE_CHAR_BLOCK = 12,
    KNX_PROPERTY_TYPE_POLL_GROUP_SETTINGS = 13,
    KNX_PROPERTY_TYPE_SHORT_CHAR_BLOCK = 14,
    KNX_PROPERTY_TYPE_DATE_TIME = 15,
    KNX_PROPERTY_TYPE_VARIABLE_LENGTH = 16,
    KNX_PROPERTY_TYPE_GENERIC1 = 17,
    KNX_PROPERTY_TYPE_GENERIC2 = 18,
    KNX_PROPERTY_TYPE_GENERIC3 = 19,
    KNX_PROPERTY_TYPE_GENERIC4 = 20,
    KNX_PROPERTY_TYPE_GENERIC5 = 21,
    KNX_PROPERTY_TYPE_GENERIC6 = 22,
    KNX_PROPERTY_TYPE_GENERIC7 = 23,
    KNX_PROPERTY_TYPE_GENERIC8 = 24,
    KNX_PROPERTY_TYPE_GENERIC9 = 25,
    KNX_PROPERTY_TYPE_GENERIC10 = 26,
    KNX_PROPERTY_TYPE_GENERIC11 = 27,
    KNX_PROPERTY_TYPE_GENERIC12 = 28,
    KNX_PROPERTY_TYPE_GENERIC13 = 29,
    KNX_PROPERTY_TYPE_GENERIC14 = 30,
    KNX_PROPERTY_TYPE_GENERIC15 = 31,
    KNX_PROPERTY_TYPE_GENERIC16 = 32,
    KNX_PROPERTY_TYPE_GENERIC17 = 33,
    KNX_PROPERTY_TYPE_GENERIC18 = 34,
    KNX_PROPERTY_TYPE_GENERIC19 = 35,
    KNX_PROPERTY_TYPE_GENERIC20 = 36,
    KNX_PROPERTY_TYPE_UTF8 = 47,
    KNX_PROPERTY_TYPE_VERSION = 48,
    KNX_PROPERTY_TYPE_ALARM_INFO = 49,
    KNX_PROPERTY_TYPE_BINARY_INFORMATION = 50,
    KNX_PROPERTY_TYPE_BITSET8 = 51,
    KNX_PROPERTY_TYPE_BITSET16 = 52,
    KNX_PROPERTY_TYPE_ENUM8 = 53,
    KNX_PROPERTY_TYPE_SCALING = 54,
    KNX_PROPERTY_TYPE_NE_VL = 60,
    KNX_PROPERTY_TYPE_NE_FL = 61,
    KNX_PROPERTY_TYPE_FUNCTION = 62,
    KNX_PROPERTY_TYPE_ESCAPE = 63,
} knx_property_type_t;

/**
 * @brief   Supported property flags
 */
enum {
    KNX_PROPERTY_FLAG_NONE = 0x00,          /**< No property flags */
    KNX_PROPERTY_FLAG_POINTER = 0x01,       /**< Property is a pointer */
    KNX_PROPERTY_FLAG_ARRAY = 0x02,         /**< Property is an array */
    KNX_PROPERTY_FLAG_WRITABLE = 0x04,      /**< Property is writeable */
    KNX_PROPERTY_FLAG_MODIFIED = 0x10       /**< Property is modified */
};

/**
 * @brief   Indicates the last property object in a list of property objects
 */
#define KNX_PROPERTY_OBJECT_END { .properties = NULL }

/**
 * @brief   Size of the value field of a property, for internal storing of a
 *          property size
 */
#ifndef KNX_PROPERTY_VALUE_SIZE
#define KNX_PROPERTY_VALUE_SIZE (4)
#endif

/**
 * @brief   Definition of a property.
 */
typedef struct {
    uint8_t id;
    knx_property_type_t type;
    uint8_t size;
    uint8_t flags;
    union {
        void *ptr;
        uint8_t value[KNX_PROPERTY_VALUE_SIZE];
    } content;
} knx_property_t;

/**
 * @brief   Definition of an object, which holds properties
 */
typedef struct {
    uint8_t count;
    knx_property_t *properties;
} knx_property_object_t;

/**
 * @brief   Find a property in a list of objects, given the object index and
 *          property identifier
 *
 * @param[in] objects       Array of objects
 * @param[in] objects       Object identifier
 * @param[in] id            Property identifier
 *
 * @return  pointer to property, if found
 * @return  @p NULL otherwise
 */
knx_property_t *knx_property_find_by_id(const knx_property_object_t *objects, uint8_t object, uint8_t id);

/**
 * @brief   Find a property in list of objects, given the object index and a
 *          property index
 *
 * @param[in] objects       Array of objects
 * @param[in] objects       Object identifier
 * @param[in] id            Property index
 *
 * @return  pointer to property, if found
 * @return  @p NULL otherwise
 */
knx_property_t *knx_property_find_by_index(const knx_property_object_t *objects, uint8_t object, uint8_t index);

/**
 * @brief   Get the size of a property
 *
 * @param[in] property      Reference to a property (must not be @p NULL)
 *
 * @return  size of object (in bytes)
 */
size_t knx_property_size(const knx_property_t *property);

/**
 * @brief   Get the of number of elements of a property
 *
 * This method returns one for any property that has not
 * @p KNX_PROPERTY_FLAG_ARRAY. For array properties, the number of elements is
 * its size property divided by the type size.
 *
 * @param[in] property      Reference to a property (must not be @p NULL)
 *
 * @return  number of elements of object
 */
size_t knx_property_elements(const knx_property_t *property);

/**
 * @brief   Read the contents of a property into a buffer
 *
 * The buffer must be large enough to fit the requested data.
 *
 * @param[in] property      Reference to a property (must not be @p NULL)
 * @param[in] count         Number of elements to copy (for arrays)
 * @param[in] start         Start element
 * @param[out] buf          Destination buffer
 * @param[in] len           Size of the buffer
 *
 * @return -1 if @p buffer is NULL
 * @return -2 if @p len smaller than required
 * @return otherwise, the number of bytes copied
 */
ssize_t knx_property_read(const knx_property_t *property, uint8_t count, uint8_t start, uint8_t *buf, size_t len);

/**
 * @brief   Write the contents of a buffer to a property
 *
 * The buffer must be large enough to contain the data requested.
 *
 * @param[in] property      Reference to a property (must not be @p NULL)
 * @param[in] count         Number of elements to copy (for arrays)
 * @param[in] start         Start element
 * @param[out] buf          Destination buffer
 * @param[in] len           Size of the buffer
 *
 * @return -1 if @p buffer is NULL
 * @return -2 if @p len smaller than required
 * @return -3 if @p property is not writable (@p KNX_PROPERTY_FLAG_WRITABLE)
 * @return otherwise, the number of bytes copied
 */
ssize_t knx_property_write(knx_property_t *property, uint8_t count, uint8_t start, const uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* KNX_DEVICE_SYSTEM7_PROPERTY_H */
/** @} */
