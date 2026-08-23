/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup tests
 * @{
 *
 * @file
 * @brief       Test application for the ST TDA74xx audio processors
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"

#include "tda74xx.h"
#include "tda74xx_params.h"

static tda74xx_t tda74xx_devs[ARRAY_SIZE(tda74xx_params)];

static tda74xx_t *_get_dev(const char *arg)
{
    unsigned dev = strtoul(arg, NULL, 10);

    if (dev >= ARRAY_SIZE(tda74xx_devs)) {
        puts("Invalid device.");
        return NULL;
    }

    return &tda74xx_devs[dev];
}

static int _info(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    for (unsigned i = 0; i < ARRAY_SIZE(tda74xx_devs); i++) {
        const tda74xx_t *dev = &tda74xx_devs[i];

        printf("TDA74xx #%d\n", i);
        printf("Inputs: %d\n", dev->params.info->inputs);
        printf("Tone control bands: %d\n", dev->params.info->bands);
        printf("Volume: %d dB (%s)\n", dev->volume,
               dev->muted ? "muted" : "unmuted");

        puts("");
    }

    printf("Input gain: %d - %d dB in steps of %d dB\n",
           TDA74XX_INPUT_GAIN_MIN, TDA74XX_INPUT_GAIN_MAX,
           TDA74XX_INPUT_GAIN_STEP);
    printf("Volume: %d - %d dB in steps of %d dB\n",
           TDA74XX_VOLUME_MIN, TDA74XX_VOLUME_MAX, TDA74XX_VOLUME_STEP);
    printf("Tone control: %d - %d dB in steps of %d dB\n",
           TDA74XX_TONE_MIN, TDA74XX_TONE_MAX, TDA74XX_TONE_STEP);
    printf("Speaker attenuation: %d - %d dB in steps of %d dB\n",
           TDA74XX_ATTENUATION_MIN, TDA74XX_ATTENUATION_MAX,
           TDA74XX_ATTENUATION_STEP);

    return 0;
}

static int _set_input(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <dev> <input (1-4)>\n", argv[0]);
        return 1;
    }

    tda74xx_t *dev = _get_dev(argv[1]);

    if (dev == NULL) {
        return 1;
    }

    unsigned input = strtoul(argv[2], NULL, 10);

    /* the driver rejects inputs the device does not have */
    if (input < 1 || input > 4) {
        puts("Invalid input.");
        return 1;
    }

    int res = tda74xx_set_input(dev, (tda74xx_input_t)(input - 1));

    if (res != 0) {
        printf("Failed to select input (%d).\n", res);
        return 1;
    }

    return 0;
}

static int _set_input_gain(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <dev> <gain (dB)>\n", argv[0]);
        return 1;
    }

    tda74xx_t *dev = _get_dev(argv[1]);

    if (dev == NULL) {
        return 1;
    }

    int res = tda74xx_set_input_gain(dev, strtol(argv[2], NULL, 10));

    if (res != 0) {
        printf("Failed to set input gain (%d).\n", res);
        return 1;
    }

    return 0;
}

static int _set_volume(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <dev> <volume (dB)>\n", argv[0]);
        return 1;
    }

    tda74xx_t *dev = _get_dev(argv[1]);

    if (dev == NULL) {
        return 1;
    }

    int res = tda74xx_set_volume(dev, strtol(argv[2], NULL, 10));

    if (res != 0) {
        printf("Failed to set volume (%d).\n", res);
        return 1;
    }

    return 0;
}

static int _set_mute(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <dev> <0|1>\n", argv[0]);
        return 1;
    }

    tda74xx_t *dev = _get_dev(argv[1]);

    if (dev == NULL) {
        return 1;
    }

    int res = tda74xx_set_mute(dev, strtoul(argv[2], NULL, 10) != 0);

    if (res != 0) {
        printf("Failed to set mute (%d).\n", res);
        return 1;
    }

    return 0;
}

