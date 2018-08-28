/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    tests_gnrc_knx_l3 GNRC KNX Layer 3 stack tests
 * @ingroup     tests
 * @brief       GNRC KNX Layer 3 stack tests
 * @{
 *
 * @file
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#include "net/gnrc.h"
#include "net/gnrc/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

extern gnrc_netif_t *_mock_netif;

void _tests_init(void);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_H */
/** @} */
