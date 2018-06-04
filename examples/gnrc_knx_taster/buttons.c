/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "buttons.h"

#include "msg.h"
#include "thread.h"
#include "thread_flags.h"
#include "xtimer.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define BUTTON_MSG_QUEUE_SIZE (4)

/**
 * @brief   Allocate memory for stack
 */
static char _stack[THREAD_STACKSIZE_DEFAULT + DEBUG_EXTRA_STACKSIZE];

/**
 * @brief   PID of the thread
 */
static kernel_pid_t button_pid = KERNEL_PID_UNDEF;

static button_t *buttons;
static uint8_t buttons_count;

static void _cb_pin(void *arg)
{
    (void)arg;

    /* unblock thread */
    thread_flags_set((thread_t *)thread_get(button_pid), MSG_BUTTON_IRQ);
}

static void _handle_event(button_t *button, uint8_t event, uint8_t repeat)
{
    DEBUG("[buttons] _handle_event: button=%p, event=%d, repeat=%d\n", button, event, repeat);

    if (button->cb != NULL) {
        button->cb(event, repeat, button->cb_arg);
    }
}

static bool _handle_interrupt(button_t *button)
{
    uint32_t now = xtimer_now_usec() / US_PER_MS;

    if (button->debounce.state == BUTTON_DEBOUNCE_IDLE) {
        bool value = button->invert ?
                     !gpio_read(button->pin) : gpio_read(button->pin);

        if (value != button->interrupt.value) {
            button->debounce.state = BUTTON_DEBOUNCE_BUSY;
            button->debounce.time = now;
            button->debounce.value = value;
        }
    }
    else if (button->debounce.state == BUTTON_DEBOUNCE_BUSY) {
        bool value = button->invert ?
                     !gpio_read(button->pin) : gpio_read(button->pin);

        if (button->debounce.value == value) {
            if ((now - button->debounce.time) >= button->debounce_time) {
                button->interrupt.last_value = button->interrupt.value;
                button->interrupt.value = value;
                button->interrupt.last_time = button->interrupt.time;
                button->interrupt.time = now;

                /* handle the press */
                button->press.available = true;
                button->press.time = now;

                if (button->interrupt.value && !button->interrupt.last_value) {
                    button->press.transition = BUTTON_TRANSITION_HIGH_TO_LOW;
                }
                else if (!button->interrupt.value && button->interrupt.last_value) {
                    button->press.transition = BUTTON_TRANSITION_LOW_TO_HIGH;
                }
                else if (button->interrupt.value && button->interrupt.last_value) {
                    button->press.transition = BUTTON_TRANSITION_HIGH_TO_HIGH;
                }
                else if (!button->interrupt.value && !button->interrupt.last_value) {
                    button->press.transition = BUTTON_TRANSITION_HIGH_TO_LOW;
                }
                else {
                    /* impossible state */
                }

                /* reset state */
                button->debounce.state = BUTTON_DEBOUNCE_IDLE;
            }
        }
        else {
            /* reset state */
            button->debounce.state = BUTTON_DEBOUNCE_IDLE;
        }
    }

    return (button->debounce.state == BUTTON_DEBOUNCE_IDLE);
}

