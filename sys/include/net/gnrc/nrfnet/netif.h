/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_gnrc_nrfnet_netif  nRFNet network interfaces
 * @ingroup     net_gnrc_nrfnet
 * @brief       nRFNet specific information on @ref net_gnrc_netif
 * @{
 *
 * @file
 * @brief   Definitions for nRFNet specific information of network interfaces.
 *
 * @author  Bas Stottelaar <basstottelaar@gmail.com>
 */
#ifndef GNRC_NRFNET_NETIF_H_
#define GNRC_NRFNET_NETIF_H_

#include <stdbool.h>

#include "kernel_types.h"

#include "net/nrfnet.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Definition of nRFNet interface type.
 */
typedef struct {
    kernel_pid_t pid;       /**< PID of the interface */
    uint16_t max_frag_size; /**< Maximum fragment size for this interface */
} gnrc_nrfnet_netif_t;

/**
 * @brief   Initializes the module
 */
void gnrc_nrfnet_netif_init(void);

/**
 * @brief   Add interface to nRFNet.
 *
 * @param[in] pid           The PID to the interface.
 * @param[in] max_frag_size The maximum fragment size for this interface.
 */
void gnrc_nrfnet_netif_add(kernel_pid_t pid, uint16_t max_frag_size);


nrfnet_addr_t *gnrc_nrfnet_netif_add_addr(kernel_pid_t pid, const nrfnet_addr_t *addr);

/**
 * @brief   Remove interface from nRFNet.
 *
 * @param[in] pid   The PID to the interface.
 */
void gnrc_nrfnet_netif_remove(kernel_pid_t pid);

/**
 * @brief   Get interface.
 *
 * @param[in] pid   The PID to the interface
 *
 * @return  The interface describing structure, on success.
 * @return  NULL, if there is no interface with PID @p pid.
 */
gnrc_nrfnet_netif_t *gnrc_nrfnet_netif_get(kernel_pid_t pid);

#ifdef __cplusplus
}
#endif

#endif /* GNRC_NRFNET_NETIF_H_ */
/** @} */
