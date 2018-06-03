/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
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
 * @see         KNX Specification version 2.1, Datapoint Types v01.08.02
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
 * @brief   KNX data point types (DPT).
 *
 * These types are the main types only. There are many more subtypes.
 */
typedef enum {
    KNX_DPT_1,      /**< 1-bit */
    KNX_DPT_2,      /**< 1-bit controlled */
    KNX_DPT_3,      /**< 3-bit controlled */
    KNX_DPT_4,      /**< character */
    KNX_DPT_5,      /**< 8-bit unsigned value */
    KNX_DPT_6,      /**< 8-bit signed value */
    KNX_DPT_7,      /**< 2-byte unsigned value */
    KNX_DPT_8,      /**< 2-byte signed value */
    KNX_DPT_9,      /**< 2-byte float value */
    KNX_DPT_10,     /**< time */
    KNX_DPT_11,     /**< date */
    KNX_DPT_12,     /**< 4-byte unsigned value */
    KNX_DPT_13,     /**< 4-byte signed value */
    KNX_DPT_14,     /**< 4-byte float value */
    KNX_DPT_15,     /**< entrance access */
    KNX_DPT_16,     /**< character string */
    KNX_DPT_17,     /**< scene number */
    KNX_DPT_18,     /**< scene control */
    KNX_DPT_19,     /**< date and time */
    KNX_DPT_20,     /**< 1-byte */
    KNX_DPT_21,     /**< 8-bit set */
    KNX_DPT_22,     /**< 16-bit set */
    KNX_DPT_23,     /**< 2-bit set */
    KNX_DPT_25,     /**< 2-nibble set */
    KNX_DPT_26,     /**< 8-bit set */
    KNX_DPT_27,     /**< 32-bit set */
    KNX_DPT_29,     /**< electrical energy */
    KNX_DPT_30,     /**< 24 times channel activation */
    KNX_DPT_206,    /**< 16-bit unsigned value and 8-bit enum */
    KNX_DPT_217,    /**< datapoint type version */
    KNX_DPT_219,    /**< alarm info */
    KNX_DPT_222,    /**< 3x 2-byte float value */
    KNX_DPT_225,    /**< scaling speed */
    KNX_DPT_229,    /**< 4-1-1 byte combined information */
    KNX_DPT_230,    /**< MBus address */
    KNX_DPT_232,    /**< 3-byte RGB color */
    KNX_DPT_234,    /**< language code ISO 639-1 */
    KNX_DPT_235,    /**< signed value with classification and validity */
    KNX_DPT_237,    /**< configuration or diagnostics */
    KNX_DPT_238,    /**< configuration or diagnostics */
    KNX_DPT_240,    /**< positions */
    KNX_DPT_241,    /**< status */
    KNX_DPT_244,    /**< converter status */
    KNX_DPT_245,    /**< converter test result */
    KNX_DPT_249,    /**< brightness color temperature transition */
    KNX_DPT_250,    /**< status */
    KNX_DPT_251,    /**< RGBW color */
    KNX_DPT_NUMOF
} knx_dpt_t;

/**
 * @brief   Get the size of a DPT in bytes
 *
 * @param[in] type          DPT enum value
 *
 * @return  size of DPT (in bytes)
 */
size_t knx_dpt_size(knx_dpt_t type);

/**
 * @brief   Get the size of a DPT in bits
 *
 * @param[in] type          DPT enum value
 *
 * @return  size of DPT (in bits)
 */
size_t knx_dpt_size_bits(knx_dpt_t type);

/**
 * @brief   Encode a float value into a 2-byte DPT9.xxx value
 *
 * @param[in] number        The number to encode
 * @param[out] out          2-byte buffer to write to (must not be NULL)
 */
void knx_dpt_to_dpt9xxx(float number, uint8_t *out);

/**
 * @brief   Encode a float value into a 4-byte DPT14.xxx
 *
 * @note    Only works for architectures that support IEE-754 floats.
 *
 * @param[in] number        The number to encode
 * @param[out] out          4-byte buffer to write to (must not be NULL)
 */
void knx_dpt_to_dpt14xxx(float number, uint8_t *out);

/**
 * @brief   Decode a 4-byte DPT14.xxx into a float value
 *
 * @note    Only works for architectures that support IEE-754 floats.
 *
 * @param[in] number        4-byte buffer to read from (must not be NULL)
 *
 * @return  float value
 */
float knx_dpt_from_dpt14xxx(uint8_t *out);

/**
 * @brief   Encode a time structure into a 6-byte DPT19.xxx
 *
 * @param[in] in            Time structure (must not be NULL)
 * @param[out] out          6-byte buffer to write to (must not be NULL)
 */
void knx_dpt_to_dpt19xxx(const struct tm *in, uint8_t *out);

/**
 * @brief   Decode a 6-byte DPT19.xxx into a time structure
 *
 * @param[in] in            6-byte buffer to read from (must not be NULL)
 * @param[out] out          Time structure to write to (must not be NULL)
 */
void knx_dpt_from_dpt19xxx(const uint8_t *in, struct tm *out);

#ifdef __cplusplus
}
#endif

#endif /* NET_KNX_DPT_H */
/** @} */
