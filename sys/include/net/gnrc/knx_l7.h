/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gnrc_knx_l7 KNX Layer 7
 * @ingroup     net_gnrc
 * @brief       KNX Layer 7 interface using the GNRC stack
 *
 * @{
 * @file
 * @brief       GNRC KNX Layer 7 API
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NET_GNRC_KNX_L7_H
#define NET_GNRC_KNX_L7_H

#include "kernel_types.h"
#include "net/gnrc.h"
#include "thread.h"

#include "knx_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default stack size to use for the KNX L7 thread
 */
#ifndef GNRC_KNX_L7_STACK_SIZE
#define GNRC_KNX_L7_STACK_SIZE        (THREAD_STACKSIZE_LARGE)
#endif

/**
 * @brief   Default priority for the KNX L7 thread
 */
#ifndef GNRC_KNX_L7_PRIO
#define GNRC_KNX_L7_PRIO              (THREAD_PRIORITY_MAIN - 2)
#endif

/**
 * @brief   Default message queue size to use for the KNX L7 thread
 */
#ifndef GNRC_KNX_L7_MSG_QUEUE_SIZE
#define GNRC_KNX_L7_MSG_QUEUE_SIZE    (8U)
#endif

/**
 * @brief   Send an update for a communication object
 *
 * The update is not immediately sent, but dispatched to be handled in the
 * KNX Layer 7 thread.
 *
 * Note that a communication object can have multiple associations. For each
 * association, a telegram will ben sent.
 *
 * @param[in] device        The KNX device to use
 * @param[in] com_object    The communication object to send updates for
 */
void gnrc_knx_l7_update_com_object(knx_device_t *device, knx_com_object_t *com_object);

/**
 * @brief   Initialization of the KNX Layer 7 thread
 *
 * If the thread is already initialzed, the same PID will be returned.
 *
 * @param[in] device        The KNX device to initialize
 *
 * @return  The PID to the KNX Layer 7 thread
 */
kernel_pid_t gnrc_knx_l7_init(knx_device_t *device);

#ifdef __cplusplus
}
#endif

#endif /* NET_GNRC_KNX_L7_H */
/** @} */
