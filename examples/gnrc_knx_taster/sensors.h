/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <stdbool.h>

#include "phydat.h"
#include "kernel_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Signature of button event callback functions
 *
 * @param[in] arg       optional context for the callback
 */
typedef void (*sensor_cb_t)(phydat_t *result, void *arg);

typedef struct {
    /* hardware configuration */
    uint8_t saul;
    bool enabled;
    bool smart_update;
    uint8_t difference;

    /* timing configuration */
    uint32_t update_time;
    uint32_t update_time_max;

    /* update state */
    struct {
        phydat_t value;
        uint32_t last_update;
        uint32_t last_publish;
    } update;

    /* callback configuration */
    sensor_cb_t cb;
    void *cb_arg;
} sensor_t;

kernel_pid_t sensor_setup(sensor_t *_sensors, uint8_t _sensors_count);

#ifdef __cplusplus
}
#endif

#endif /* SENSORS_H */
/** @} */
