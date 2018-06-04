/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
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

#define PROPERTY(x) (&(com_objects[COM_OBJECT_ ## x]))

extern knx_property_t properties_ota[];

extern knx_property_t properties_shell[];

extern knx_property_object_t objects[];

#ifdef __cplusplus
}
#endif

#endif /* PROPERTIES_H */
/** @} */
