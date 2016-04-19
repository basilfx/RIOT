/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>
#include <string.h>

#include "net/nrfnet.h"

static uint8_t pipe_segments[] = { 0x3c, 0x5a, 0x69, 0x96, 0xa5, 0xc3 };

nrfnet_addr_t* nrfnet_pipe_to_addr(nrfnet_addr_t* result,
                                   nrfnet_addr_info_t* addr_info,
                                   nrfnet_pipe_t pipe)
{
    uint16_t i = 0;
    uint16_t m = byteorder_ntohs(addr_info->mask.u16);

    while (m) {
        i += 3;
        m >>= 3;
    }

    result->u16 = byteorder_htons((pipe << i) |
                                  byteorder_ntohs(addr_info->addr.u16));

    return result;
}

nrfnet_pipe_t nrfnet_pipe_from_addr(nrfnet_addr_info_t* addr_info,
                                    nrfnet_addr_t* addr)
{
    uint16_t i = byteorder_ntohs(addr->u16);
    uint16_t m = byteorder_ntohs(addr_info->mask.u16);

    while (m) {
        i >>= 3;
        m >>= 3;
    }

    return i & 0x07;
}

nrfnet_pipe_addr_t nrfnet_pipe_addr_from_addr(nrfnet_addr_t* addr, uint8_t pipe)
{
    uint64_t result;
    uint8_t* out = (uint8_t*) &result;

    out[0] = pipe_segments[pipe];

    uint8_t w;
    uint16_t i = 4;
    uint16_t shift = 12;

    while (i--) {
        w = (byteorder_ntohs(addr->u16) >> shift) & 0xF ;
        w |= ~w << 4;
        out[i+1] = w;

        shift -= 4;
    }

    return result;
}


void nrfnet_hdr_dump(nrfnet_hdr_t* hdr)
{
    char temp[NRFNET_ADDR_MAX_STR_LEN];

    nrfnet_addr_to_str(temp, &hdr->from, NRFNET_ADDR_MAX_STR_LEN);
    printf("From: %s\n", temp);

    nrfnet_addr_to_str(temp, &hdr->to, NRFNET_ADDR_MAX_STR_LEN);
    printf("To: %s\n", temp);

    printf("ID: %d\n", hdr->id);
    printf("Length: %u\n", hdr->length);
    printf("Offset: %u\n", hdr->offset);
    printf("Next: 0x%x\n", hdr->next);
}

int nrfnet_hdr_fragments(nrfnet_hdr_t* hdr)
{
    return (hdr->length + NRFNET_DATA_SIZE - 1) / NRFNET_DATA_SIZE;
}

bool nrfnet_hdr_is_fragmented(nrfnet_hdr_t* hdr)
{
    return hdr->length > NRFNET_DATA_SIZE;
}

bool nrfnet_hdr_is_valid(nrfnet_hdr_t* hdr)
{
    /* ignore invalid addresses */
    if (!nrfnet_addr_is_valid(&hdr->from) || !nrfnet_addr_is_valid(&hdr->to)) {
        return false;
    }

    /* each packet must contain data */
    if (hdr->length == 0) {
        return false;
    }

    /* offset field is only four bits wide */
    if (hdr->offset > 0x0f) {
        return false;
    }

    /* offset may not exceed length */
    if (hdr->offset > ((hdr->length + NRFNET_DATA_SIZE) - 1) / NRFNET_DATA_SIZE) {
        return false;
    }

    /* next field is only four bits wide */
    if (hdr->next > 0x0f) {
        return false;
    }

    return true;
}

void nrfnet_hdr_uncompress(nrfnet_hdr_t *dest, nrfnet_comp_hdr_t *src)
{
    dest->from.u16.u16 = src->from;
    dest->to.u16.u16 = src->to;
    dest->id = src->id;
    dest->length = src->length;
    dest->offset = src->offset;
    dest->next = src->next;
}

void nrfnet_hdr_compress(nrfnet_comp_hdr_t *dest, nrfnet_hdr_t *src)
{
    dest->from = src->from.u16.u16;
    dest->to = src->to.u16.u16;
    dest->id = src->id;
    dest->length = src->length;
    dest->offset = src->offset;
    dest->next = src->next;

    /* always set to zero */
    dest->reserved = 0;
}


