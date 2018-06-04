/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <string.h>

#include "thread.h"
#include "msg.h"
#include "xtimer.h"
#include "saul_reg.h"
#include "phydat.h"

#include "device.h"
#include "sensors.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static char _stack[THREAD_STACKSIZE_DEFAULT + DEBUG_EXTRA_STACKSIZE];

static sensor_t *sensors;
static uint8_t sensors_count;

static void _read(sensor_t *sensor)
{
    /* check if sensor is due for update */
    uint32_t now = xtimer_now_usec() / US_PER_MS;

    if ((now - sensor->update.last_update) < (sensor->update_time * 1000)) {
        DEBUG("[sensors] _read: sensor not due\n");
        return;
    }

    /* force reading once in a while */
    bool force = false;

    if ((now - sensor->update.last_publish) > (sensor->update_time_max * 1000)) {
        DEBUG("[sensors] _read: forcing sensor update\n");
        force = true;
    }

    /* read sensor value */
    DEBUG("[sensors] _read: device=%d\n", sensor->saul);

    phydat_t res;
    saul_reg_t *dev = saul_reg_find_nth(sensor->saul);
    int dim = saul_reg_read(dev, &res);

    sensor->update.last_update = now;

    if (dim <= 0) {
        DEBUG("[sensors] _read: failed to read from device %d\n", sensor->saul);
        return;
    }

    /* determine whether the result should be published */
    if (!force && sensor->smart_update) {
        bool update = false;

        for (unsigned i = 0; i < (unsigned)dim; i++) {
            int32_t diff;

            if (sensor->update.value.val[i] < 0) {
                diff = (-1 * sensor->update.value.val[i]) / (sensor->difference * 100);
            }
            else {
                diff = sensor->update.value.val[i] / (sensor->difference * 100);
            }
            int32_t lower = sensor->update.value.val[i] - diff;
            int32_t upper = sensor->update.value.val[i] + diff;

            DEBUG("[sensors] _read: diff=%d lower=%" PRId32 " "
                  "upper=%" PRId32 " old=%" PRId16 " new=%" PRId16 "\n",
                  sensor->difference,
                  lower, upper,
                  sensor->update.value.val[i],
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
        memcpy(&(sensor->update.value), &res, sizeof(res));
    }

    /* invoke user callback */
    sensor->update.last_publish = now;

    if (sensor->cb != NULL) {
        sensor->cb(&res, sensor->cb_arg);
    }
}

static void *_event_loop(void *args)
{
    (void)args;

    /* start event loop */
    while (1) {
        uint32_t start = xtimer_now_usec();

        for (unsigned i = 0; i < sensors_count; i++) {
            if (sensors[i].enabled) {
                _read(&(sensors[i]));
            }
        }

        uint32_t stop = xtimer_now_usec();
        uint32_t interval = 1 * US_PER_SEC;
        uint32_t delta = (stop > start && (stop - start) < interval)
            ? (stop - start) : 0;

        xtimer_usleep(interval - delta);
    }

    return NULL;
}

kernel_pid_t sensor_setup(sensor_t *_sensors, uint8_t _sensors_count)
{
    sensors = _sensors;
    sensors_count = _sensors_count;

    DEBUG("[sensors] sensor_setup: setting up %d sensors\n", sensors_count);

    return thread_create(_stack, sizeof(_stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST, _event_loop, NULL, "sensors");
}
