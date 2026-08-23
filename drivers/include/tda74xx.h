/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @defgroup    drivers_tda74xx ST TDA74xx audio processors
 * @ingroup     drivers_multimedia
 * @brief       Driver for the ST TDA74xx digitally controlled audio processors
 *
 * The ST TDA74xx are digitally controlled stereo audio processors. They
 * multiplex two or four stereo inputs into a single stereo output, and offer
 * an input gain stage, a volume stage, tone control and a speaker attenuator
 * per output channel.
 *
 * The supported audio processors are:
 *
 * * TDA7439: 4 stereo inputs, bass, middle and treble control
 * * TDA7440: 4 stereo inputs, bass and treble control
 * * TDA7449: 2 stereo inputs, bass and treble control
 *
 * The TDA7439D, TDA7439DS and TDA7440D are variants of the above in a
 * different package, and are supported by the module of the part they are
 * named after.
 *
 * All settings are expressed in decibels. Because the devices only support
 * discrete steps, values are rounded towards zero to the nearest supported
 * step. The devices are write-only, therefore the driver keeps track of the
 * configured volume to be able to restore it after unmuting.
 *
 * After initialization a device is configured to a known state: the first
 * input is selected, the input gain is 0 dB, the tone control is flat, the
 * speaker attenuators are 0 dB and the volume is muted. Therefore, the volume
 * must be set (or the device must be unmuted) before any sound is produced.
 *
 * @{
 *
 * @file
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#include <stdbool.h>
#include <stdint.h>

#include "periph/i2c.h"

#include "tda74xx_info.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Compile time macro to enable/disable features
 *
 * Middle band support is only enabled if a module is selected that supports
 * it.
 */
#if MODULE_TDA7439
#  define TDA74XX_HAS_MIDDLE 1
#endif

/**
 * @name    Supported setting ranges (in dB)
 * @{
 */
#define TDA74XX_INPUT_GAIN_MIN      (0)     /**< Minimum input gain */
#define TDA74XX_INPUT_GAIN_MAX      (30)    /**< Maximum input gain */
#define TDA74XX_INPUT_GAIN_STEP     (2)     /**< Input gain step size */

#define TDA74XX_VOLUME_MIN          (-47)   /**< Minimum volume */
#define TDA74XX_VOLUME_MAX          (0)     /**< Maximum volume */
#define TDA74XX_VOLUME_STEP         (1)     /**< Volume step size */

#define TDA74XX_TONE_MIN            (-14)   /**< Minimum tone gain */
#define TDA74XX_TONE_MAX            (14)    /**< Maximum tone gain */
#define TDA74XX_TONE_STEP           (2)     /**< Tone gain step size */

#define TDA74XX_ATTENUATION_MIN     (-79)   /**< Minimum speaker attenuation */
#define TDA74XX_ATTENUATION_MAX     (0)     /**< Maximum speaker attenuation */
#define TDA74XX_ATTENUATION_STEP    (1)     /**< Speaker attenuation step size */
/** @} */

/**
 * @brief   Input values
 *
 * Devices with two stereo inputs only support the first two inputs.
 */
typedef enum {
    TDA74XX_INPUT_1 = 0,            /**< Input 1 */
    TDA74XX_INPUT_2 = 1,            /**< Input 2 */
    TDA74XX_INPUT_3 = 2,            /**< Input 3 */
    TDA74XX_INPUT_4 = 3             /**< Input 4 */
} tda74xx_input_t;

/**
 * @brief   Speaker values
 */
typedef enum {
    TDA74XX_SPEAKER_RIGHT   = 0,    /**< Right speaker */
    TDA74XX_SPEAKER_LEFT    = 1,    /**< Left speaker */
    TDA74XX_SPEAKER_BOTH    = 2     /**< Both speakers */
} tda74xx_speaker_t;

/**
 * @brief   Device initialization parameters
 */
typedef struct {
    i2c_t i2c_dev;                  /**< I2C bus the audio processor is connected to */
    uint8_t address;                /**< audio processor address */
    const tda74xx_info_t *info;     /**< device information */
} tda74xx_params_t;

/**
 * @brief   Device descriptor for the driver
 */
typedef struct {
    tda74xx_params_t params;    /**< device parameters */
    int8_t volume;              /**< configured volume (in dB) */
    bool muted;                 /**< whether the volume stage is muted */
} tda74xx_t;

/**
 * @brief   Initialize the given device
 *
 * The driver will configure the device to a known state, with the volume
 * muted.
 *
 * @param[in,out] dev       Device descriptor of the driver
 * @param[in]     params    Initialization parameters
 *
 * @retval                  0 on success
 * @retval                  -ENODEV if no device is found
 */
int tda74xx_init(tda74xx_t *dev, const tda74xx_params_t *params);

/**
 * @brief   Select the input
 *
 * @param[in] dev           Device descriptor of the driver
 * @param[in] input         Input to select
 *
 * @retval                  0 on success
 * @retval                  -ERANGE if the device does not have the input
 * @retval                  -EIO on I2C error
 */
int tda74xx_set_input(tda74xx_t *dev, tda74xx_input_t input);

/**
 * @brief   Set the gain of the input stage
 *
 * @param[in] dev           Device descriptor of the driver
 * @param[in] gain          Gain to set (in dB, see @ref TDA74XX_INPUT_GAIN_MIN
 *                          and @ref TDA74XX_INPUT_GAIN_MAX)
 *
 * @retval                  0 on success
 * @retval                  -ERANGE if gain is out of range
 * @retval                  -EIO on I2C error
 */
int tda74xx_set_input_gain(tda74xx_t *dev, int8_t gain);

/**
 * @brief   Set the volume
 *
 * Setting the volume clears the mute state.
 *
 * @param[in] dev           Device descriptor of the driver
 * @param[in] volume        Volume to set (in dB, see @ref TDA74XX_VOLUME_MIN
 *                          and @ref TDA74XX_VOLUME_MAX)
 *
 * @retval                  0 on success
 * @retval                  -ERANGE if volume is out of range
 * @retval                  -EIO on I2C error
 */
int tda74xx_set_volume(tda74xx_t *dev, int8_t volume);

/**
 * @brief   Mute or unmute the volume stage
 *
 * Unmuting restores the volume that was configured last.
 *
 * @param[in] dev           Device descriptor of the driver
 * @param[in] mute          True to mute, false to unmute
 *
 * @retval                  0 on success
 * @retval                  -EIO on I2C error
 */
int tda74xx_set_mute(tda74xx_t *dev, bool mute);

/**
 * @brief   Set the gain of the bass filter
 *
 * @param[in] dev           Device descriptor of the driver
 * @param[in] gain          Gain to set (in dB, see @ref TDA74XX_TONE_MIN and
 *                          @ref TDA74XX_TONE_MAX)
 *
 * @retval                  0 on success
 * @retval                  -ERANGE if gain is out of range
 * @retval                  -EIO on I2C error
 */
int tda74xx_set_bass(tda74xx_t *dev, int8_t gain);

#if TDA74XX_HAS_MIDDLE || DOXYGEN
/**
 * @brief   Set the gain of the middle filter
 *
 * Only devices with three tone control bands have a middle filter.
 *
 * @param[in] dev           Device descriptor of the driver
 * @param[in] gain          Gain to set (in dB, see @ref TDA74XX_TONE_MIN and
 *                          @ref TDA74XX_TONE_MAX)
 *
 * @retval                  0 on success
 * @retval                  -ERANGE if gain is out of range
 * @retval                  -EIO on I2C error
 */
int tda74xx_set_middle(tda74xx_t *dev, int8_t gain);
#endif /* TDA74XX_HAS_MIDDLE || DOXYGEN */

/**
 * @brief   Set the gain of the treble filter
 *
 * @param[in] dev           Device descriptor of the driver
 * @param[in] gain          Gain to set (in dB, see @ref TDA74XX_TONE_MIN and
 *                          @ref TDA74XX_TONE_MAX)
 *
 * @retval                  0 on success
 * @retval                  -ERANGE if gain is out of range
 * @retval                  -EIO on I2C error
 */
int tda74xx_set_treble(tda74xx_t *dev, int8_t gain);

/**
 * @brief   Set the attenuation of a speaker
 *
 * The speaker attenuators are intended for balance control, in addition to the
 * volume stage.
 *
 * @param[in] dev           Device descriptor of the driver
 * @param[in] speaker       Speaker to set
 * @param[in] attenuation   Attenuation to set (in dB, see
 *                          @ref TDA74XX_ATTENUATION_MIN and
 *                          @ref TDA74XX_ATTENUATION_MAX)
 *
 * @retval                  0 on success
 * @retval                  -ERANGE if attenuation is out of range
 * @retval                  -EIO on I2C error
 */
int tda74xx_set_attenuation(tda74xx_t *dev, tda74xx_speaker_t speaker,
                            int8_t attenuation);

#ifdef __cplusplus
}
#endif

/** @} */
