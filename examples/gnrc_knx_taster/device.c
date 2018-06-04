/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <errno.h>
#include <string.h>
#include <math.h>

#include "net/knx.h"

#include "device.h"

#include "common/buttons.h"
#include "common/sensors.h"

#include "memory.h"
#include "properties.h"
#include "storage.h"
#include "com_objects.h"
#include "ota.h"

#include "net/gnrc/knx_l7.h"

#include "property_types.h"

#include "periph/pm.h"
#include "periph/rtc.h"
#include "periph/cpuid.h"

#include "xtimer.h"
#include "isrpipe.h"

#include "board.h"

#include "byteorder.h"
#include "random.h"
#include "hashes/sha256.h"

#define ENABLE_DEBUG 0
#include "debug.h"

#define DEVICE_MSG_QUEUE_SIZE (4)

static char _stack[THREAD_STACKSIZE_DEFAULT + DEBUG_EXTRA_STACKSIZE];

static kernel_pid_t pid;

static kernel_pid_t pid_buttons;
//static kernel_pid_t pid_display;
static kernel_pid_t pid_sensors;

static void _cb_buttons_interface(buttons_event_t event, uint8_t repeat, void *arg);
static void _cb_buttons_prog(buttons_event_t event, uint8_t repeat, void *arg);
static void _cb_sensors(phydat_t *result, void *arg);

static knx_assoc_mapping_t mappings[255];

static knx_assoc_t associations = {
    .mappings = mappings,
};

device_button_t device_buttons[] = {
    {
        .channel = 0,
        .switch_a = &(com_objects[COM_OBJECT_CHANNEL0_SWITCH_A]),
        .switch_b = &(com_objects[COM_OBJECT_CHANNEL0_SWITCH_B]),
        .switch_c = &(com_objects[COM_OBJECT_CHANNEL0_SWITCH_C]),
        .dimming = &(com_objects[COM_OBJECT_CHANNEL0_DIMMING]),
        .value = &(com_objects[COM_OBJECT_CHANNEL0_VALUE])
    },
    {
        .channel = 1,
        .switch_a = &(com_objects[COM_OBJECT_CHANNEL1_SWITCH_A]),
        .switch_b = &(com_objects[COM_OBJECT_CHANNEL1_SWITCH_B]),
        .switch_c = &(com_objects[COM_OBJECT_CHANNEL1_SWITCH_C]),
        .dimming = &(com_objects[COM_OBJECT_CHANNEL1_DIMMING]),
        .value = &(com_objects[COM_OBJECT_CHANNEL1_VALUE])
    },
    {
        .channel = 2,
        .switch_a = &(com_objects[COM_OBJECT_CHANNEL2_SWITCH_A]),
        .switch_b = &(com_objects[COM_OBJECT_CHANNEL2_SWITCH_B]),
        .switch_c = &(com_objects[COM_OBJECT_CHANNEL2_SWITCH_C]),
        .dimming = &(com_objects[COM_OBJECT_CHANNEL2_DIMMING]),
        .value = &(com_objects[COM_OBJECT_CHANNEL2_VALUE])
    },
    {
        .channel = 3,
        .switch_a = &(com_objects[COM_OBJECT_CHANNEL3_SWITCH_A]),
        .switch_b = &(com_objects[COM_OBJECT_CHANNEL3_SWITCH_B]),
        .switch_c = &(com_objects[COM_OBJECT_CHANNEL3_SWITCH_C]),
        .dimming = &(com_objects[COM_OBJECT_CHANNEL3_DIMMING]),
        .value = &(com_objects[COM_OBJECT_CHANNEL3_VALUE])
    },
    {
        .channel = 4,
        .switch_a = &(com_objects[COM_OBJECT_CHANNEL4_SWITCH_A]),
        .switch_b = &(com_objects[COM_OBJECT_CHANNEL4_SWITCH_B]),
        .switch_c = &(com_objects[COM_OBJECT_CHANNEL4_SWITCH_C]),
        .dimming = &(com_objects[COM_OBJECT_CHANNEL4_DIMMING]),
        .value = &(com_objects[COM_OBJECT_CHANNEL4_VALUE])
    },
    {
        .channel = 5,
        .switch_a = &(com_objects[COM_OBJECT_CHANNEL5_SWITCH_A]),
        .switch_b = &(com_objects[COM_OBJECT_CHANNEL5_SWITCH_B]),
        .switch_c = &(com_objects[COM_OBJECT_CHANNEL5_SWITCH_C]),
        .dimming = &(com_objects[COM_OBJECT_CHANNEL5_DIMMING]),
        .value = &(com_objects[COM_OBJECT_CHANNEL5_VALUE])
    }
};

