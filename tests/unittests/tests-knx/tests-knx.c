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
 * @brief       Central setup for the KNX module unit tests
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "tests-knx.h"

void tests_knx(void)
{
    TESTS_RUN(tests_knx_addr_tests());
    TESTS_RUN(tests_knx_apci_tests());
    TESTS_RUN(tests_knx_telegram_tests());
    TESTS_RUN(tests_knx_tpci_tests());
}
