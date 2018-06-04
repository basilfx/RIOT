/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "common/buttons.h"

#include "msg.h"
#include "mutex.h"
#include "thread.h"
#include "thread_flags.h"
#include "xtimer.h"
#include "utlist.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief   Allocate memory for stack
 */
static char _stack[THREAD_STACKSIZE_DEFAULT + DEBUG_EXTRA_STACKSIZE];

/**
 * @brief   Linked list of button entries
 */
static buttons_reg_entry_t *_entries_list;

/**
 * @brief   Lock for modifying the button entries.
 */
static mutex_t _lock = MUTEX_INIT;

/**
 * @brief   PID of the thread
 */
static kernel_pid_t buttons_pid = KERNEL_PID_UNDEF;

static void _cb_pin(void *arg)
{
    (void)arg;

    /* unblock event loop thread */
    thread_flags_set((thread_t *)thread_get(buttons_pid), FLAG_BUTTONS_IRQ);
}

static void _handle_event(buttons_reg_entry_t *entry, uint8_t event, uint8_t repeat)
{
    DEBUG("[buttons] _handle_event: entry=%p event=%d repeat=%d\n", entry, event, repeat);

    if (entry->cb != NULL) {
        entry->cb(event, repeat, entry->cb_arg);
    }
}

static bool _handle_interrupt(buttons_reg_entry_t *entry)
{
    uint32_t now = xtimer_now_usec() / US_PER_MS;

    if (entry->debounce.state == BUTTONS_DEBOUNCE_IDLE) {
        bool value = entry->invert ?
                     !gpio_read(entry->pin) : gpio_read(entry->pin);

        if (value != entry->interrupt.value) {
            entry->debounce.state = BUTTONS_DEBOUNCE_BUSY;
            entry->debounce.time = now;
            entry->debounce.value = value;
        }
    }
    else if (entry->debounce.state == BUTTONS_DEBOUNCE_BUSY) {
        bool value = entry->invert ?
                     !gpio_read(entry->pin) : gpio_read(entry->pin);

        if (entry->debounce.value == value) {
            if ((now - entry->debounce.time) >= entry->debounce_time) {
                entry->interrupt.last_value = entry->interrupt.value;
                entry->interrupt.value = value;
                entry->interrupt.last_time = entry->interrupt.time;
                entry->interrupt.time = now;

                /* handle the press */
                entry->press.available = true;
                entry->press.time = now;

                if (entry->interrupt.value && !entry->interrupt.last_value) {
                    entry->press.transition = BUTTONS_TRANSITION_HIGH_TO_LOW;
                }
                else if (!entry->interrupt.value && entry->interrupt.last_value) {
                    entry->press.transition = BUTTONS_TRANSITION_LOW_TO_HIGH;
                }
                else if (entry->interrupt.value && entry->interrupt.last_value) {
                    entry->press.transition = BUTTONS_TRANSITION_HIGH_TO_HIGH;
                }
                else if (!entry->interrupt.value && !entry->interrupt.last_value) {
                    entry->press.transition = BUTTONS_TRANSITION_HIGH_TO_LOW;
                }
                else {
                    /* impossible state */
                }

                /* reset state */
                entry->debounce.state = BUTTONS_DEBOUNCE_IDLE;
            }
        }
        else {
            /* reset state */
            entry->debounce.state = BUTTONS_DEBOUNCE_IDLE;
        }
    }

    return (entry->debounce.state == BUTTONS_DEBOUNCE_IDLE);
}

static bool _handle_press(buttons_reg_entry_t *entry)
{
    uint32_t now = xtimer_now_usec() / US_PER_MS;

    /* check if there is something to do */
    if (entry->press.sequence == BUTTONS_SEQUENCE_0 && entry->press.repeat == 0) {
        if (!entry->press.available) {
            return true;
        }
    }

    /* enter the state machine */
    switch (entry->press.sequence) {
        case BUTTONS_SEQUENCE_0:
            if (entry->press.available && entry->press.transition == BUTTONS_TRANSITION_HIGH_TO_LOW) {
                entry->press.available = false;
                entry->press.sequence = BUTTONS_SEQUENCE_0_1;
            }
            break;

        case BUTTONS_SEQUENCE_0_1:
            if (entry->press.available && entry->press.transition == BUTTONS_TRANSITION_LOW_TO_HIGH) {
                entry->press.available = false;
                entry->press.sequence = BUTTONS_SEQUENCE_0_1_0;
            }
            else {
                if ((now - entry->press.time) > entry->long_press_time) {
                    _handle_event(entry, BUTTONS_EVENT_LONG_PRESS_START, entry->press.repeat);
                    entry->press.sequence = BUTTONS_SEQUENCE_0_1_H;
                }
            }
            break;
        case BUTTONS_SEQUENCE_0_1_H:
            if (entry->press.available) {
                _handle_event(entry, BUTTONS_EVENT_LONG_PRESS_STOP, entry->press.repeat);
                entry->press.available = false;
                entry->press.sequence = BUTTONS_SEQUENCE_0;
                entry->press.repeat = 0;
            }
            else {
                if ((now - entry->press.time) > entry->longer_press_time) {
                    _handle_event(entry, BUTTONS_EVENT_LONGER_PRESS_START, entry->press.repeat);
                    entry->press.sequence = BUTTONS_SEQUENCE_0_1_H_H;
                }
            }
            break;
        case BUTTONS_SEQUENCE_0_1_H_H:
            if (entry->press.available) {
                _handle_event(entry, BUTTONS_EVENT_LONG_PRESS_STOP, entry->press.repeat);
                _handle_event(entry, BUTTONS_EVENT_LONGER_PRESS_STOP, entry->press.repeat);
                entry->press.available = false;
                entry->press.sequence = BUTTONS_SEQUENCE_0;
                entry->press.repeat = 0;
            }
            break;
        case BUTTONS_SEQUENCE_0_1_0:
            if (entry->press.available && entry->press.transition == 1) {
                entry->press.available = false;

                if (entry->press.repeat < entry->max_repeat) {
                    entry->press.sequence = BUTTONS_SEQUENCE_0_1;
                    entry->press.repeat++;
                }
                else {
                    _handle_event(entry, BUTTONS_EVENT_PRESS, entry->press.repeat);
                    entry->press.sequence = BUTTONS_SEQUENCE_0;
                    entry->press.repeat = 0;
                }
            }
            else {
                if ((now - entry->press.time) > entry->press_time) {
                    _handle_event(entry, BUTTONS_EVENT_PRESS, entry->press.repeat);
                    entry->press.sequence = BUTTONS_SEQUENCE_0;
                    entry->press.repeat = 0;
                }
            }
            break;
    }

    return false;
}

