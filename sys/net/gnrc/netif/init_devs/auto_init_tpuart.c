/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_auto_init_gnrc_netif
 * @{
 *
 * @file
 * @brief       Auto initialization for TPUART transceiver interfaces
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#include "log.h"
#include "board.h"
#include "net/gnrc/netif/knx.h"
#include "net/gnrc.h"

#include "tpuart.h"
#include "tpuart_netdev.h"
#include "tpuart_params.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief   Calculate the number of configured TPUART devices
 */
#define TPUART_NUMOF        ARRAY_SIZE(tpuart_params)

/**
 * @brief   Define stack parameters for the MAC layer thread
 */
#ifndef TPUART_STACKSIZE
#define TPUART_STACKSIZE    (THREAD_STACKSIZE_DEFAULT + DEBUG_EXTRA_STACKSIZE)
#endif

#ifndef TPUART_PRIO
#define TPUART_PRIO         (GNRC_NETIF_PRIO)
#endif

/**
 * @brief   Allocate memory for network devices.
 */
static netdev_tpuart_t tpuart_devs[TPUART_NUMOF];

/**
 * @brief   Allocate memory for the network interfaces
 */
static gnrc_netif_t tpuart_netifs[TPUART_NUMOF];

/**
 * @brief   Allocate memory for stacks
 */
static char tpuart_stacks[TPUART_NUMOF][TPUART_STACKSIZE];

void auto_init_tpuart(void)
{
    for (unsigned i = 0; i < TPUART_NUMOF; i++) {
        LOG_DEBUG("[auto_init_netif] initializing tpuart #%u\n", i);

        int res = netdev_tpuart_setup(&tpuart_devs[i], &tpuart_params[i]);

        if (res != TPUART_OK) {
            LOG_ERROR("[auto_init_netif] error initializing tpuart #%u\n", i);
            continue;
        }

        gnrc_netif_knx_create(&tpuart_netifs[i], tpuart_stacks[i],
                              TPUART_STACKSIZE, TPUART_PRIO,
                              "tpuart", (netdev_t *)&tpuart_devs[i]);
    }
}
/** @} */
