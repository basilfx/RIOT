/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     drivers_tda74xx
 * @{
 *
 * @file
 * @brief       Device driver implementation for the tda74xx
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <assert.h>
#include <errno.h>

#include "tda74xx.h"
#include "tda74xx_constants.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#define TDA74XX_I2C     (dev->params.i2c_dev)
#define TDA74XX_ADDR    (dev->params.address)
#define TDA74XX_INFO    (dev->params.info)

/**
 * @brief   Write one or more registers
 *
 * To write more than one register, @p reg must have the increment flag set.
 */
static int _write(const tda74xx_t *dev, uint8_t reg, const void *values, size_t len)
{
    i2c_acquire(TDA74XX_I2C);

    int res = i2c_write_regs(TDA74XX_I2C, TDA74XX_ADDR, reg, values, len, 0);

    i2c_release(TDA74XX_I2C);

    if (res != 0) {
        DEBUG("[tda74xx] _write: writing register 0x%02x failed\n", reg);
        return -EIO;
    }

    return 0;
}

/**
 * @brief   Convert an input into a register value
 *
 * The input multiplexer numbers its inputs in reverse order.
 */
static uint8_t _input_value(tda74xx_input_t input)
{
    return TDA74XX_INPUT_4 - input;
}

/**
 * @brief   Convert an attenuation (in dB) into a register value
 *
 * The volume register and the speaker attenuation registers are split into a
 * coarse part in steps of 8 dB and a fine part in steps of 1 dB. Because the
 * coarse part starts at bit three, the register value is numerically equal to
 * the attenuation in dB.
 */
static uint8_t _attenuation_value(int8_t attenuation)
{
    return (uint8_t)(-attenuation);
}

/**
 * @brief   Convert a tone gain (in dB) into a register value
 */
static uint8_t _tone_value(int8_t gain)
{
    /* the devices only support even gains, therefore round towards zero */
    gain = (gain / TDA74XX_TONE_STEP) * TDA74XX_TONE_STEP;

    if (gain <= 0) {
        return (gain - TDA74XX_TONE_MIN) / TDA74XX_TONE_STEP;
    }

    return TDA74XX_TONE_BOOST | ((TDA74XX_TONE_MAX - gain) / TDA74XX_TONE_STEP);
}

/**
 * @brief   Set the gain of one of the tone filters
 */
static int _set_tone(tda74xx_t *dev, uint8_t reg, int8_t gain)
{
    if (gain < TDA74XX_TONE_MIN || gain > TDA74XX_TONE_MAX) {
        DEBUG("[tda74xx] _set_tone: gain out of range\n");
        return -ERANGE;
    }

    uint8_t value = _tone_value(gain);

    return _write(dev, reg, &value, sizeof(value));
}

int tda74xx_init(tda74xx_t *dev, const tda74xx_params_t *params)
{
    /* initialize the device descriptor */
    dev->params = *params;
    dev->volume = TDA74XX_VOLUME_MAX;
    dev->muted = true;

    /* configure the input and volume stages to a known state */
    const uint8_t stages[] = {
        _input_value(TDA74XX_INPUT_1),  /* input selection */
        0,                              /* input gain (0 dB) */
        TDA74XX_VOLUME_MUTE             /* volume (muted) */
    };

    if (_write(dev, TDA74XX_REG_INPUT_SELECT | TDA74XX_REG_INCREMENT, stages,
               sizeof(stages)) != 0) {
        DEBUG("[tda74xx] tda74xx_init: init failed\n");
        return -ENODEV;
    }

    /* configure the tone control and the speaker attenuators to a known state.
     * these registers are adjacent and always end at the left speaker
     * register, therefore the number of registers to write follows from the
     * register of the bass filter. devices without a middle filter still
     * expect the register in between to be written while incrementing */
    const uint8_t filters[] = {
        TDA74XX_TONE_FLAT,  /* bass gain (0 dB) */
        TDA74XX_TONE_FLAT,  /* middle gain (0 dB), or without a function */
        TDA74XX_TONE_FLAT,  /* treble gain (0 dB) */
        0,                  /* right speaker attenuation (0 dB) */
        0                   /* left speaker attenuation (0 dB) */
    };

    size_t len = TDA74XX_REG_SPEAKER_LEFT - TDA74XX_INFO->reg_bass + 1;

    if (_write(dev, TDA74XX_INFO->reg_bass | TDA74XX_REG_INCREMENT,
               &filters[sizeof(filters) - len], len) != 0) {
        DEBUG("[tda74xx] tda74xx_init: init failed\n");
        return -ENODEV;
    }

    return 0;
}

int tda74xx_set_input(tda74xx_t *dev, tda74xx_input_t input)
{
    if (input >= TDA74XX_INFO->inputs) {
        DEBUG("[tda74xx] tda74xx_set_input: device does not have the input\n");
        return -ERANGE;
    }

    DEBUG("[tda74xx] tda74xx_set_input: input=%d\n", (int)input);

    uint8_t value = _input_value(input);

    return _write(dev, TDA74XX_REG_INPUT_SELECT, &value, sizeof(value));
}

int tda74xx_set_input_gain(tda74xx_t *dev, int8_t gain)
{
    if (gain < TDA74XX_INPUT_GAIN_MIN || gain > TDA74XX_INPUT_GAIN_MAX) {
        DEBUG("[tda74xx] tda74xx_set_input_gain: gain out of range\n");
        return -ERANGE;
    }

    DEBUG("[tda74xx] tda74xx_set_input_gain: gain=%d\n", (int)gain);

    /* the devices only support even gains, therefore round down */
    uint8_t value = gain / TDA74XX_INPUT_GAIN_STEP;

    return _write(dev, TDA74XX_REG_INPUT_GAIN, &value, sizeof(value));
}

int tda74xx_set_volume(tda74xx_t *dev, int8_t volume)
{
    if (volume < TDA74XX_VOLUME_MIN || volume > TDA74XX_VOLUME_MAX) {
        DEBUG("[tda74xx] tda74xx_set_volume: volume out of range\n");
        return -ERANGE;
    }

    DEBUG("[tda74xx] tda74xx_set_volume: volume=%d\n", (int)volume);

    uint8_t value = _attenuation_value(volume);

    int res = _write(dev, TDA74XX_REG_VOLUME, &value, sizeof(value));

    if (res != 0) {
        return res;
    }

    dev->volume = volume;
    dev->muted = false;

    return 0;
}

int tda74xx_set_mute(tda74xx_t *dev, bool mute)
{
    DEBUG("[tda74xx] tda74xx_set_mute: mute=%d\n", (int)mute);

    if (!mute) {
        return tda74xx_set_volume(dev, dev->volume);
    }

    uint8_t value = TDA74XX_VOLUME_MUTE;

    int res = _write(dev, TDA74XX_REG_VOLUME, &value, sizeof(value));

    if (res != 0) {
        return res;
    }

    dev->muted = true;

    return 0;
}

int tda74xx_set_bass(tda74xx_t *dev, int8_t gain)
{
    DEBUG("[tda74xx] tda74xx_set_bass: gain=%d\n", (int)gain);

    return _set_tone(dev, TDA74XX_INFO->reg_bass, gain);
}

#if TDA74XX_HAS_MIDDLE
int tda74xx_set_middle(tda74xx_t *dev, int8_t gain)
{
    assert(TDA74XX_INFO->bands == TDA74XX_INFO_BANDS_THREE);

    DEBUG("[tda74xx] tda74xx_set_middle: gain=%d\n", (int)gain);

    /* the register of the middle filter follows the register of the bass
     * filter */
    return _set_tone(dev, TDA74XX_INFO->reg_bass + 1, gain);
}
#endif /* TDA74XX_HAS_MIDDLE */

int tda74xx_set_treble(tda74xx_t *dev, int8_t gain)
{
    DEBUG("[tda74xx] tda74xx_set_treble: gain=%d\n", (int)gain);

    return _set_tone(dev, TDA74XX_REG_TREBLE, gain);
}

int tda74xx_set_attenuation(tda74xx_t *dev, tda74xx_speaker_t speaker,
                            int8_t attenuation)
{
    assert(speaker <= TDA74XX_SPEAKER_BOTH);

    if (attenuation < TDA74XX_ATTENUATION_MIN ||
        attenuation > TDA74XX_ATTENUATION_MAX) {
        DEBUG("[tda74xx] tda74xx_set_attenuation: attenuation out of range\n");
        return -ERANGE;
    }

    DEBUG("[tda74xx] tda74xx_set_attenuation: speaker=%d attenuation=%d\n",
          (int)speaker, (int)attenuation);

    uint8_t value = _attenuation_value(attenuation);

    if (speaker == TDA74XX_SPEAKER_BOTH) {
        /* the registers of both speakers are adjacent, therefore both can be
         * written in a single transaction */
        uint8_t values[] = { value, value };

        return _write(dev, TDA74XX_REG_SPEAKER_RIGHT | TDA74XX_REG_INCREMENT,
                      values, sizeof(values));
    }

    uint8_t reg = (speaker == TDA74XX_SPEAKER_LEFT)
                  ? TDA74XX_REG_SPEAKER_LEFT
                  : TDA74XX_REG_SPEAKER_RIGHT;

    return _write(dev, reg, &value, sizeof(value));
}
