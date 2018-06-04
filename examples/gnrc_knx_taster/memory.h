/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef MEMORY_H
#define MEMORY_H

#include "knx_device.h"
#include "byteorder.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MEMORY_SEGMENT_DEVICE_TABLE     (0)
#define MEMORY_SEGMENT_ADDR_TABLE       (1)
#define MEMORY_SEGMENT_ASSOC_TABLE      (2)
#define MEMORY_SEGMENT_COM_OBJECT_TABLE (3)
#define MEMORY_SEGMENT_SETTINGS         (4)
#define MEMORY_SEGMENT_OTA              (5)
#define MEMORY_SEGMENT_LOAD_STATE_TABLE (6)

#define MEMORY_SEGMENT(x) (&(segments[MEMORY_SEGMENT_ ## x]))

typedef struct __attribute__((packed)) {
    struct {
        /**< 0x0000 (0): Settings -> General_StartupTime (16 bits, enumeration) */
        be_uint16_t startup_delay;

        /**< 0x0002 (2): Settings -> General_RandomStartupTime (1 bits, unsignedInt) */
        uint8_t randomize_startup_delay;
    } general;

    uint8_t dummy1[5];

    /**< 0x0008 (8): Settings -> Channel0_Enabled (1 bits, unsignedInt) */
    /**< 0x0009 (9): Settings -> Channel0_Function (8 bits, enumeration) */
    /**< 0x000a (10): Settings -> Channel0_Invert (1 bits, unsignedInt) */
    /**< 0x000b (11): Settings -> Channel0_PullUpDown (1 bits, unsignedInt) */
    /**< 0x0018 (24): Settings -> Channel1_Enabled (1 bits, unsignedInt) */
    /**< 0x0019 (25): Settings -> Channel1_Function (8 bits, enumeration) */
    /**< 0x001a (26): Settings -> Channel1_Invert (1 bits, unsignedInt) */
    /**< 0x001b (27): Settings -> Channel1_PullUpDown (1 bits, unsignedInt) */
    /**< 0x0028 (40): Settings -> Channel2_Enabled (1 bits, unsignedInt) */
    /**< 0x0029 (41): Settings -> Channel2_Function (8 bits, enumeration) */
    /**< 0x002a (42): Settings -> Channel2_Invert (1 bits, unsignedInt) */
    /**< 0x002b (43): Settings -> Channel2_PullUpDown (1 bits, unsignedInt) */
    /**< 0x0038 (56): Settings -> Channel3_Enabled (1 bits, unsignedInt) */
    /**< 0x0039 (57): Settings -> Channel3_Function (8 bits, enumeration) */
    /**< 0x003a (58): Settings -> Channel3_Invert (1 bits, unsignedInt) */
    /**< 0x003b (59): Settings -> Channel3_PullUpDown (1 bits, unsignedInt) */
    /**< 0x0048 (72): Settings -> Channel4_Enabled (1 bits, unsignedInt) */
    /**< 0x0049 (73): Settings -> Channel4_Function (8 bits, enumeration) */
    /**< 0x004a (74): Settings -> Channel4_Invert (1 bits, unsignedInt) */
    /**< 0x004b (75): Settings -> Channel4_PullUpDown (1 bits, unsignedInt) */
    /**< 0x0058 (88): Settings -> Channel5_Enabled (1 bits, unsignedInt) */
    /**< 0x0059 (89): Settings -> Channel5_Function (8 bits, enumeration) */
    /**< 0x005a (90): Settings -> Channel5_Invert (1 bits, unsignedInt) */
    /**< 0x005b (91): Settings -> Channel5_PullUpDown (1 bits, unsignedInt) */
    struct {
        uint8_t enabled;
        uint8_t function;
        uint8_t invert;
        uint8_t pull;
        uint8_t dummy[12];
    } channels[6];

    uint8_t dummy2[24];

    struct {
        /**< 0x0080 (128): Settings -> Timing_Debounce (16 bits, unsignedInt) */
        be_uint16_t debounce;

        /**< 0x0082 (130): Settings -> Timing_Press (16 bits, unsignedInt) */
        be_uint16_t press;

        /**< 0x0084 (132): Settings -> Timing_LongPress (16 bits, unsignedInt) */
        be_uint16_t long_press;

        /**< 0x0086 (134): Settings -> Timing_LongerPress (16 bits, unsignedInt) */
        be_uint16_t longer_press;
    } timings;

    uint8_t dummy3[120];

    /**< 0x0100 (256): Settings -> Sensor0_Enabled (1 bits, unsignedInt) */
    /**< 0x0101 (257): Settings -> Sensor0_SmartUpdate (1 bits, unsignedInt) */
    /**< 0x0102 (258): Settings -> Sensor0_Difference (8 bits, enumeration) */
    /**< 0x0104 (260): Settings -> Sensor0_UpdateTime (16 bits, enumeration) */
    /**< 0x0106 (262): Settings -> Sensor0_UpdateTimeMax (16 bits, enumeration) */
    /**< 0x0108 (264): Settings -> Sensor0_Source (128 bits, text) */
    /**< 0x0118 (280): Settings -> Sensor0_SmoothingAlgorithm (8 bits, enumeration) */
    /**< 0x0119 (281): Settings -> Sensor0_SmoothingEMAAlpha (8 bits, enumeration) */
    /**< 0x011a (282): Settings -> Sensor0_SmoothingMASamples (8 bits, enumeration) */
    /**< 0x0120 (288): Settings -> Sensor1_Enabled (1 bits, unsignedInt) */
    /**< 0x0121 (289): Settings -> Sensor1_SmartUpdate (1 bits, unsignedInt) */
    /**< 0x0122 (290): Settings -> Sensor1_Difference (8 bits, enumeration) */
    /**< 0x0124 (292): Settings -> Sensor1_UpdateTime (16 bits, enumeration) */
    /**< 0x0126 (294): Settings -> Sensor1_UpdateTimeMax (16 bits, enumeration) */
    /**< 0x0128 (296): Settings -> Sensor1_Source (128 bits, text) */
    /**< 0x0138 (312): Settings -> Sensor1_SmoothingAlgorithm (8 bits, enumeration) */
    /**< 0x0139 (313): Settings -> Sensor1_SmoothingEMAAlpha (8 bits, enumeration) */
    /**< 0x013a (314): Settings -> Sensor1_SmoothingMASamples (8 bits, enumeration) */
    /**< 0x0140 (320): Settings -> Sensor2_Enabled (1 bits, unsignedInt) */
    /**< 0x0141 (321): Settings -> Sensor2_SmartUpdate (1 bits, unsignedInt) */
    /**< 0x0142 (322): Settings -> Sensor2_Difference (8 bits, enumeration) */
    /**< 0x0144 (324): Settings -> Sensor2_UpdateTime (16 bits, enumeration) */
    /**< 0x0146 (326): Settings -> Sensor2_UpdateTimeMax (16 bits, enumeration) */
    /**< 0x0148 (328): Settings -> Sensor2_Source (128 bits, text) */
    /**< 0x0158 (344): Settings -> Sensor2_SmoothingAlgorithm (8 bits, enumeration) */
    /**< 0x0159 (345): Settings -> Sensor2_SmoothingEMAAlpha (8 bits, enumeration) */
    /**< 0x015a (346): Settings -> Sensor2_SmoothingMASamples (8 bits, enumeration) */
    /**< 0x0160 (352): Settings -> Sensor3_Enabled (1 bits, unsignedInt) */
    /**< 0x0161 (353): Settings -> Sensor3_SmartUpdate (1 bits, unsignedInt) */
    /**< 0x0162 (354): Settings -> Sensor3_Difference (8 bits, enumeration) */
    /**< 0x0164 (356): Settings -> Sensor3_UpdateTime (16 bits, enumeration) */
    /**< 0x0166 (358): Settings -> Sensor3_UpdateTimeMax (16 bits, enumeration) */
    /**< 0x0168 (360): Settings -> Sensor3_Source (128 bits, text) */
    /**< 0x0178 (376): Settings -> Sensor3_SmoothingAlgorithm (8 bits, enumeration) */
    /**< 0x0179 (377): Settings -> Sensor3_SmoothingEMAAlpha (8 bits, enumeration) */
    /**< 0x017a (378): Settings -> Sensor3_SmoothingMASamples (8 bits, enumeration) */
    struct {
        uint8_t enabled;
        uint8_t smart_update;
        uint8_t difference;
        uint8_t dummy1[1];
        be_uint16_t update_time;
        be_uint16_t update_time_max;
        uint8_t source[16];
        uint8_t smoothing;
        uint8_t smoothing_ema_alpha;
        uint8_t smoothing_ma_samples;
        uint8_t dummy2[5];
    } sensors[4];
} memory_settings_t;

extern knx_table_device_t device_table;
extern knx_table_addr_t addr_table;
extern knx_table_assoc_t assoc_table;
extern knx_table_com_objects_t com_object_table;
extern knx_table_load_state_t load_state_table;

extern memory_settings_t settings;
extern uint8_t ota[2048];

extern knx_memory_segment_t segments[];

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_H */
/** @} */
