/*
 * Copyright (C) 2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_efm32_drivers_coretemp
 * @{
 *
 * @file
 * @brief       SAUL adoption for EFM32 internal temperature sensor
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "coretemp.h"

#include "saul_reg.h"

static int _read(const void *dev, phydat_t *res)
{
    (void)dev;

    res->val[0] = (int16_t)coretemp_read();
    res->unit = UNIT_TEMP_C;
    res->scale = -2;

    return 1;
}

const saul_driver_t coretemp_temperature_saul_driver = {
    .read = _read,
    .write = saul_notsup,
    .type = SAUL_SENSE_TEMP
};

const saul_reg_t saul_entry = {
    .name = "coretemp",
    .driver = &coretemp_temperature_saul_driver
};