static void _load(buttons_reg_entry_t *entry)
{
    DEBUG("[buttons] _load: entry=%p enabled=%d invert=%d pull=%d "
          "max_repeat=%d debounce_time=%" PRIu32 "\n",
          entry,
          entry->enabled,
          entry->invert,
          entry->pull,
          entry->max_repeat,
          entry->debounce_time);

    /* reset state */
    entry->debounce.state = BUTTONS_DEBOUNCE_IDLE;
    entry->press.sequence = BUTTONS_SEQUENCE_0;
    entry->press.repeat = 0;

    /* configure hardware (pull-down on non-connected ports) */
    if (entry->enabled) {
        if (entry->invert) {
            if (entry->pull) {
                /* high (pull) -> low */
                gpio_init_int(entry->pin, GPIO_IN_PU, GPIO_BOTH, _cb_pin, entry);
            }
            else {
                /* high -> low */
                gpio_init_int(entry->pin, GPIO_IN, GPIO_BOTH, _cb_pin, entry);
            }
        }
        else {
            if (entry->pull) {
                /* low (pull) -> high */
                gpio_init_int(entry->pin, GPIO_IN_PD, GPIO_BOTH, _cb_pin, entry);
            }
            else {
                /* low -> high */
                gpio_init_int(entry->pin, GPIO_IN, GPIO_BOTH, _cb_pin, entry);
            }
        }
    }
    else {
        gpio_init(entry->pin, GPIO_IN_PD);
    }
}

static void _unload(buttons_reg_entry_t *entry)
{
    DEBUG("[buttons] _unload: entry=%p\n", entry);

    /* disable pin */
    gpio_init(entry->pin, GPIO_IN_PD);
}

static void *_event_loop(void *args)
{
    (void)args;

    while (1) {
        uint32_t start = xtimer_now_usec();
        bool wait = true;

        /* handle buttons one by one */
        buttons_reg_entry_t *entry;

        LL_FOREACH(_entries_list, entry) {
            if (entry->enabled) {
                wait &= _handle_interrupt(entry);
                wait &= _handle_press(entry);
            }
        }

        /* block thread if no button needs attention (it will be woken up
           on next interrupt) */
        if (wait) {
            thread_flags_wait_one(FLAG_BUTTONS_IRQ);
        }
        else {
            uint32_t stop = xtimer_now_usec();
            uint32_t interval = 1 * US_PER_MS;
            uint32_t delta = (stop > start && (stop - start) < interval)
                           ? (stop - start)
                           : 0;

            xtimer_usleep(interval - delta);
        }
    }

    return NULL;
}

void buttons_register(buttons_reg_entry_t *entry)
{
    /* load button */
    _load(entry);

    /* add to the list */
    mutex_lock(&_lock);
    LL_APPEND(_entries_list, entry);
    mutex_unlock(&_lock);
}

void buttons_unregister(buttons_reg_entry_t *entry)
{
    /* remove from the list */
    mutex_lock(&_lock);
    LL_DELETE(_entries_list, entry);
    mutex_unlock(&_lock);

    /* unload button */
    _unload(entry);
}

kernel_pid_t buttons_init(void)
{
    if (buttons_pid == KERNEL_PID_UNDEF) {
        buttons_pid = thread_create(_stack,
                                    sizeof(_stack),
                                    THREAD_PRIORITY_MAIN - 1,
                                    THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST,
                                    _event_loop,
                                    NULL,
                                    "buttons");
    }

    return buttons_pid;
}
