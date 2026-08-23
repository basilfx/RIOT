/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @ingroup     drivers_tda74xx
 * @{
 *
 * @file
 * @brief       Definitions of the ST TDA74xx audio processors
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "tda74xx_info.h"

const tda74xx_info_t tda7439_info = {
    .inputs = TDA7439_INFO_INPUTS,
    .bands = TDA7439_INFO_BANDS,
    .reg_bass = TDA7439_INFO_REG_BASS,
};

const tda74xx_info_t tda7440_info = {
    .inputs = TDA7440_INFO_INPUTS,
    .bands = TDA7440_INFO_BANDS,
    .reg_bass = TDA7440_INFO_REG_BASS,
};

const tda74xx_info_t tda7449_info = {
    .inputs = TDA7449_INFO_INPUTS,
    .bands = TDA7449_INFO_BANDS,
    .reg_bass = TDA7449_INFO_REG_BASS,
};
