/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef GNRC_NRFNET_H_
#define GNRC_NRFNET_H_

#include "kernel_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Message queue size for the nrfnet thread.
 */
#ifndef GNRC_NRFNET_MSG_QUEUE_SIZE
#define GNRC_NRFNET_MSG_QUEUE_SIZE   (8U)
#endif

/**
 * @brief   Priority of the nrfnet thread.
 */
#ifndef GNRC_NRFNET_PRIO
#define GNRC_NRFNET_PRIO             (THREAD_PRIORITY_MAIN - 1)
#endif

/**
 * @brief   Stack size used for the nrfnet thread.
 */
#ifndef GNRC_NRFNET_STACK_SIZE
#define GNRC_NRFNET_STACK_SIZE       (THREAD_STACKSIZE_DEFAULT)
#endif

/**
 * @brief   The number of heads for reassembling packets. A head is allocated
 *          per packet, and may chain multiple holes.
 */
#ifndef GNRC_NRFNET_FRAG_HEADS
#define GNRC_NRFNET_FRAG_HEADS       (16U)
#endif

/**
 * @brief   The number of holes for reassembling packets. A single hole
 *          contains the payload of a single fragment.
 */
#ifndef GNRC_NRFNET_FRAG_HOLES
#define GNRC_NRFNET_FRAG_HOLES       (32U)
#endif

/**
 * @brief   The maximum lifetime (in milliseconds) of an incomplete packet
 *          before it is garbage collected.
 */
#ifndef GNRC_NRFNET_FRAG_EXPIRE
#define GNRC_NRFNET_FRAG_EXPIRE      (10U * 1000U)
#endif

/**
 * @brief   The PID of the nrfnet thread.
 */
extern kernel_pid_t gnrc_nrfnet_pid;

/**
 * @brief   Start the nrfnet thread and start processing packets.
 *
 * @return  PID of the nrfnet thread
 * @return  negative value on error
 */
kernel_pid_t gnrc_nrfnet_init(void);

#ifdef __cplusplus
}
#endif

#endif /* GNRC_NRFNET_H_ */