device_sensor_t device_sensors[] = {
    {
        .channel = 0,
        .value = &(com_objects[COM_OBJECT_SENSOR0_VALUE]),
        .dpt = KNX_DPT_9
    },
    {
        .channel = 1,
        .value = &(com_objects[COM_OBJECT_SENSOR1_VALUE]),
        .dpt = KNX_DPT_9
    },
    {
        .channel = 2,
        .value = &(com_objects[COM_OBJECT_SENSOR2_VALUE]),
        .dpt = KNX_DPT_14
    },
    {
        .channel = 3,
        .value = &(com_objects[COM_OBJECT_SENSOR3_VALUE]),
        .dpt = KNX_DPT_9
    }
};

buttons_reg_entry_t buttons[] = {
    /* programming button */
    {
        .pin = PB0_PIN,
        .max_repeat = 3,
        .cb = _cb_buttons_prog,
        .enabled = true,
        .invert = true,
        .pull = true,
        .debounce_time = 30,
        .press_time = 200,
        .long_press_time = 600,
        .longer_press_time = 1000,
    },

    /* interface buttons */
    {
        .pin = IF0_PIN,
        .max_repeat = 3,
        .cb = _cb_buttons_interface,
        .cb_arg = &(device_buttons[0])
    },
    {
        .pin = IF1_PIN,
        .max_repeat = 3,
        .cb = _cb_buttons_interface,
        .cb_arg = &(device_buttons[1])
    },
    {
        .pin = IF2_PIN,
        .max_repeat = 3,
        .cb = _cb_buttons_interface,
        .cb_arg = &(device_buttons[2])
    },
    {
        .pin = IF3_PIN,
        .max_repeat = 3,
        .cb = _cb_buttons_interface,
        .cb_arg = &(device_buttons[3])
    },
    {
        .pin = IF4_PIN,
        .max_repeat = 3,
        .cb = _cb_buttons_interface,
        .cb_arg = &(device_buttons[4])
    },
    {
        .pin = IF5_PIN,
        .max_repeat = 3,
        .cb = _cb_buttons_interface,
        .cb_arg = &(device_buttons[5])
    }
};

sensors_reg_entry_t sensors[] = {
    /* temperature */
    {
        .saul_name = "coretemp",
        .enabled = true,
        .cb = _cb_sensors,
        .cb_arg = &(device_sensors[0]),
        .smoothing_algorithm = SENSORS_SMOOTHING_ALGORITHM_MA,
        .smoothing_ema_alpha = 10,
        .smoothing_ma_samples = 10
    },
    /* relative humidity */
    {
        .saul_name = "",
        .enabled = true,
        .cb = _cb_sensors,
        .cb_arg = &(device_sensors[1])
    },
    /* TVOC */
    {
        .saul_name = "",
        .enabled = true,
        .cb = _cb_sensors,
        .cb_arg = &(device_sensors[2])
    },
    /* pressure */
    {
        .saul_name = "",
        .enabled = true,
        .cb = _cb_sensors,
        .cb_arg = &(device_sensors[3])
    },
};

knx_device_t device = {
    .address = KNX_ADDR_UNDEFINED,
    .mask_version = KNX_DEVICE_MASK_0705,

    .info = &device_table,

    .segments = segments,
    .objects = objects,
    .associations = &associations,
    .com_objects = com_objects,
};

static void _knx_restart(knx_device_t *dev)
{
    (void)dev;

    /* update storage */
    storage_acquire();
    storage_write();
    storage_release();

    /* reboot device */
    pm_reboot();
}

