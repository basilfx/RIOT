/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef COM_OBJECTS_H
#define COM_OBJECTS_H

#include "knx_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COM_OBJECT_SYSTEM_TIME        (0)

#define COM_OBJECT_CHANNEL0_SWITCH_A  (1)
#define COM_OBJECT_CHANNEL0_SWITCH_B  (2)
#define COM_OBJECT_CHANNEL0_SWITCH_C  (3)
#define COM_OBJECT_CHANNEL0_DIMMING   (4)
#define COM_OBJECT_CHANNEL0_VALUE     (5)

#define COM_OBJECT_CHANNEL1_SWITCH_A  (6)
#define COM_OBJECT_CHANNEL1_SWITCH_B  (7)
#define COM_OBJECT_CHANNEL1_SWITCH_C  (8)
#define COM_OBJECT_CHANNEL1_DIMMING   (9)
#define COM_OBJECT_CHANNEL1_VALUE     (10)

#define COM_OBJECT_CHANNEL2_SWITCH_A  (11)
#define COM_OBJECT_CHANNEL2_SWITCH_B  (12)
#define COM_OBJECT_CHANNEL2_SWITCH_C  (13)
#define COM_OBJECT_CHANNEL2_DIMMING   (14)
#define COM_OBJECT_CHANNEL2_VALUE     (15)

#define COM_OBJECT_CHANNEL3_SWITCH_A  (16)
#define COM_OBJECT_CHANNEL3_SWITCH_B  (17)
#define COM_OBJECT_CHANNEL3_SWITCH_C  (18)
#define COM_OBJECT_CHANNEL3_DIMMING   (19)
#define COM_OBJECT_CHANNEL3_VALUE     (20)

#define COM_OBJECT_CHANNEL4_SWITCH_A  (21)
#define COM_OBJECT_CHANNEL4_SWITCH_B  (22)
#define COM_OBJECT_CHANNEL4_SWITCH_C  (23)
#define COM_OBJECT_CHANNEL4_DIMMING   (24)
#define COM_OBJECT_CHANNEL4_VALUE     (25)

#define COM_OBJECT_CHANNEL5_SWITCH_A  (26)
#define COM_OBJECT_CHANNEL5_SWITCH_B  (27)
#define COM_OBJECT_CHANNEL5_SWITCH_C  (28)
#define COM_OBJECT_CHANNEL5_DIMMING   (29)
#define COM_OBJECT_CHANNEL5_VALUE     (30)

#define COM_OBJECT_SENSOR0_VALUE      (31)
#define COM_OBJECT_SENSOR1_VALUE      (32)
#define COM_OBJECT_SENSOR2_VALUE      (33)
#define COM_OBJECT_SENSOR3_VALUE      (34)

#define COM_OBJECT(x) (&(com_objects[COM_OBJECT_ ## x]))

typedef struct {
    bool switch_a;
    bool switch_b;
    bool switch_c;
    uint8_t dimming;
    bool value;
} usr_interface_t;

typedef struct {
    uint8_t date_time[8];

    usr_interface_t interface[6];

    uint8_t sensor0_value[2];
    uint8_t sensor1_value[2];
    uint8_t sensor2_value[2];
    uint8_t sensor3_value[4];
} com_objects_t;

extern com_objects_t values;

extern knx_com_object_t com_objects[];


#ifdef __cplusplus
}
#endif

#endif /* COM_OBJECTS_H */
/** @} */