nrfnet_addr_info_t* nrfnet_addr_info_init(nrfnet_addr_info_t* result,
                                          nrfnet_addr_t* addr)
{
    /* copy the address */
    result->addr.u16 = addr->u16;

    /* establish the address mask */
    uint16_t address = byteorder_ntohs(addr->u16);
    uint16_t mask_check = 0xFFFF;
    uint8_t level = 0;

    while (address & mask_check) {
        mask_check <<= 3;
        level++;
    }

    /* mask is the inverse of what is left */
    uint16_t mask = ~mask_check;

    result->mask.u16 = byteorder_htons(~mask_check);
    result->level = level;

    /* parent address mask is the next level down */
    uint16_t parent_mask = mask >> 3;

    /* parent address is the part IN the mask */
    result->parent_addr.u16 = byteorder_htons(address & parent_mask);

    /* parent pipe is the part OUT of the mask */
    uint16_t i = address;
    uint16_t m = parent_mask;

    while (m) {
        i >>= 3;
        m >>= 3;
    }

    result->parent_pipe = i;

    return result;
}

bool nrfnet_addr_info_is_descendant(nrfnet_addr_info_t* addr_info,
                                    nrfnet_addr_t* addr)
{
    return (addr->u16.u16 & addr_info->mask.u16.u16) ==
        addr_info->addr.u16.u16;
}

bool nrfnet_addr_info_is_child(nrfnet_addr_info_t* addr_info,
                               nrfnet_addr_t* addr)
{
    if (nrfnet_addr_info_is_descendant(addr_info, addr)) {
        uint16_t child_mask = (~byteorder_ntohs(addr_info->mask.u16)) << 3;

        return (byteorder_ntohs(addr->u16) & child_mask) == 0;
    }

    return false;
}

nrfnet_addr_t* nrfnet_addr_info_child(nrfnet_addr_t* result,
                                      nrfnet_addr_info_t* addr_info,
                                      nrfnet_addr_t* addr)
{
    uint16_t child_mask = (byteorder_ntohs(addr_info->mask.u16) << 3) | 0x07;
    uint16_t child = byteorder_ntohs(addr->u16) & child_mask;

    result->u16 = byteorder_htons(child);

    return result;
}


bool nrfnet_addr_equal(nrfnet_addr_t* addr_a, nrfnet_addr_t* addr_b)
{
    if (addr_a == NULL || addr_b == NULL) {
        return false;
    }

    return (addr_a->u16.u16 == addr_b->u16.u16);
}

bool nrfnet_addr_is_valid(nrfnet_addr_t* addr)
{
    uint16_t temp = byteorder_ntohs(addr->u16);

    while (temp) {
        uint8_t digit = temp & 0x07;

        if (digit < 1 || digit > 5) {
            return false;
        }

        temp >>= 3;
    }

    return true;
}

char *nrfnet_addr_to_str(char *result, const nrfnet_addr_t *addr, uint8_t len)
{
    /* return if one or both is null */
    if (result == NULL || addr == NULL) {
        return NULL;
    }

    /* ensure the result buffer is large enough */
    if (len < NRFNET_ADDR_MAX_STR_LEN) {
        return NULL;
    }

    /* convert to string */
    uint16_t temp = byteorder_ntohs(addr->u16);
    snprintf(result, len, "0%o", temp);

    return result;
}

nrfnet_addr_t *nrfnet_addr_from_str(nrfnet_addr_t *result, const char *addr)
{
    /* return if one or both is null */
    if (result == NULL || addr == NULL) {
        return NULL;
    }

    /* address has to start with a 0 (and not an empty string too) */
    if (addr[0] != '0') {
        return NULL;
    }

    /* address may not be longer than defined (including the null byte) */
    if (strlen(addr) >= NRFNET_ADDR_MAX_STR_LEN) {
        return NULL;
    }

    /* convert octal to decimal */
    uint16_t temp;
    sscanf(addr, "0%o", (unsigned int *)&temp);

    result->u16 = byteorder_htons(temp);

    return result;
}
