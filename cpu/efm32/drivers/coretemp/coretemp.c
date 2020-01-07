/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
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
 * @brief       Implementation of EFM32 internal temperature sensor
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "coretemp.h"

#include "periph/adc.h"

#include "board.h"

int16_t coretemp_read(void)
{
    /* initialize factory calibration values */
#ifdef _SILICON_LABS_32B_SERIES_0
    float cal_temp = (float)((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT);
    float cal_value = (float)((DEVINFO->ADC0CAL2 & _DEVINFO_ADC0CAL2_TEMP1V25_MASK) >> _DEVINFO_ADC0CAL2_TEMP1V25_SHIFT);
#else
    float cal_temp = (float)((DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT);
    float cal_value = (float)((DEVINFO->ADC0CAL3 & _DEVINFO_ADC0CAL3_TEMPREAD1V25_MASK) >> _DEVINFO_ADC0CAL3_TEMPREAD1V25_SHIFT);
#endif

    /* convert temperature channel */
    int32_t value = adc_sample(0, ADC_RES_12BIT);

    /* convert sample to degrees Celsius, using the correction factors */
#ifdef _SILICON_LABS_32B_SERIES_0
    float temperature = cal_temp - ((cal_value - value) / -6.27);
#else
    float temperature = cal_temp - ((cal_value - value) / -6.01);
#endif

    return (int16_t)(temperature * 100.0);
}

int coretemp_init(void)
{
    /* sanity check to ensure  the internal temperature sensor is configured */
#ifdef _SILICON_LABS_32B_SERIES_0
    if (adc_channel_config[0].input != adcSingleInputTemp) {
#else
    if (adc_channel_config[0].input != adcPosSelTEMP) {
#endif
        return -1;
    }

    /* initialize ADC */
    if (adc_init(0) != 0) {
        return -2;
    }

    return 0;
}
