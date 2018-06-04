/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef STORAGE_H
#define STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

int storage_init(void);
int storage_validate(void);
int storage_prepare(void);
int storage_read(void);
int storage_write(void);

void storage_acquire(void);
void storage_release(void);

#ifdef __cplusplus
}
#endif

#endif /* STORAGE_H */
/** @} */
