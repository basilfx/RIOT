/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_knx    KNX
 * @ingroup     net
 * @brief       KNX types and helper functions
 *
 * @see         KNX Specification version 2.1
 *
 * # About
 * This module implements types and methods for working with the KNX stack.
 * KNX is a open standard for building automation (commercial and domestic).
 *
 * # Stack
 * The stack implemented in separate modules, using @ref net_gnrc_netapi:
 *
 * * Network layer: @ref net_gnrc_knx_l3
 * * Transport layer: @ref net_gnrc_knx_l4
 * * Application layer: @ref net_gnrc_knx_l7
 *
 * @{
 * @file
 * @brief       KNX type and helper function definitions
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */
#ifndef NET_KNX_H
#define NET_KNX_H

#include "net/knx/addr.h"
#include "net/knx/apci.h"
#include "net/knx/dpt.h"
#include "net/knx/telegram.h"
#include "net/knx/tpci.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* NET_KNX_H */
/** @} */
