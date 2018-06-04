/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <string.h>

#include "common/sensors.h"

#include "saul_reg.h"
#include "xtimer.h"
#include "utlist.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief   Allocate memory for stack
 */
static char _stack[THREAD_STACKSIZE_DEFAULT + DEBUG_EXTRA_STACKSIZE];

/**
 * @brief   Linked list of sensor entries
 */
static sensors_reg_entry_t *_entries_list;

/**
 * @brief   Lock for modifying the sensor entries.
 */
static mutex_t _lock = MUTEX_INIT;

/**
 * @brief   PID of the thread
 */
static kernel_pid_t sensors_pid = KERNEL_PID_UNDEF;

static void _smooth_ema(sensors_reg_entry_t *entry, phydat_t *in, phydat_t *out, int dim)
{
    /* copy current value in history if smoothing not read */
    if (!entry->smoothing.state.ema.ready) {
        entry->smoothing.state.ema.ready = true;
        memcpy(&entry->smoothing.state.ema.history, in, sizeof(phydat_t));
    }

    /* perform smoothing */
    uint8_t alpha = entry->smoothing_ema_alpha;

    if (alpha == 0) {
        alpha = 1;
    }

    DEBUG("[sensors] _smooth_ema: alpha=%d\n", alpha);

    for (unsigned i = 0; i < (unsigned)dim; i++) {
        int32_t val = in->val[i];
        int32_t last_val = entry->smoothing.state.ema.history.val[i];

        /* alpha is normally 0 - 1.0, but division by its inverse is used to
           prevent floats */
        out->val[i] = (int16_t) (last_val + ((val - last_val) / (100 / alpha)));
    }

    memcpy(&entry->smoothing.state.ema.history, out, sizeof(phydat_t));
}

static void _smooth_ma(sensors_reg_entry_t *entry, phydat_t *in, phydat_t *out, int dim)
{
    uint8_t samples = entry->smoothing_ma_samples;

    if (samples < 1) {
        samples = 1;
    }
    else if (samples > SENSORS_SMOOTHING_MA_MAX_SAMPLES) {
        samples = SENSORS_SMOOTHING_MA_MAX_SAMPLES;
    }

    /* copy new value to history */
    memcpy(&entry->smoothing.state.ma.samples[entry->smoothing.state.ma.index], in, sizeof(phydat_t));

    entry->smoothing.state.ma.index = (entry->smoothing.state.ma.index + 1) % samples;
    entry->smoothing.state.ma.count = entry->smoothing.state.ma.count + 1;

    if (entry->smoothing.state.ma.count > samples) {
        entry->smoothing.state.ma.count = samples;
    }

    DEBUG("[sensors] _smooth_ma: samples=%d count=%d index=%d\n",
          samples,
          entry->smoothing.state.ma.count,
          entry->smoothing.state.ma.index);

    /* perform smoothing */
    for (unsigned i = 0; i < (unsigned)dim; i++) {
        int32_t total = 0;
        uint8_t count = entry->smoothing.state.ma.count;

        for (unsigned j = 0; j < count; j++) {
            total = total + entry->smoothing.state.ma.samples[j].val[i];
        }

        out->val[i] = (int16_t)(total / count);
    }
}

static void _smooth(sensors_reg_entry_t *entry, phydat_t *res, int dim)
{
    DEBUG("[sensors] _smooth: algorithm=%d\n", entry->smoothing_algorithm);

    /* early exit if filtering is disabled */
    if (entry->smoothing_algorithm == SENSORS_SMOOTHING_ALGORITHM_NONE) {
        return;
    }

    /* reset smoothing state if algorithm changed */
    if (entry->smoothing.last_algorithm != entry->smoothing_algorithm) {
        memset(&entry->smoothing.state, 0, sizeof(entry->smoothing.state));
        entry->smoothing.last_algorithm = entry->smoothing_algorithm;
    }

    /* perform filtering */
    phydat_t out;

    memcpy(&out, res, sizeof(phydat_t));

    switch (entry->smoothing_algorithm) {
        case SENSORS_SMOOTHING_ALGORITHM_EMA:
            _smooth_ema(entry, res, &out, dim);
            break;
        case SENSORS_SMOOTHING_ALGORITHM_MA:
            _smooth_ma(entry, res, &out, dim);
            break;
        default:
            break;
    }

    memcpy(res, &out, sizeof(phydat_t));
}

