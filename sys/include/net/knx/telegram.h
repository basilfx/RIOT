/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_knx_telegram    KNX telegram
 * @ingroup     net_knx
 * @brief       KNX telegram architecture
 *
 * @{
 *
 * @file
 * @brief       Definitions for KNX telegrams
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NET_KNX_TELEGRAM_H
#define NET_KNX_TELEGRAM_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "net/knx/addr.h"

#include "byteorder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Type definition of a KNX telegram
 *
 * @note    It is assumed that the length of the telegram is at least
 *          @p KNX_TELEGRAM_MIN_LEN bytes.
 */
typedef uint8_t knx_telegram_t;

/**
 * @brief   Minimum length of a standard telegram (including checksum)
 */
#define KNX_TELEGRAM_TYPE_STANDARD_MIN_LEN      (8U)

/**
 * @brief   Maximum length of a standard telegram
 */
#define KNX_TELEGRAM_TYPE_STANDARD_MAX_LEN      (23U)

/**
 * @brief   Minimum length of an extended telegram (including checksum)
 */
#define KNX_TELEGRAM_TYPE_EXTENDED_MIN_LEN      (9U)

/**
 * @brief   Maximum length of an extended telegram
 */
#define KNX_TELEGRAM_TYPE_EXTENDED_MAX_LEN      (263U)

/**
 * @brief   Minimum length of a poll telegram (including checksum)
 */
#define KNX_TELEGRAM_TYPE_POLL_MIN_LEN          (7U)

/**
 * @brief   Maximum length of a poll telegram
 */
#define KNX_TELEGRAM_TYPE_POLL_MAX_LEN          (7U)

/**
 * @brief   Minimum length of a supported telegram
 */
#define KNX_TELEGRAM_MIN_LEN    KNX_TELEGRAM_TYPE_POLL_MIN_LEN

/**
 * @brief   Maximum length of a supported telegram
 */
#define KNX_TELEGRAM_MAX_LEN    KNX_TELEGRAM_TYPE_EXTENDED_MAX_LEN

/**
 * @brief   Telegram control field masks
 * @{
 */
#define KNX_TELEGRAM_CONTROL_DATA_REQUEST   (0x90)
#define KNX_TELEGRAM_CONTROL_EDATA_REQUEST  (0x10)
#define KNX_TELEGRAM_CONTROL_POLL_REQUEST   (0xf0)
/** @} */

/**
 * @brief   Supported telegram types
 */
typedef enum {
    KNX_TELEGRAM_TYPE_STANDARD,     /**< standard telegram type */
    KNX_TELEGRAM_TYPE_EXTENDED,     /**< extended telegram type */
    KNX_TELEGRAM_TYPE_POLL,         /**< poll telegram type */
    KNX_TELEGRAM_TYPE_UNKNOWN       /**< unknown telegram type */
} knx_telegram_type_t;

/**
 * @brief   Telegram priorities
 */
typedef enum {
    KNX_TELEGRAM_PRIORITY_LOW       = 0x03, /**< low priority */
    KNX_TELEGRAM_PRIORITY_ALARM     = 0x02, /**< alarm priority */
    KNX_TELEGRAM_PRIORITY_HIGH      = 0x01, /**< high priority */
    KNX_TELEGRAM_PRIORITY_SYSTEM    = 0x00  /**< system priority */
} knx_telegram_priority_t;

/**
 * @brief   Check whether a given buffer represents a supported telegram
 *
 * This method can be used to determine if the buffer can be safely casted to
 * a `knx_telegram_t`.
 *
 * @param[in] telegram      The buffer to check
 * @param[in] length        Length of the buffer
 *
 * @return  true, if buffer is a (supported) telegram
 * @return  false, if the buffer is not a (supported) telegram
 */
bool knx_telegram_is(const uint8_t *telegram, size_t len);

/**
 * @brief   Calculate the checksum of the telegram.
 *
 * This will also include the checksum byte, so use `len - 1` for only the data
 * part of the telegram.
 *
 * @param[in] telegram      The telegram (must not be NULL)
 * @param[in] len           Length of the telegram
 *
 * @return  Calculated checksum value
 */
uint8_t knx_telegram_checksum(const knx_telegram_t *telegram, size_t len);

/**
 * @brief   Retrieve the checksum byte
 *
 * @param[in] telegram      The telegram (must not be NULL)
 * @param[in] len           Length of the telegram
 *
 * @return  Checksum value
 */
uint8_t knx_telegram_get_checksum(const knx_telegram_t *telegram, size_t len);

/**
 * @brief   Update the checksum byte value
 *
 * @param[out] telegram     The telegram (must not be NULL)
 * @param[in] len           Length of the telegram
 * @param[in] checksum      Checksum value
 */
void knx_telegram_set_checksum(knx_telegram_t *telegram, size_t len, uint8_t checksum);

/**
 * @brief   Calculate and set the checksum for a complete telegram
 *
 * The telegram includes an additional byte for the checksum value.
 *
 * @param[out] telegram     The telegram (must not be NULL)
 * @param[in] length        Length of the telegram (including checksum byte)
 */
void knx_telegram_update_checksum(knx_telegram_t *telegram, size_t len);

/**
 * @brief   Returns true if the checksum of a telegram is valid
 *
 * @param[in] telegram      The telegram (must not be NULL)
 * @param[in] length        Length of the telegram (including checksum byte)
 *
 * @return  true, if valid
 * @return  false, if invalid
 */
bool knx_telegram_is_checksum_valid(const knx_telegram_t *telegram, size_t len);

/**
 * @brief   Get the telegram type
 *
 * Only telegram types defined in @p knx_telegram_type_t are supported.
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  @p knx_telegram_type_t telegram type
 * @return  @p KNX_TELEGRAM_TYPE_UNKNOWN if unsupported telegram type
 */
knx_telegram_type_t knx_telegram_get_type(const knx_telegram_t *telegram);

/**
 * @brief   Set the telegram type
 *
 * Only telegram types defined in @p knx_telegram_type_t are supported.
 *
 * @param[in] telegram      The telegram (must not be NULL)
 */
void knx_telegram_set_type(knx_telegram_t *telegram, knx_telegram_type_t type);

/**
 * @brief   Get the source address of a telegram
 *
 * @param[in] telegram      The telegram (must not be NULL)
 * @param[out] result       Resulting address
 *
 * @return  @p result if successfully read
 * @return  @c NULL if @p result is @c NULL or telegram type not supported
 */
knx_addr_t *knx_telegram_get_src_addr(const knx_telegram_t *telegram, knx_addr_t *result);

/**
 * @brief   Set the source address of a telegram
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] addr          Source address
 */
void knx_telegram_set_src_addr(knx_telegram_t *telegram, const knx_addr_t *addr);

/**
 * @brief   Get the destination address of a telegram
 *
 * @param[in] telegram      The telegram (must not be NULL)
 * @param[out] result       Resulting address
 *
 * @return  @p result if successfully read
 * @return  @c NULL if @p result is @c NULL or telegram type not supported
 */
knx_addr_t *knx_telegram_get_dst_addr(const knx_telegram_t *telegram, knx_addr_t *result);

/**
 * @brief   Set the destination address of a telegram
 *
 * @note    This does not set the address type bits (@see
 *          knx_telegram_set_group_addressed)
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] addr          Source address
 */
void knx_telegram_set_dst_addr(knx_telegram_t *telegram, const knx_addr_t *addr);

/**
 * @brief   Returns true when the telegram is group addressed
 *
 * @note    Only applies to standard or extended telegram types, and returns
 *          false for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  @p true if group addressed
 * @return  @p false if physical addressed
 */
bool knx_telegram_is_group_addressed(const knx_telegram_t *telegram);

/**
 * @brief   Mark telegram as group addressed
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[inout] telegram       The telegram (must not be NULL)
 * @param[in] group_addressed   Whether group addressed or not
 */
void knx_telegram_set_group_addressed(knx_telegram_t *telegram, bool group_addressed);

/**
 * @brief   Return a pointer to the payload of a telegram
 *
 * @note    Only applies to standard or extended telegram types, and returns
 *          NULL for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 * @param[in] merged        If true, start at the last telegram header byte
 *
 * @return  Pointer to the first byte of the payload of the telegram
 */
uint8_t *knx_telegram_get_payload(knx_telegram_t *telegram, bool merged);

/**
 * @brief   Get the length of the payload of a telegram
 *
 * @note    Only applies to standard or extended telegram types, and returns 0
 *          for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  Length of the payload of the telegram
 */
size_t knx_telegram_get_payload_length(const knx_telegram_t *telegram);

/**
 * @brief   Set the length of the payload of a telegram
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] len           Length of the payload
 */
void knx_telegram_set_payload_length(knx_telegram_t *telegram, size_t len);

/**
 * @brief   Get the routing count of a telegram
 *
 * @note    Only applies to standard or extended telegram types, and returns 0
 *          for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  The routing count
 */
uint8_t knx_telegram_get_routing_count(const knx_telegram_t *telegram);

/**
 * @brief   Set the routing count of a telegram
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] count         The routing count (<= 7)
 */
void knx_telegram_set_routing_count(knx_telegram_t *telegram, uint8_t count);

/**
 * @brief   Return the priority of a telegram
 *
 * @note    Only applies to standard or extended telegram types, and returns
 *          KNX_TELEGRAM_PRIORITY_LOW for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  A @p knx_telegram_priority_t value
 */
knx_telegram_priority_t knx_telegram_get_priority(const knx_telegram_t *telegram);

/**
 * @brief   Set the priority of a telegram
 *
 * @note    Only applies to standard or extended telegram types, and is a
 *          no-op for non-supported telegrams
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] priority      Telegram priority
 */
void knx_telegram_set_priority(knx_telegram_t *telegram, knx_telegram_priority_t priority);

/**
 * @brief   Returns true if the telegram is repeated
 *
 * @note    Only applies to standard or extended telegram types, and returns
 *          false for non-supported telegrams
 *
 * @param[in] telegram      The telegram (must not be NULL)
 *
 * @return  true, if telegram is repeated
 * @return  false, if telegram is not repeateds
 */
bool knx_telegram_is_repeated(const knx_telegram_t *telegram);

/**
 * @brief   Set the repeated property
 *
 * @note    Only applies to standard or extended telegram types
 *
 * @param[inout] telegram   The telegram (must not be NULL)
 * @param[in] repeated      True if the telegram is repeated
 */
void knx_telegram_set_repeated(knx_telegram_t *telegram, bool repeated);

/**
 * @brief   Construct a telegram
 *
 * @note    Not all arguments apply to all telegram types
 *
 * @param[in,out] telegram      The telegram (must not be NULL)
 * @param[in] type              Type of the telegram
 * @param[in] src               Telegram source address
 * @param[in] dest              Telegram destination address
 * @param[in] group_addressed   If true, mark telegram as group addressed
 */
void knx_telegram_build(knx_telegram_t *telegram, knx_telegram_type_t type, const knx_addr_t *src, const knx_addr_t *dest, bool group_addressed);

/**
 * @brief   Dump telegram information to stdout
 *
 * @param[in] telegram      The telegram (must not be NULL)
 * @param[in] len           Length of the telegram
 */
void knx_telegram_print(const knx_telegram_t *telegram, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* NET_KNX_TELEGRAM_H */
/** @} */
