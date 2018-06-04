/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>
#include <stdint.h>

#include "periph/gpio.h"

#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FLAG_BUTTONS_IRQ    (1)

/**
 * @brief   Button debounce states
 */
typedef enum {
    BUTTONS_DEBOUNCE_IDLE,
    BUTTONS_DEBOUNCE_BUSY
} buttons_debounce_t;

/**
 * @brief   Button sequences
 */
typedef enum {
    BUTTONS_SEQUENCE_0,
    BUTTONS_SEQUENCE_0_1,
    BUTTONS_SEQUENCE_0_1_H,
    BUTTONS_SEQUENCE_0_1_H_H,
    BUTTONS_SEQUENCE_0_1_0
} buttons_sequence_t;

/**
 * @brief   Button events
 */
typedef enum {
    BUTTONS_EVENT_PRESS,
    BUTTONS_EVENT_LONG_PRESS_START,
    BUTTONS_EVENT_LONG_PRESS_STOP,
    BUTTONS_EVENT_LONGER_PRESS_START,
    BUTTONS_EVENT_LONGER_PRESS_STOP
} buttons_event_t;

/**
 * @brief   Button transitions
 */
typedef enum {
    BUTTONS_TRANSITION_LOW_TO_HIGH,
    BUTTONS_TRANSITION_HIGH_TO_LOW,
    BUTTONS_TRANSITION_HIGH_TO_HIGH,
    BUTTONS_TRANSITION_LOW_TO_LOW
} buttons_transition_t;

/**
 * @brief   Definition of a button event callback function
 *
 * @param[in] arg       optional context argument for the callback
 */
typedef void (*buttons_cb_t)(buttons_event_t event, uint8_t repeat, void *arg);

/**
 * @brief   Button registration entry
 *
 * The `next` and nested properties are considered private.
 */
typedef struct buttons_reg_entry {
    struct buttons_reg_entry *next;         /**< next for linked list */

    /* general configuration */
    bool enabled;

    /* hardware configuration */
    gpio_t pin;
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
    buttons_cb_t cb;
    void *cb_arg;

    /* debounce state */
    struct {
        bool value;
        uint8_t time;
        buttons_debounce_t state;
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
        buttons_transition_t transition;
        uint32_t time;
        buttons_sequence_t sequence;
        uint8_t repeat;
    } press;
} buttons_reg_entry_t;

/**
 * @brief   Register a button entry
 *
 * @param[in] entry     Button entry to register
 */
void buttons_register(buttons_reg_entry_t* entry);

/**
 * @brief   Unregister a button entry
 *
 * @param[in] entry     Button entry to unregister
 */
void buttons_unregister(buttons_reg_entry_t* entry);

/**
 * @brief   Initialize the buttons thread.
 *
 * This starts the processing of all registered entries. Entries can still be
 * registered if the thread is running.
 *
 * Only one instance of this thread will be created. The same PID is returned
 * otherwise.
 *
 * @return  PID of event loop thread
 */
kernel_pid_t buttons_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTONS_H */
/** @} */
