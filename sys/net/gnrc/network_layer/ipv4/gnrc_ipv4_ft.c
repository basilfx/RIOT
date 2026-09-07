/*
 * SPDX-FileCopyrightText: 2026 Bas Stottelaar <basstottelaar@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only
 */

/**
 * @{
 *
 * @file
 */

#include <errno.h>
#include <string.h>

#include "net/gnrc/ipv4/ft.h"

static gnrc_ipv4_ft_t _entries[CONFIG_GNRC_IPV4_FT_SIZE];

static gnrc_ipv4_ft_t *_find(const ipv4_addr_t *dst, uint8_t dst_len)
{
    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_FT_SIZE; i++) {
        if ((_entries[i].iface != KERNEL_PID_UNDEF) &&
            (_entries[i].dst_len == dst_len) &&
            ipv4_addr_equal(&_entries[i].dst, dst)) {
            return &_entries[i];
        }
    }
    return NULL;
}

int gnrc_ipv4_ft_get(const ipv4_addr_t *dst, gnrc_ipv4_ft_t *fte)
{
    gnrc_ipv4_ft_t *best = NULL;

    for (unsigned i = 0; i < CONFIG_GNRC_IPV4_FT_SIZE; i++) {
        gnrc_ipv4_ft_t *entry = &_entries[i];

        if (entry->iface == KERNEL_PID_UNDEF) {
            continue;
        }
        if (ipv4_addr_match_prefix(&entry->dst, dst, entry->dst_len) &&
            ((best == NULL) || (entry->dst_len > best->dst_len))) {
            best = entry;
        }
    }
    if (best == NULL) {
        return -ENETUNREACH;
    }
    memcpy(fte, best, sizeof(*fte));
    return 0;
}

int gnrc_ipv4_ft_add(const ipv4_addr_t *dst, uint8_t dst_len,
                     const ipv4_addr_t *next_hop, kernel_pid_t iface)
{
    static const ipv4_addr_t unspecified = { .u8 = { 0, 0, 0, 0 } };
    gnrc_ipv4_ft_t *entry;

    if (dst == NULL) {
        dst = &unspecified;
    }
    entry = _find(dst, dst_len);
    if (entry == NULL) {
        entry = _find(&unspecified, 0);
        for (unsigned i = 0; (entry == NULL) && (i < CONFIG_GNRC_IPV4_FT_SIZE); i++) {
            if (_entries[i].iface == KERNEL_PID_UNDEF) {
                entry = &_entries[i];
            }
        }
        if (entry == NULL) {
            return -ENOMEM;
        }
    }
    entry->dst = *dst;
    entry->dst_len = dst_len;
    entry->next_hop = *next_hop;
    entry->iface = iface;
    return 0;
}

void gnrc_ipv4_ft_remove(const ipv4_addr_t *dst, uint8_t dst_len)
{
    static const ipv4_addr_t unspecified = { .u8 = { 0, 0, 0, 0 } };
    gnrc_ipv4_ft_t *entry;

    if (dst == NULL) {
        dst = &unspecified;
    }
    entry = _find(dst, dst_len);
    if (entry != NULL) {
        entry->iface = KERNEL_PID_UNDEF;
    }
}

/** @} */