static int _set_bass(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <dev> <gain (dB)>\n", argv[0]);
        return 1;
    }

    tda74xx_t *dev = _get_dev(argv[1]);

    if (dev == NULL) {
        return 1;
    }

    int res = tda74xx_set_bass(dev, strtol(argv[2], NULL, 10));

    if (res != 0) {
        printf("Failed to set bass gain (%d).\n", res);
        return 1;
    }

    return 0;
}

#if TDA74XX_HAS_MIDDLE
static int _set_middle(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <dev> <gain (dB)>\n", argv[0]);
        return 1;
    }

    tda74xx_t *dev = _get_dev(argv[1]);

    if (dev == NULL) {
        return 1;
    }

    if (dev->params.info->bands != TDA74XX_INFO_BANDS_THREE) {
        puts("Device does not have a middle filter.");
        return 1;
    }

    int res = tda74xx_set_middle(dev, strtol(argv[2], NULL, 10));

    if (res != 0) {
        printf("Failed to set middle gain (%d).\n", res);
        return 1;
    }

    return 0;
}
#endif /* TDA74XX_HAS_MIDDLE */

static int _set_treble(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s <dev> <gain (dB)>\n", argv[0]);
        return 1;
    }

    tda74xx_t *dev = _get_dev(argv[1]);

    if (dev == NULL) {
        return 1;
    }

    int res = tda74xx_set_treble(dev, strtol(argv[2], NULL, 10));

    if (res != 0) {
        printf("Failed to set treble gain (%d).\n", res);
        return 1;
    }

    return 0;
}

static int _set_attenuation(int argc, char **argv)
{
    if (argc != 4) {
        printf("Usage: %s <dev> <left|right|both> <attenuation (dB)>\n",
               argv[0]);
        return 1;
    }

    tda74xx_t *dev = _get_dev(argv[1]);

    if (dev == NULL) {
        return 1;
    }

    tda74xx_speaker_t speaker;

    if (strcmp(argv[2], "left") == 0) {
        speaker = TDA74XX_SPEAKER_LEFT;
    }
    else if (strcmp(argv[2], "right") == 0) {
        speaker = TDA74XX_SPEAKER_RIGHT;
    }
    else if (strcmp(argv[2], "both") == 0) {
        speaker = TDA74XX_SPEAKER_BOTH;
    }
    else {
        puts("Invalid speaker.");
        return 1;
    }

    int res = tda74xx_set_attenuation(dev, speaker, strtol(argv[3], NULL, 10));

    if (res != 0) {
        printf("Failed to set attenuation (%d).\n", res);
        return 1;
    }

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "info", "Print audio processor info.", _info },
    { "set_input", "Select the input.", _set_input },
    { "set_input_gain", "Set the input gain (in dB).", _set_input_gain },
    { "set_volume", "Set the volume (in dB).", _set_volume },
    { "set_mute", "Mute or unmute the volume.", _set_mute },
    { "set_bass", "Set the bass gain (in dB).", _set_bass },
#if TDA74XX_HAS_MIDDLE
    { "set_middle", "Set the middle gain (in dB).", _set_middle },
#endif /* TDA74XX_HAS_MIDDLE */
    { "set_treble", "Set the treble gain (in dB).", _set_treble },
    { "set_attenuation", "Set the speaker attenuation (in dB).", _set_attenuation },
    { NULL, NULL, NULL }
};

int main(void)
{
    /* initialize the audio processors */
    puts("Initializing audio processors... ");

    for (unsigned i = 0; i < ARRAY_SIZE(tda74xx_params); i++) {
        if (tda74xx_init(&tda74xx_devs[i], &tda74xx_params[i]) == 0) {
            printf("TDA74xx #%d [OK]\n", i);
        }
        else {
            printf("TDA74xx #%d [FAILED]\n", i);
        }
    }

    /* run shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
