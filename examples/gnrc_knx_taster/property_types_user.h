/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef PROPERTY_TYPES_USER_H
#define PROPERTY_TYPES_USER_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    /**< OTA update object */
    OT_USER_OTA     = 65300,

    /**< Remote shell object */
    OT_USER_SHELL   = 65301
};

enum {
    /**< Firmware slot */
    PID_USER_OTA_SLOT     = 31,

    /**< Firmware size */
    PID_USER_OTA_SIZE     = 32,

    /**< OTA flash control */
    PID_USER_OTA_FLASH    = 33,

    /**< OTA state */
    PID_USER_OTA_STATE    = 34,

    /**< Shell channel property */
    PID_USER_SHELL_PIPE   = 41,
};

#ifdef __cplusplus
}
#endif

#endif /* PROPERTY_TYPES_USER_H */
/** @} */
