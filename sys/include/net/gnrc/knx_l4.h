/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gnrc_knx_l4 KNX Layer 4
 * @ingroup     net_gnrc
 * @brief       KNX Layer 4 interface using the GNRC stack
 *
 * @{
 * @file
 * @brief       GNRC KNX Layer 4 API
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NET_GNRC_KNX_L4_H
#define NET_GNRC_KNX_L4_H

#include "kernel_types.h"
#include "net/gnrc.h"
#include "net/knx.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default stack size to use for the KNX thread
 */
#ifndef GNRC_KNX_L4_STACK_SIZE
#define GNRC_KNX_L4_STACK_SIZE        (THREAD_STACKSIZE_DEFAULT)
#endif

/**
 * @brief   Default priority for the IPv6 thread
 */
#ifndef GNRC_KNX_L4_PRIO
#define GNRC_KNX_L4_PRIO              (THREAD_PRIORITY_MAIN - 3)
#endif

/**
 * @brief   Default message queue size to use for the IPv6 thread
 */
#ifndef GNRC_KNX_L4_MSG_QUEUE_SIZE
#define GNRC_KNX_L4_MSG_QUEUE_SIZE    (8U)
#endif

/**
 * @brief   Definition of a KNX Layer 4 connection
 */
typedef struct {
    bool connected;         /**< Indicates if connected or not */
    uint8_t src_seq;        /**< Expected sequence number of source */
    uint8_t dest_seq;       /**< Next sequence number for sending */
    knx_addr_t dest;        /**< Address of remote */
    uint64_t timestamp;     /**< Timestamp of last message */
} knx_connection_t;

/**
 * @brief   Initialization of the KNX Layer 4 thread
 *
 * If the thread is already initialzed, the same PID will be returned.
 *
 * @return  The PID to the KNX Layer 4 thread
 */
kernel_pid_t gnrc_knx_l4_init(void);

/* for testing */
#ifdef TEST_SUITES
/**
 * @brief   Return the connection instance
 *
 * @note    This method is only for testing purposes
 *
 * @return  Pointer to the connection instance
 */
knx_connection_t *gnrc_knx_l4_get_connection(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* NET_GNRC_KNX_L4_H */
/** @} */
