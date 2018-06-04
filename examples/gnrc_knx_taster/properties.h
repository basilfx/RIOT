/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "knx_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PROPERTY_OBJECT_DEVICE              (0)
#define PROPERTY_OBJECT_ADDRESS_TABLE       (1)
#define PROPERTY_OBJECT_ASSOCIATION_TABLE   (2)
#define PROPERTY_OBJECT_APPLICATION_PROGRAM (3)
#define PROPERTY_OBJECT_INTERFACE_PROGRAM   (4)
#define PROPERTY_OBJECT_USER_OTA            (5)

extern knx_property_t properties_ota[];

extern knx_property_t properties_shell[];

extern knx_property_object_t objects[];

#ifdef __cplusplus
}
#endif

#endif /* PROPERTIES_H */
/** @} */
