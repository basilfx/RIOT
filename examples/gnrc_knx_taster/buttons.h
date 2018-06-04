/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include "kernel_types.h"

#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_BUTTON_IRQ      (1)
#define MSG_BUTTON_TIMER    (2)

typedef enum {
    BUTTON_SEQUENCE_0,
    BUTTON_SEQUENCE_0_1,
    BUTTON_SEQUENCE_0_1_H,
    BUTTON_SEQUENCE_0_1_H_H,
    BUTTON_SEQUENCE_0_1_0
} button_sequence_t;

typedef enum {
    BUTTON_DEBOUNCE_IDLE,
    BUTTON_DEBOUNCE_BUSY
} button_debounce_t;

typedef enum {
    BUTTON_EVENT_PRESS,
    BUTTON_EVENT_LONG_PRESS_START,
    BUTTON_EVENT_LONG_PRESS_STOP,
    BUTTON_EVENT_LONGER_PRESS_START,
    BUTTON_EVENT_LONGER_PRESS_STOP
} button_event_t;

typedef enum {
    BUTTON_TRANSITION_LOW_TO_HIGH,
    BUTTON_TRANSITION_HIGH_TO_LOW,
    BUTTON_TRANSITION_HIGH_TO_HIGH,
    BUTTON_TRANSITION_LOW_TO_LOW
} button_transition_t;

/**
 * @brief   Signature of button event callback functions
 *
 * @param[in] arg       optional context for the callback
 */
typedef void (*button_cb_t)(button_event_t event, uint8_t repeat, void *arg);

typedef struct {
    /* hardware configuration */
    gpio_t pin;
    bool enabled;
    bool invert;
    bool pull;

    /* timing configuration */
    uint32_t debounce_time;
    uint32_t press_time;
    uint32_t long_press_time;
    uint32_t longer_press_time;

    /* sequence configuration */
    uint8_t max_repeat;

    /* callback configuration */
    button_cb_t cb;
    void *cb_arg;

    /* debounce state */
    struct {
        bool value;
        uint8_t time;
        button_debounce_t state;
    } debounce;

    /* interrupt state */
    struct {
        bool value;
        bool last_value;
        uint32_t time;
        uint32_t last_time;
    } interrupt;

    /* event state */
    struct {
        bool available;
        button_transition_t transition;
        uint32_t time;
        button_sequence_t sequence;
        uint8_t repeat;
    } press;
} button_t;

void button_load(void);
kernel_pid_t button_setup(button_t *_buttons, uint8_t _buttons_count);

#ifdef __cplusplus
}
#endif

#endif /* BUTTONS_H */
/** @} */
