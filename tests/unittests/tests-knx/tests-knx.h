/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @addtogroup  unit tests
 * @{
 *
 * @file
 * @brief       Unittests for the ``knx`` module
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */
#ifndef TESTS_KNX_H
#define TESTS_KNX_H

#include "embUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   The entry point of this test suite.
 */
void tests_knx(void);

/**
 * @brief   KNX address related tests.
 */
Test *tests_knx_addr_tests(void);

/**
 * @brief   KNX APCI related tests.
 */
Test *tests_knx_apci_tests(void);

/**
 * @brief   KNX telegram related tests.
 */
Test *tests_knx_telegram_tests(void);

/**
 * @brief   KNX TPCI related tests.
 */
Test *tests_knx_tpci_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* TESTS_KNX_H */
/** @} */
