/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#pragma once

/**
 * @ingroup     cpu_esp8266
 * @{
 *
 * @file
 * @brief       Wrapper for sys/lock.h
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is a wrapper for sys/lock.h to provide the `_lock_*` locking
 * functions implemented in `cpu/esp_common/syscalls.c` if newlib is compiled
 * with retargetable locking. In that case, newlib's `__retarget_lock_*`
 * functions are mapped to these functions. Older newlib versions patched by
 * Espressif already declare these functions themselves.
 */

#ifndef DOXYGEN

#include_next <sys/lock.h>

#if defined(_RETARGETABLE_LOCKING)

#ifdef __cplusplus
extern "C" {
#endif

typedef _LOCK_T _lock_t;

void _lock_init(_lock_t *lock);
void _lock_init_recursive(_lock_t *lock);
void _lock_close(_lock_t *lock);
void _lock_close_recursive(_lock_t *lock);
void _lock_acquire(_lock_t *lock);
void _lock_acquire_recursive(_lock_t *lock);
int  _lock_try_acquire(_lock_t *lock);
int  _lock_try_acquire_recursive(_lock_t *lock);
void _lock_release(_lock_t *lock);
void _lock_release_recursive(_lock_t *lock);

#ifdef __cplusplus
}
#endif

#endif /* defined(_RETARGETABLE_LOCKING) */

#endif /* DOXYGEN */
/** @} */
