/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <stdbool.h>

#include "thread.h"
#include "phydat.h"
#include "saul_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Number of samples for a moving average filter
 */
#define SENSORS_SMOOTHING_MA_MAX_SAMPLES 10

/**
 * @brief   Smoothing filters for sensor values.
 */
typedef enum {
    SENSORS_SMOOTHING_ALGORITHM_NONE,
    SENSORS_SMOOTHING_ALGORITHM_EMA,
    SENSORS_SMOOTHING_ALGORITHM_MA,
    SENSORS_SMOOTHING_ALGORITHM_NUMOF,
} sensors_smoothing_algorithm_t;

/**
 * @brief   Signature of button event callback functions
 *
 * @param[in] arg       optional context for the callback
 */
typedef void (*sensors_cb_t)(phydat_t *result, void *arg);

/**
 * @brief   Sensor registration entry
 *
 * The `next` and nested properties are considered private.
 */
typedef struct sensors_reg_entry {
    struct sensors_reg_entry *next;     /**< next for linked list */

    /* general configuration */
    bool enabled;

    /* sensor configuration */
    saul_reg_t *saul_dev;
    bool smart_update;
    uint8_t difference;

    /* filter configuration */
    sensors_smoothing_algorithm_t smoothing_algorithm;
    uint8_t smoothing_ema_alpha;
    uint8_t smoothing_ma_samples;

    /* timing configuration */
    uint32_t update_time;
    uint32_t update_time_max;

    /* callback configuration */
    sensors_cb_t cb;
    void *cb_arg;

    /* update state */
    struct {
        phydat_t value;
        uint32_t last_update;
        uint32_t last_publish;
        uint32_t count;
    } update;

    /* smoothing state */
    struct {
        sensors_smoothing_algorithm_t last_algorithm;
        union {
            struct {
                bool ready;
                phydat_t history;
            } ema;
            struct {
                uint8_t index;
                uint8_t count;
                phydat_t samples[10];
            } ma;
        } state;
    } smoothing;
} sensors_reg_entry_t;

/**
 * @brief   Register a sensor entry
 *
 * @param[in] entry     Sensor entry to register
 */
void sensors_register(sensors_reg_entry_t* entry);

/**
 * @brief   Unregister a sensor entry
 *
 * @param[in] entry     Sensor entry to unregister
 */
void sensors_unregister(sensors_reg_entry_t* entry);

/**
 * @brief   Utility function to print the name of an smoothing algorithm
 *
 * @param[in] algorithm Name of the algorithm
 */
const char *sensors_smoothing_algorithm_to_str(sensors_smoothing_algorithm_t algorithm);

/**
 * @brief   Initialize the sensors thread.
 *
 * This starts the processing of all registered entries. Entries can still be
 * registered if the thread is running.
 *
 * Only one instance of this thread will be created. The same PID is returned
 * otherwise.
 *
 * @return  PID of event loop thread
 */
kernel_pid_t sensors_init(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSORS_H */
/** @} */