static bool _handle_press(button_t *button)
{
    uint32_t now = xtimer_now_usec() / US_PER_MS;

    /* check if there is something to do */
    if (button->press.sequence == BUTTON_SEQUENCE_0 && button->press.repeat == 0) {
        if (!button->press.available) {
            return true;
        }
    }

    /* enter the state machine */
    switch (button->press.sequence) {
        case BUTTON_SEQUENCE_0:
            if (button->press.available && button->press.transition == BUTTON_TRANSITION_HIGH_TO_LOW) {
                button->press.available = false;
                button->press.sequence = BUTTON_SEQUENCE_0_1;
            }
            break;

        case BUTTON_SEQUENCE_0_1:
            if (button->press.available && button->press.transition == BUTTON_TRANSITION_LOW_TO_HIGH) {
                button->press.available = false;
                button->press.sequence = BUTTON_SEQUENCE_0_1_0;
            }
            else {
                if ((now - button->press.time) > button->long_press_time) {
                    _handle_event(button, BUTTON_EVENT_LONG_PRESS_START, button->press.repeat);
                    button->press.sequence = BUTTON_SEQUENCE_0_1_H;
                }
            }
            break;
        case BUTTON_SEQUENCE_0_1_H:
            if (button->press.available) {
                _handle_event(button, BUTTON_EVENT_LONG_PRESS_STOP, button->press.repeat);
                button->press.available = false;
                button->press.sequence = BUTTON_SEQUENCE_0;
                button->press.repeat = 0;
            }
            else {
                if ((now - button->press.time) > button->longer_press_time) {
                    _handle_event(button, BUTTON_EVENT_LONGER_PRESS_START, button->press.repeat);
                    button->press.sequence = BUTTON_SEQUENCE_0_1_H_H;
                }
            }
            break;
        case BUTTON_SEQUENCE_0_1_H_H:
            if (button->press.available) {
                _handle_event(button, BUTTON_EVENT_LONG_PRESS_STOP, button->press.repeat);
                _handle_event(button, BUTTON_EVENT_LONGER_PRESS_STOP, button->press.repeat);
                button->press.available = false;
                button->press.sequence = BUTTON_SEQUENCE_0;
                button->press.repeat = 0;
            }
            break;
        case BUTTON_SEQUENCE_0_1_0:
            if (button->press.available && button->press.transition == 1) {
                button->press.available = false;

                if (button->press.repeat < button->max_repeat) {
                    button->press.sequence = BUTTON_SEQUENCE_0_1;
                    button->press.repeat++;
                }
                else {
                    _handle_event(button, BUTTON_EVENT_PRESS, button->press.repeat);
                    button->press.sequence = BUTTON_SEQUENCE_0;
                    button->press.repeat = 0;
                }
            }
            else {
                if ((now - button->press.time) > button->press_time) {
                    _handle_event(button, BUTTON_EVENT_PRESS, button->press.repeat);
                    button->press.sequence = BUTTON_SEQUENCE_0;
                    button->press.repeat = 0;
                }
            }
            break;
    }

    return false;
}

static void *_event_loop(void *args)
{
    (void)args;

    /* configure the buttons */
    button_load();

    while (1) {
        uint32_t start = xtimer_now_usec();
        bool sleep = true;

        /* handle buttons one by one */
        for (unsigned i = 0; i < buttons_count; i++) {
            if (buttons[i].enabled) {
                sleep &= _handle_interrupt(&(buttons[i]));
                sleep &= _handle_press(&(buttons[i]));
            }
        }

        /* block thread if no button needs attention (it will be woken up
           on next interrupt) */
        if (sleep) {
            thread_flags_wait_one(MSG_BUTTON_IRQ);
        }
        else {
            uint32_t stop = xtimer_now_usec();
            uint32_t interval = 1 * US_PER_MS;
            uint32_t delta = (stop > start && (stop - start) < interval)
                ? (stop - start) : 0;

            xtimer_usleep(interval - delta);
        }
    }

    return NULL;
}

void button_load(void)
{
    for (unsigned i = 0; i < buttons_count; i++) {
        DEBUG("[buttons] button_load: button=%d enabled=%d invert=%d pull=%d "
              "max_repeat=%d debounce_time=%" PRIu32 "\n",
              i,
              buttons[i].enabled,
              buttons[i].invert,
              buttons[i].pull,
              buttons[i].max_repeat,
              buttons[i].debounce_time);

        /* reset state */
        buttons[i].debounce.state = BUTTON_DEBOUNCE_IDLE;
        buttons[i].press.sequence = BUTTON_SEQUENCE_0;
        buttons[i].press.repeat = 0;

        /* configure hardware (pull-down on non-connected ports) */
        if (buttons[i].enabled) {
            if (buttons[i].invert) {
                if (buttons[i].pull) {
                    /* high (pull) -> low */
                    gpio_init_int(buttons[i].pin, GPIO_IN_PU, GPIO_BOTH, _cb_pin, &(buttons[i]));
                }
                else {
                    /* high -> low */
                    gpio_init_int(buttons[i].pin, GPIO_IN, GPIO_BOTH, _cb_pin, &(buttons[i]));
                }
            }
            else {
                if (buttons[i].pull) {
                    /* low (pull) -> high */
                    gpio_init_int(buttons[i].pin, GPIO_IN_PD, GPIO_BOTH, _cb_pin, &(buttons[i]));
                }
                else {
                    /* low -> high */
                    gpio_init_int(buttons[i].pin, GPIO_IN, GPIO_BOTH, _cb_pin, &(buttons[i]));
                }
            }
        }
        else {
            gpio_init(buttons[i].pin, GPIO_IN_PD);
        }
    }
}

kernel_pid_t button_setup(button_t *_buttons, uint8_t _buttons_count)
{
    buttons = _buttons;
    buttons_count = _buttons_count;

    DEBUG("[buttons] button_setup: setting up %d buttons\n", buttons_count);

    if (button_pid == KERNEL_PID_UNDEF) {
        button_pid = thread_create(_stack, sizeof(_stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST, _event_loop, NULL, "buttons");
    }

    return button_pid;
}