static void _knx_mem_write(knx_device_t *dev, knx_event_mem_write_t *event)
{
    (void)dev;

    // DEBUG("[device] _mem_write: segment address %04x\n", event->segment->start);

    /* mark segment as modified */
    event->segment->flags |= KNX_MEMORY_FLAG_MODIFIED;

    /* process any changes to specific memory segments */
    if (event->segment == MEMORY_SEGMENT(DEVICE_TABLE)) {
        msg_t msg = {
            .type = MSG_DEVICE_PROGRAMMING_MODE,
            .content.value = device_table.programming_mode
        };

        msg_send(&msg, pid);
    }
    else {
        // DEBUG("[device] _mem_write: unhandled\n");
    }
}

static void _knx_prop_read(knx_device_t *dev, knx_event_prop_read_t *event)
{
    (void)dev;
    (void)event;
}

static void _knx_prop_write(knx_device_t *dev, knx_event_prop_write_t *event)
{
    knx_property_t *prop = event->property;

    if (prop == &(properties_ota[3])) {
       ota_write(dev, (ota_event_t *)event->data);
    }
    else if (prop->type == KNX_PROPERTY_TYPE_CONTROL) {
        DEBUG("[device] _prop_write: data is %02x\n", event->data[0]);

        uint8_t *state = (uint8_t *)prop->content.ptr;

        switch (event->data[0]) {
            case 0x01: /* loading -> 0x02 */
                *state = 0x02;
                break;
            case 0x02: /* loaded -> 0x01 */
                *state = 0x01;
                break;
            case 0x03: /* task -> 0x02 */
                *state = 0x02;
                break;
            case 0x04: /* unload -> 0x00 */
                *state = 0x00;
                break;
            default:
                DEBUG("[device] _knx_prop_write: unhandled\n");
        }
    }
    else {
        DEBUG("[device] _knx_prop_write: unhandled\n");
    }
}

static void _knx_com_object_write(knx_device_t *dev, knx_event_com_object_write_t *event)
{
    (void)dev;

    DEBUG("[device] _com_object_write: object id=%p\n", event->com_object);

    if (event->com_object == COM_OBJECT(SYSTEM_TIME)) {
        struct tm time;

        knx_dpt_from_dpt19xxx(values.date_time, &time);

        rtc_set_time(&time);

        DEBUG("[device] _com_object_write: datetime=%02x%02x%02x%02x%02x%02x%02x%02x\n", values.date_time[0], values.date_time[1], values.date_time[2], values.date_time[3],
              values.date_time[4], values.date_time[5], values.date_time[6], values.date_time[7]);
    }
    else {
        msg_t msg = {
            .type = MSG_DEVICE_COM_OBJECT,
            .content.ptr = event->com_object
        };

        msg_send(&msg, pid);
    }
}

static void _knx_set_address(knx_device_t *dev, knx_event_set_address_t *event)
{
    if (knx_addr_equal(&(dev->address), event->address)) {
        DEBUG("[device] _set_address: address is the same\n");
        return;
    }

    /* set address */
    memcpy(&(device.address), event->address, sizeof(device.address));

    if (netif_set_opt(device.iface, NETOPT_ADDRESS, 0, &(device.address), sizeof(device.address)) < 0) {
        DEBUG("[main] main: unable to set addr\n");
        return;
    }

    /* store address */
    storage_acquire();
    storage_write();
    storage_release();
}

static void _buttons_toggle(device_button_t *device_button)
{
    bool value;

    knx_com_object_read(device_button->value, &value, 1);
    value = !value;
    knx_com_object_write(device_button->switch_a, &value, 1);
    knx_com_object_write(device_button->value, &value, 1);

    /* send an update */
    gnrc_knx_l7_update_com_object(&device, device_button->switch_a);
}

static void _buttons_dim(device_button_t *device_button, int8_t direction)
{
    /* determine the dimming value */
    uint8_t value;

    if (direction == 1) {
        value = 0x09;
    }
    else if (direction == -1) {
        value = 0x01;
    }
    else {
        value = 0x00;
    }

    knx_com_object_write(device_button->dimming, &value, 1);

    /* send an update */
    gnrc_knx_l7_update_com_object(&device, device_button->dimming);
}