static void _read(sensors_reg_entry_t *entry)
{
    DEBUG("[sensors] _read: sensor=%p\n", entry);

    /* check if sensor is due for update */
    uint32_t now = xtimer_now_usec() / US_PER_MS;

    if ((now - entry->update.last_update) < (entry->update_time * 1000)) {
        DEBUG("[sensors] _read: sensor not due\n");
        return;
    }

    /* force reading once in a while */
    bool force = false;

    if ((now - entry->update.last_publish) > (entry->update_time_max * 1000)) {
        DEBUG("[sensors] _read: forcing sensor update\n");
        force = true;
    }

    /* read sensor value */
    DEBUG("[sensors] _read: reading saul_reg=%p\n", entry->saul_dev);

    if (entry->saul_dev == NULL) {
        DEBUG("[sensors] _read: SAUL registration is NULL.\n");
        return;
    }

    phydat_t res;
    int dim = saul_reg_read(entry->saul_dev, &res);

    entry->update.last_update = now;

    if (dim <= 0) {
        DEBUG("[sensors] _read: failed to read\n");
        return;
    }

    /* smooth data if configured */
    _smooth(entry, &res, dim);

    /* determine whether the result should be published */
    if (!force && entry->smart_update) {
        bool update = false;

        for (unsigned i = 0; i < (unsigned)dim; i++) {
            int32_t diff;

            if (entry->update.value.val[i] < 0) {
                diff = (-1 * entry->update.value.val[i]) / (entry->difference * 100);
            }
            else {
                diff = entry->update.value.val[i] / (entry->difference * 100);
            }

            int32_t lower = entry->update.value.val[i] - diff;
            int32_t upper = entry->update.value.val[i] + diff;

            DEBUG("[sensors] _read: diff=%d lower=%" PRId32 " "
                  "upper=%" PRId32 " old=%" PRId16 " new=%" PRId16 "\n",
                  entry->difference,
                  lower,
                  upper,
                  entry->update.value.val[i],
                  res.val[i]);

            if (res.val[i] < lower || res.val[i] > upper) {
                DEBUG("[sensors] _read: sensor value changed out of bounds\n");
                update = true;

                break;
            }
        }

        /* no need to process any further */
        if (!update) {
            return;
        }

        /* copy value over for next time */
        memcpy(&(entry->update.value), &res, sizeof(res));
    }

    /* invoke user callback */
    entry->update.count++;
    entry->update.last_publish = now;

    if (entry->cb != NULL) {
        entry->cb(&res, entry->cb_arg);
    }
}

static void *_event_loop(void *args)
{
    (void)args;

    /* start event loop */
    while (1) {
        uint32_t start = xtimer_now_usec();

        /* handle sensors one by one */
        sensors_reg_entry_t *entry;

        LL_FOREACH(_entries_list, entry) {
            if (entry->enabled) {
                _read(entry);
            }
        }

        uint32_t stop = xtimer_now_usec();
        uint32_t interval = 1 * US_PER_SEC;
        uint32_t delta = (stop > start && (stop - start) < interval)
                       ? (stop - start)
                       : 0;

        xtimer_usleep(interval - delta);
    }

    return NULL;
}

void sensors_register(sensors_reg_entry_t *entry)
{
    mutex_lock(&_lock);
    LL_APPEND(_entries_list, entry);
    mutex_unlock(&_lock);
}

void sensors_unregister(sensors_reg_entry_t *entry)
{
    mutex_lock(&_lock);
    LL_DELETE(_entries_list, entry);
    mutex_unlock(&_lock);
}

kernel_pid_t sensors_init(void)
{
    if (sensors_pid == KERNEL_PID_UNDEF) {
        sensors_pid = thread_create(_stack,
                                    sizeof(_stack),
                                    THREAD_PRIORITY_MAIN - 1,
                                    THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST,
                                    _event_loop,
                                    NULL,
                                    "sensors");
    }

    return sensors_pid;
}
