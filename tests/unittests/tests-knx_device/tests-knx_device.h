/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup  unittests
 * @{
 *
 * @file
 * @brief       Unit tests for the ``knx_device`` module
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */
#ifndef TESTS_KNX_DEVICE_H
#define TESTS_KNX_DEVICE_H

#include "embUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   The entry point of this test suite.
 */
void tests_knx_device(void);

/**
 * @brief   KNX device association tests.
 */
Test *tests_knx_assoc(void);

/**
 * @brief   KNX device communication object tests.
 */
Test *tests_knx_com_object(void);

/**
 * @brief   KNX device memory tests.
 */
Test *tests_knx_memory(void);

/**
 * @brief   KNX device property tests.
 */
Test *tests_knx_property_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* TESTS_KNX_DEVICE_H */
/** @} */
