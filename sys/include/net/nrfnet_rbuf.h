/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef NRFNET_RBUF_H
#define NRFNET_RBUF_H

#include <stdint.h>
#include <stdbool.h>

#include "net/nrfnet.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NRFNET_RBUF_LIFETIME    (5U)

/**
 * @brief   Definition of a hole.
 */
typedef struct hole {
    bool used;

    uint8_t offset;
    uint8_t data[NRFNET_PAYLOAD_SIZE];

    struct hole* next;
} nrfnet_rbuf_hole_t;

/**
 * @brief   Definition of a head.
 */
typedef struct {
    bool used;

    nrfnet_hdr_t hdr;
    uint32_t timestamp;
    uint8_t fragments;

    nrfnet_rbuf_hole_t* next;
} nrfnet_rbuf_head_t;

/**
 * @brief   Definition of a reassembly buffer.
 */
typedef struct {
    nrfnet_rbuf_hole_t* holes;
    nrfnet_rbuf_head_t* heads;
    uint8_t hole_count;
    uint8_t head_count;
} nrfnet_rbuf_t;

/**
 * @brief   Initialize a new reassembly buffer. It ensures all holes and heads
 *          are available.
 */
void nrfnet_rbuf_init(nrfnet_rbuf_t* rbuf);

/**
 * @brief   Dump the internal reassembly buffer state to stdout.
 * @note    For debugging purposes.
 */
void nrfnet_rbuf_dump(nrfnet_rbuf_t* rbuf);

/**
 * @brief   Perform a garbage collection on the reassembly buffer. It frees all
 *          heads and holes of heads that have expired the given @p lifetime at
 *          the given @p timestamp.
 */
void nrfnet_rbuf_gc(nrfnet_rbuf_t* rbuf,
                    uint32_t timestamp,
                    uint32_t lifetime);

/**
 * @brief   Clear the given head and its associated holes.
 */
void nrfnet_rbuf_free(nrfnet_rbuf_head_t* head);

/**
 * @brief   Add a single nRFNet packet to an existing head, or allocate a new
 *          one if it is new sequence of packets.
 */
nrfnet_rbuf_head_t* nrfnet_rbuf_add(nrfnet_rbuf_t* rbuf,
                                    nrfnet_hdr_t* hdr,
                                    uint8_t* data,
                                    size_t length,
                                    uint32_t timestamp);

/**
 * @brief   Copy a fully received packet from the reassembly buffer to the
 *          given destination.
 * @note    The packet is not freed afterwards.
 */
uint8_t* nrfnet_rbuf_cpy(nrfnet_rbuf_head_t* head,
                         uint8_t* dest,
                         size_t length);

#ifdef __cplusplus
}
#endif

#endif /* NRFNET_RBUF_H */