static void _cb_buttons_interface(buttons_event_t event, uint8_t repeat, void *arg)
{
    device_button_t *device_button = (device_button_t *)arg;

    DEBUG("[device] _cb_buttons_interface: channel=%d function=%d\n",
          device_button->channel,
          settings.channels[device_button->channel].function);

    /* handle event based on action */
    switch (settings.channels[device_button->channel].function) {
        case DEVICE_BUTTON_FUNCTION_SWITCH:
            _buttons_toggle(device_button);

            break;
        case DEVICE_BUTTON_FUNCTION_INTELLIGENT_DIM:
            if (event == BUTTONS_EVENT_PRESS) {
                _buttons_toggle(device_button);
            }
            else if (event == BUTTONS_EVENT_LONG_PRESS_START) {
                if (repeat == 1) {
                    _buttons_dim(device_button, 1);
                }
                else {
                    _buttons_dim(device_button, -1);
                }
            }
            else if (event == BUTTONS_EVENT_LONG_PRESS_STOP) {
                _buttons_dim(device_button, 0);
            }

            break;
        default:
            break;
    }
}

/**
 * @brief   Callback for the programming button
 */
static void _cb_buttons_prog(buttons_event_t event, uint8_t repeat, void *arg)
{
    (void)repeat;
    (void)arg;

    if (event == BUTTONS_EVENT_LONGER_PRESS_START) {
        if (repeat == 0) {
            DEBUG("[device] _cb_buttons_prog: toggeling programming mode\n");

            msg_t msg = {
                .type = MSG_DEVICE_PROGRAMMING_MODE,
                .content.value = true
            };

            msg_send(&msg, pid);
        }
        else if (repeat == 1) {
            // Not action defined.
        }
        else if (repeat == 2) {
            DEBUG("[device] _cb_buttons_prog: rebooting\n");

            _knx_restart(NULL);
        }
    }
}

static void _cb_sensors(phydat_t *result, void *arg)
{
    device_sensor_t *device_sensor = (device_sensor_t *)arg;

    DEBUG("[device] _cb_sensors: channel=%d dpt=%d\n",
          device_sensor->channel,
          device_sensor->dpt);

    /* update communication objects based on DPT */
    uint8_t data[4];
    float value;

    switch (device_sensor->dpt) {
        case KNX_DPT_9:
            value = (float)result->val[0];

            knx_dpt_to_dpt14xxx(value, data);
            knx_com_object_write(device_sensor->value, &data, 4);

            break;
        case KNX_DPT_14:
            value = (float)result->val[0] / 100.0;

            knx_dpt_to_dpt9xxx(value, data);
            knx_com_object_write(device_sensor->value, &data, 2);

            break;
        default:
            DEBUG("[device] _cb_sensors: unable to encode.\n");
            return;
    }

    /* send an update */
    gnrc_knx_l7_update_com_object(&device, device_sensor->value);
}

static void _knx_cb(knx_device_t *dev, knx_device_event_t event, void *args)
{
    // DEBUG("[device] _knx_cb: received event %d with args %p\n", event, args);

    switch (event) {
        case KNX_DEVICE_EVENT_RESTART:
            _knx_restart(dev);
            break;
        case KNX_DEVICE_EVENT_MEM_WRITE:
            _knx_mem_write(dev, (knx_event_mem_write_t *)args);
            break;
        case KNX_DEVICE_EVENT_PROP_READ:
            _knx_prop_read(dev, (knx_event_prop_read_t *)args);
            break;
        case KNX_DEVICE_EVENT_PROP_WRITE:
            _knx_prop_write(dev, (knx_event_prop_write_t *)args);
            break;
        case KNX_DEVICE_EVENT_COM_OBJECT_WRITE:
            _knx_com_object_write(dev, (knx_event_com_object_write_t *)args);
            break;
        case KNX_DEVICE_EVENT_SET_ADDRESS:
            _knx_set_address(dev, (knx_event_set_address_t *)args);
            break;
        default:
            DEBUG("[device] _knx_cb: event not handled\n");
    }
}

