/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_knx_addr    KNX addresses
 * @ingroup     net_knx
 * @brief       KNX address architecture
 *
 * @{
 *
 * @file
 * @brief       Definitions for KNX addresses
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NET_KNX_ADDR_H
#define NET_KNX_ADDR_H

#include <stdint.h>
#include <stdbool.h>

#include "byteorder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Length of a KNX address in bytes
 */
#define KNX_ADDR_LEN                (2U)

/**
 * @brief   Maximum length of a KNX address as string
 */
#define KNX_ADDR_MAX_STR_LEN        (10U)

/**
 * @brief   KNX broadcast address (to be used with @p knx_addr_t)
 */
#define KNX_ADDR_BROADCAST          { { 0x00, 0x00 } }

/**
 * @brief   KNX undefined address (to be used with @p knx_addr_t)
 */
#define KNX_ADDR_UNDEFINED          { { 0xFF, 0xFF } }

/**
 * @brief   Data type to represent a KNX address
 */
typedef union {
    uint8_t u8[2];              /**< divided by two 8-bit words. */
    network_uint16_t u16;       /**< divided by one 16-bit words. */
} knx_addr_t;

/**
 * @brief   Data type to represent a KNX physical address
 */
typedef knx_addr_t knx_addr_physical_t;

/**
 * @brief   Data type to represent a KNX group address
 */
typedef knx_addr_t knx_addr_group_t;

/**
 * @brief   Definition of the KNX broadcast address
 */
extern const knx_addr_t knx_addr_broadcast;

/**
 * @brief   Definition of the KNX undefined address
 */
extern const knx_addr_t knx_addr_undefined;

/**
 * @brief   Checks if two KNX addresses are equal
 *
 * @param[in] a     A KNX address
 * @param[in] b     Another KNX address
 *
 * @return  true, if @p a and @p b are equal
 * @return  false, otherwise.
 */
bool knx_addr_equal(const knx_addr_t *a, const knx_addr_t *b);

/**
 * @brief   Compare two KNX addresses
 *
 * @param[in] a     A KXN address
 * @param[in] b     Another KNX address
 *
 * @return  0 if @a and @b are equal
 * @return  <0 if @p a is smaller than @p b
 * @return  >0 if @p a is bigger than @p b
 */
int knx_addr_compare(const knx_addr_t *a, const knx_addr_t *b);

/**
 * @brief   Convert an area, line and device number into a byte-represented
 *          KNX physical address
 *
 * @param[out] result       The resulting byte representation
 * @param[in] area          Area number (less than 16)
 * @param[in] line          Line number (less than 16)
 * @param[in] device        Device number
 *
 * @return  @p result, on success
 * @return  NULL, if @p area or @p line were invalid
 * @return  NULL, if @p result was NULL
 */
knx_addr_physical_t *knx_addr_physical(knx_addr_physical_t *result, uint8_t area, uint8_t line, uint8_t device);

/**
 * @brief   Convert a main, mid and sub number into a byte-represented KNX
 *          group address
 *
 * @param[out] result       The resulting byte representation
 * @param[in] main          Main number (less than 32)
 * @param[in] mid           Mid number (less than 8)
 * @param[in] sub           Sub number
 *
 * @return  @p result, on success
 * @return  NULL, if @p main or @p mid were invalid
 * @return  NULL, if @p result was NULL
 */
knx_addr_group_t *knx_addr_group(knx_addr_group_t *result, uint8_t main, uint8_t mid, uint8_t sub);

/**
 * @brief   Convert a main and sub number into a byte-represented KNX group
 *          address
 *
 * @param[out] result       The resulting byte representation
 * @param[in] main          Area number (less than 32)
 * @param[in] sub           Device number (less than 2048)
 *
 * @return  @p result, on success
 * @return  NULL, if @p main or @p sub were invalid
 * @return  NULL, if @p result was NULL
 */
knx_addr_group_t *knx_addr_group2(knx_addr_group_t *result, uint8_t main, uint16_t sub);

/**
 * @brief   Convert a string representation of a KNX physical address (x.y.z)
 *          into a byte-representation
 *
 * @param[out] result       The resulting byte representation
 * @param[in] addr          The input address string
 *
 * @return  @p result, on success
 * @return  NULL, if @p result was NULL
 * @return  NULL, if @p addr was NULL
 */
knx_addr_physical_t *knx_addr_physical_from_str(knx_addr_physical_t *result, const char *addr);

/**
 * @brief   Convert a string representation of a KNX group address (x/y/z)
 *          into a byte-representation
 *
 * @param[out] result       The resulting byte representation
 * @param[in] addr          The input address string
 *
 * @return  @p result, on success
 * @return  NULL, if @p result was NULL
 * @return  NULL, if @p addr was NULL
 */
knx_addr_group_t *knx_addr_group_from_str(knx_addr_group_t *result, const char *addr);

/**
 * @brief   Convert a string representation of a KNX group address (x/y)
 *          into a byte-representation
 *
 * @param[out] result       The resulting byte representation
 * @param[in] addr          The input address string
 *
 * @return  @p result, on success
 * @return  NULL, if @p result was NULL
 * @return  NULL, if @p addr was NULL
 */
knx_addr_group_t *knx_addr_group2_from_str(knx_addr_group_t *result, const char *addr);

/**
 * @brief   Convert a byte-representation of a physical address into a string
 *          representation (x.y.z)
 *
 * @param[out] result       The resulting string representation
 * @param[in] addr          The input address
 * @param[in] len           Length of the result buffer
 *
 * @return  @p result, on success
 * @return  NULL, if @p result was NULL
 * @return  NULL, if @p addr was NULL
 * @return  NULL, if @p len was less than @p KNX_ADDR_MAX_STR_LEN.
 */
char *knx_addr_physical_to_str(char *result, const knx_addr_t *addr, uint8_t len);

/**
 * @brief   Convert a byte-representation of a group address into a string
 *          representation (x/y/z)
 *
 * @param[out] result       The resulting string representation
 * @param[in] addr          The input address
 * @param[in] len           Length of the result buffer
 *
 * @return  @p result, on success
 * @return  NULL, if @p result was NULL
 * @return  NULL, if @p addr was NULL
 * @return  NULL, if @p len was less than @p KNX_ADDR_MAX_STR_LEN.
 */
char *knx_addr_group_to_str(char *result, const knx_addr_group_t *addr, uint8_t len);

/**
 * @brief   Convert a byte-representation of a group address into a string
 *          representation (x/y)
 *
 * @param[out] result       The resulting string representation
 * @param[in] addr          The input address
 * @param[in] len           Length of the result buffer
 *
 * @return  @p result, on success
 * @return  NULL, if @p result was NULL
 * @return  NULL, if @p addr was NULL
 * @return  NULL, if @p len less than @p KNX_ADDR_MAX_STR_LEN.
 */
char *knx_addr_group2_to_str(char *result, const knx_addr_group_t *addr, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* NET_KNX_ADDR_H */
/** @} */
