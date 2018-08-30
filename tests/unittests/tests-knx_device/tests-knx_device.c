/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     unittests
 * @{
 *
 * @file
 * @brief       Central setup for the KNX device module unit tests
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "tests-knx_device.h"

void tests_knx_device(void)
{
    TESTS_RUN(tests_knx_assoc());
    TESTS_RUN(tests_knx_com_object());
    TESTS_RUN(tests_knx_memory());
    TESTS_RUN(tests_knx_property_tests());
}