static void *_event_loop(void *args)
{
    msg_t msg, msg_q[DEVICE_MSG_QUEUE_SIZE];

    (void)args;
    msg_init_queue(msg_q, DEVICE_MSG_QUEUE_SIZE);

    /* start event loop */
    while (1) {
        // DEBUG("[sensors]: _event_loop: waiting for incoming message.\n");
        msg_receive(&msg);

        switch (msg.type) {
            case MSG_DEVICE_PROGRAMMING_MODE:
                device.info->programming_mode = !!msg.content.value;
                gpio_write(LED0_PIN, !!msg.content.value);
                break;
            case MSG_DEVICE_COM_OBJECT:
                break;
            default:
                DEBUG("[sensors] _event_loop: received unidentified message\n");
                break;
        }
    }

    return NULL;
}

void device_reset(void)
{
    /* generate device serial number (prepended with manufacturer id) */
    uint8_t cpuid[CPUID_LEN];
    uint8_t hash[SHA256_DIGEST_LENGTH];

    cpuid_get(cpuid);
    sha256(cpuid, sizeof(cpuid), &hash);

    memcpy(&(device.info->serial[0]), &(device.info->manufacturer_id), 2);
    memcpy(&(device.info->serial[2]), hash, 4);
}

void device_load(void)
{
    /* copy button settings from memory (buttons 1-6 are interface) */
    memory_settings_t *settings = MEMORY_SEGMENT(SETTINGS)->ptr;

    for (unsigned i = 0; i < 6; i++) {
        buttons[i + 1].enabled = (bool)settings->channels[i].enabled;
        buttons[i + 1].invert = (bool)settings->channels[i].invert;
        buttons[i + 1].pull = (bool)settings->channels[i].pull;

        buttons[i + 1].debounce_time = byteorder_ntohs(settings->timings.debounce);
        buttons[i + 1].press_time = byteorder_ntohs(settings->timings.press);
        buttons[i + 1].long_press_time = byteorder_ntohs(settings->timings.long_press);
        buttons[i + 1].longer_press_time = byteorder_ntohs(settings->timings.longer_press);
    }

    /* copy sensor settings from memory */
    for (unsigned i = 0; i < ARRAY_SIZE(sensors); i++) {
        sensors[i].enabled = (bool)settings->sensors[i].enabled;
        sensors[i].smart_update = (bool)settings->sensors[i].smart_update;
        sensors[i].difference = settings->sensors[i].difference;
        sensors[i].update_time = byteorder_ntohs(settings->sensors[i].update_time);
        sensors[i].update_time_max = byteorder_ntohs(settings->sensors[i].update_time_max);
    }

    /* register buttons */
    for (unsigned i = 0; i < ARRAY_SIZE(buttons); i++) {
        buttons_register(&buttons[i]);
    }

    /* register sensors */
    for (unsigned i = 0; i < ARRAY_SIZE(sensors); i++) {
        sensors_register(&sensors[i]);
    }
}

void device_init(netif_t *iface)
{
    memory_settings_t *settings = MEMORY_SEGMENT(SETTINGS)->ptr;

    device.event_callback = _knx_cb;
    device.iface = iface;

    /* load the communication objects from the table */
    knx_com_object_update(com_objects, &com_object_table, 35);

    /* prepare the association table */
    knx_assoc_update(device.associations,
                     device.com_objects,
                     &assoc_table,
                     &addr_table,
                     ARRAY_SIZE(mappings));

    /* sleep here if boot delay is requested */
    uint32_t sleep = byteorder_ntohs(settings->general.startup_delay);

    if (sleep > 30000) {
        sleep = 30000;
    }

    if (settings->general.randomize_startup_delay) {
        sleep = random_uint32_range(0, sleep);
    }

    DEBUG("[device] device_init: adding startup delay of %" PRIu32 " ms\n", sleep);
    xtimer_usleep(sleep * US_PER_MS);

    /* setup the different threads that provide functionality */
    pid_buttons = buttons_init();
    //pid_display = display_setup();
    pid_sensors = sensors_init();

    device_load();
    device_setup();
}

kernel_pid_t device_setup(void)
{
    if (pid == KERNEL_PID_UNDEF) {
        pid = thread_create(_stack, sizeof(_stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_WOUT_YIELD | THREAD_CREATE_STACKTEST, _event_loop, NULL, "device");
    }

    return pid;
}
