/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
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
 * @brief       Unittests for the ``nrfnet`` module
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */
#ifndef TESTS_NRFNET_H_
#define TESTS_NRFNET_H_

#include "embUnit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   The entry point of this test suite.
 */
void tests_nrfnet(void);

/**
 * @brief   The entry point for the address info tests.
 */
Test *tests_nrfnet_addr_info_tests(void);

/**
 * @brief   The entry point for the address tests.
 */
Test *tests_nrfnet_addr_tests(void);

/**
 * @brief   The entry point for the header tests.
 */
Test *tests_nrfnet_hdr_tests(void);

/**
 * @brief   The entry point for the reassembly buffer tests.
 */
Test *tests_nrfnet_rbuf_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* TESTS_NRFNET_H_ */
/** @} */
