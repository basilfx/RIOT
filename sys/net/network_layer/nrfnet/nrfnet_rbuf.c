/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <string.h>
#include <stdio.h>

#include "net/nrfnet_rbuf.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

void nrfnet_rbuf_init(nrfnet_rbuf_t* rbuf)
{
    for (uint32_t i = 0; i < rbuf->head_count; i++) {
        rbuf->heads[i].used = false;
        rbuf->heads[i].next = NULL;
    }

    for (uint32_t i = 0; i < rbuf->hole_count; i++) {
        rbuf->holes[i].used = false;
        rbuf->holes[i].next = NULL;
    }
}

void nrfnet_rbuf_dump(nrfnet_rbuf_t* rbuf)
{
    int used;
    int fragments;
    int pending;

    /* count the number of heads in use */
    used = 0;
    fragments = 0;
    pending = 0;

    for (uint32_t i = 0; i < rbuf->head_count; i++) {
        if (rbuf->heads[i].used) {
            used++;

            fragments += nrfnet_hdr_fragments(&rbuf->heads[i].hdr);
            pending += rbuf->heads[i].fragments;
        }
    }

    printf("Nr. heads in use: %d/%d\n", used, rbuf->head_count);
    printf("Nr. fragments pending: %d/%d\n", fragments, pending);

    /* count the number of holes in use */
    used = 0;

    for (uint32_t i = 0; i < rbuf->hole_count; i++) {
        if (rbuf->holes[i].used) {
            used++;
        }
    }

    printf("Nr. holes in use: %d/%d\n", used, rbuf->hole_count);
}

void nrfnet_rbuf_gc(nrfnet_rbuf_t* rbuf, uint32_t timestamp, uint32_t lifetime)
{
    for (uint32_t i = 0; i < rbuf->head_count; i++) {
        if (rbuf->heads[i].used) {
            if (rbuf->heads[i].timestamp + lifetime < timestamp) {
                DEBUG("nrfnet_rbuf: head with ID %d expired\n", rbuf->heads[i].hdr.id);
                nrfnet_rbuf_free(&rbuf->heads[i]);
            }
        }
    }
}

void nrfnet_rbuf_free(nrfnet_rbuf_head_t* head)
{
    /* clear the holes */
    nrfnet_rbuf_hole_t* current = head->next;

    while (current != NULL) {
        current->used = false;
        current->next = false;

        current = current->next;
    }

    /* clear the head */
    head->used = false;
    head->next = NULL;
}

/**
 * Hole: a slot for fragmented packet data.
 * Head: start of a to-reassemble-packet
 *
 * When a fragmented packet is complete, the `fragments` property is zero and
 * the fragments are ordered in the `next` property.
 */
nrfnet_rbuf_head_t* nrfnet_rbuf_add(nrfnet_rbuf_t* rbuf,
                                    nrfnet_hdr_t* hdr,
                                    uint8_t* data,
                                    size_t length,
                                    uint32_t timestamp)
{
    nrfnet_rbuf_head_t* head = NULL;
    nrfnet_rbuf_head_t* head_free = NULL;

    /* try to find an existing head */
    for (uint32_t i = 0; i < rbuf->head_count; i++) {
        nrfnet_rbuf_head_t* current = &rbuf->heads[i];

        if (current->used) {
            if (nrfnet_addr_equal(&current->hdr.from, &hdr->from)) {
                if (nrfnet_addr_equal(&current->hdr.to, &hdr->to)) {
                    if (current->hdr.length == hdr->length) {
                        if (current->hdr.id == hdr->id) {
                            head = current;
                            break;
                        }
                    }
                }
            }
        } else {
            /* store a reference to the first free head */
            if (head_free == NULL) {
                head_free = current;
            }
        }
    }

    /* allocate a new head if needed */
    if (head == NULL) {
        if (head_free == NULL) {
            DEBUG("nrfnet_rbuf: no heads free\n");
            return NULL;
        }

        head = head_free;

        memcpy(&head->hdr, hdr, sizeof(nrfnet_hdr_t));
        head->used = true;
        head->timestamp = timestamp;
        head->fragments = nrfnet_hdr_fragments(hdr);

        DEBUG("nrfnet_rbuf: allocated head for %d fragments\n", head->fragments);
    } else {
        DEBUG("nrfnet_rbuf: found existing head for header with ID %d\n", head->hdr.id);
    }

    /* find a free hole to enqueue data */
    nrfnet_rbuf_hole_t* hole = NULL;

    for (uint32_t i = 0; i < rbuf->hole_count; i++) {
        nrfnet_rbuf_hole_t* current = &rbuf->holes[i];

        if (!current->used) {
            hole = current;
            break;
        }
    }

    if (hole == NULL) {
        DEBUG("nrfnet_rbuf: no holes free\n");
        return NULL;
    }

    /* enqueue the data */
    hole->used = true;
    hole->offset = hdr->offset;
    memcpy(hole->data, data, length);

    if (head->next == NULL) {
        head->next = hole;
        DEBUG("nrfnet_rbuf: starting new chain of holes\n");
    } else {
        nrfnet_rbuf_hole_t* current = head->next;
        nrfnet_rbuf_hole_t* prev = NULL;

        while (current != NULL && current->offset < hole->offset) {
            prev = current;
            current = current->next;
        }

        /* in case the offset is less than the first hole */
        if (prev == NULL) {
            head->next = hole;
        } else {
            prev->next = hole;
        }

        hole->next = current;

        if (hole->next != NULL) {
            DEBUG("nrfnet_rbuf: hole (offset %d) inserted before hole with offset %d\n", hole->offset, hole->next->offset);
        } else {
            DEBUG("nrfnet_rbuf: hole (offset %d) appended\n", hole->offset);
        }
    }

    head->fragments--;
    DEBUG("nrfnet_rbuf: %d fragments left\n", head->fragments);

    return head;
}

uint8_t* nrfnet_rbuf_cpy(nrfnet_rbuf_head_t* head, uint8_t* dest, size_t length)
{
    uint8_t bytes;
    uint8_t last_bytes;

    /* don't copy to null buffer */
    if (dest == NULL) {
        return NULL;
    }

    /* don't copy if complete yet */
    if (head->fragments != 0) {
        DEBUG("nrfnet_rbuf: missing %d fragments", head->fragments);
        return NULL;
    }

    /* copy the data */
    nrfnet_rbuf_hole_t* current = head->next;
    bytes = 0;

    while (current != NULL && bytes < length && bytes < head->hdr.length) {
        if ((bytes + NRFNET_DATA_SIZE) > length) {
            last_bytes = (length - bytes);
        } else {
            last_bytes = NRFNET_DATA_SIZE;
        }

        memcpy(&dest[bytes], current->data, last_bytes);
        DEBUG("nrfnet_rbuf: copied bytes %d - %d\n", bytes, bytes + last_bytes);

        bytes = bytes + last_bytes;
        current = current->next;
    }

    return dest;
}
