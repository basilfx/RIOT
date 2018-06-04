/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "common/sensors.h"

const char *sensors_smoothing_algorithm_to_str(sensors_smoothing_algorithm_t algorithm)
{
    switch (algorithm) {
        case SENSORS_SMOOTHING_ALGORITHM_EMA:
            return "Exponential Moving Average";
        case SENSORS_SMOOTHING_ALGORITHM_MA:
            return "Moving Average";
        default:
            return "None";
    }
}