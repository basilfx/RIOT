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
 * @brief       Auto initialization for NCN5120 transceiver interfaces
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#include "log.h"
#include "board.h"
#include "net/gnrc/netif/knx.h"
#include "net/gnrc.h"

#include "ncn5120.h"
#include "ncn5120_netdev.h"
#include "ncn5120_params.h"

#define ENABLE_DEBUG 0
#include "debug.h"

/**
 * @brief   Calculate the number of configured NCN5120 devices
 */
#define NCN5120_NUMOF       ARRAY_SIZE(ncn5120_params)

/**
 * @brief   Define stack parameters for the MAC layer thread
 */
#ifndef NCN5120_STACKSIZE
#define NCN5120_STACKSIZE   (THREAD_STACKSIZE_DEFAULT + DEBUG_EXTRA_STACKSIZE)
#endif

#ifndef NCN5120_PRIO
#define NCN5120_PRIO        (GNRC_NETIF_PRIO)
#endif

/**
 * @brief   Allocate memory for network devices.
 */
static netdev_ncn5120_t ncn5120_devs[NCN5120_NUMOF];

/**
 * @brief   Allocate memory for the network interfaces
 */
static gnrc_netif_t ncn5120_netifs[NCN5120_NUMOF];

/**
 * @brief   Allocate memory for stacks
 */
static char ncn5120_stacks[NCN5120_NUMOF][NCN5120_STACKSIZE];

void auto_init_ncn5120(void)
{
    for (unsigned i = 0; i < NCN5120_NUMOF; i++) {
        LOG_DEBUG("[auto_init_netif] initializing ncn5120 #%u\n", i);

        int res = netdev_ncn5120_setup(&ncn5120_devs[i], &ncn5120_params[i]);

        if (res != NCN5120_OK) {
            LOG_ERROR("[auto_init_netif] error initializing ncn5120 #%u\n", i);
            continue;
        }

        gnrc_netif_knx_create(&ncn5120_netifs[i], ncn5120_stacks[i],
                              NCN5120_STACKSIZE, NCN5120_PRIO,
                              "ncn5120", (netdev_t *)&ncn5120_devs[i]);
    }
}
/** @} */
