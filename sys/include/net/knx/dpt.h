/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_knx_dpt    KNX data point types
 * @ingroup     net_knx
 * @brief       KNX data point types
 *
 * @{
 *
 * @file
 * @brief       Definitions for KNX data point types
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NET_KNX_DPT_H
#define NET_KNX_DPT_H

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Encode a float value into a DPT9.xxx 16-bit float
 *
 * @param[in] number        The number to encode
 * @param[out] out          16-bit buffer to write to (must not be NULL)
 */
void knx_dpt_to_dpt9xxx(float number, uint16_t *out);

/**
 * @brief   Encode a float value into a DPT14.xxx 32-bit float
 *
 * @param[in] number        The number to encode
 * @param[out] out          32-bit buffer to write to (must not be NULL)
 */
void knx_dpt_to_dpt14xxx(float number, uint32_t *out);

/**
 * @brief   Decode a DPT19.xxx 6-byte value into a time structure
 *
 * @param[in] number        6-byte value (must not be NULL)
 * @param[out] out          Time structure to write to (must not be NULL)
 */
void knx_dpt_from_dpt19xxx(const uint8_t *in, struct tm *out);

#ifdef __cplusplus
}
#endif

#endif /* NET_KNX_DPT_H */
/** @} */
