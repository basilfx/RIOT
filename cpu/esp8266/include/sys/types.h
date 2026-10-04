/*
 * SPDX-FileCopyrightText: 2019 Gunar Schorcht
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     cpu_esp8266
 * @{
 *
 * @file
 * @brief       Wrapper for sys/types.h
 *
 * @author      Gunar Schorcht <gunar@schorcht.net>
 *
 * This file is a wrapper for sys/types.h to define the types fsblkcnt_t and
 * fsfilcnt_t needed in statvfs.h if they are not defined by newlib.
 */

#ifndef DOXYGEN

#include_next <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _FSBLKCNT_T_DECLARED
#include <stdint.h>
typedef uint32_t fsblkcnt_t;
typedef uint32_t fsfilcnt_t;
#define _FSBLKCNT_T_DECLARED
#endif

#ifdef __cplusplus
}
#endif

#endif /* DOXYGEN */
/** @} */
