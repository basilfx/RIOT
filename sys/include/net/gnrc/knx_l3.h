/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gnrc_knx_l3 KNX Layer 3
 * @ingroup     net_gnrc
 * @brief       KNX Layer 3 interface using the GNRC stack
 *
 * @{
 * @file
 * @brief       GNRC KNX Layer 3 API
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NET_GNRC_KNX_L3_H
#define NET_GNRC_KNX_L3_H

#include "kernel_types.h"
#include "net/gnrc.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default stack size to use for the KNX thread
 */
#ifndef GNRC_KNX_L3_STACK_SIZE
#define GNRC_KNX_L3_STACK_SIZE        (THREAD_STACKSIZE_DEFAULT)
#endif

/**
 * @brief   Default priority for the IPv6 thread
 */
#ifndef GNRC_KNX_L3_PRIO
#define GNRC_KNX_L3_PRIO              (THREAD_PRIORITY_MAIN - 4)
#endif

/**
 * @brief   Default message queue size to use for the IPv6 thread
 */
#ifndef GNRC_KNX_L3_MSG_QUEUE_SIZE
#define GNRC_KNX_L3_MSG_QUEUE_SIZE    (8U)
#endif

/**
 * @brief   Initialization of the KNX Layer 3 thread
 *
 * If the thread is already initialzed, the same PID will be returned.
 *
 * @return  The PID to the KNX Layer 3 thread
 */
kernel_pid_t gnrc_knx_l3_init(void);

#ifdef __cplusplus
}
#endif

#endif /* NET_GNRC_KNX_L3_H */
/** @} */
