/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef NRFNET_H
#define NRFNET_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "byteorder.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NRFNET_PAYLOAD_SIZE (32U)
#define NRFNET_HEADER_SIZE  (8U)
#define NRFNET_DATA_SIZE    (NRFNET_PAYLOAD_SIZE - NRFNET_HEADER_SIZE)

/**
 * @brief   Maximum length of a nRFNet address as string of an octal number.
 */
#define NRFNET_ADDR_MAX_STR_LEN (sizeof("07777"))

/**
 * @brief   An unintialized address.
 */
#define NRFNET_ADDR_UNSPECIFIED { 0x00, 0x00 }

/**
 * @brief   The type of a physical pipe index. There are only 6 pipes
 *          available.
 */
typedef uint8_t nrfnet_pipe_t;

/**
 * @brief   The type of a physical pipe address. The maximum length is actually
 *          five bytes.
 */
typedef uint64_t nrfnet_pipe_addr_t;

/**
 * @brief   Data type to represent a nRFNet address.
 */
typedef union {
    uint8_t u8[2];          /**< as two 8-bit unsigned integer */
    network_uint16_t u16;   /**< as one 16-bit unsigned integer */
} nrfnet_addr_t;

/**
 * @brief   Network address information to offload computations. This structure
 *          contains all information to compute parent or descendant addresses
 *          with.
 */
typedef struct {
    uint8_t level;
    nrfnet_addr_t addr;
    nrfnet_addr_t mask;
    nrfnet_addr_t parent_addr;
    nrfnet_pipe_t parent_pipe;
} nrfnet_addr_info_t;

/**
 * @brief   Compressed network header.
 */
typedef struct __attribute__((packed)) {
    uint16_t from : 12;
    uint16_t to : 12;
    uint16_t id;
    uint8_t length;
    uint8_t offset : 4;
    uint8_t next : 4;
    uint8_t reserved;
} nrfnet_comp_hdr_t;

/**
 * @brief   Uncompressed network header.
 */
typedef struct {
    nrfnet_addr_t from;
    nrfnet_addr_t to;
    uint16_t id;
    uint8_t length;
    uint8_t offset;
    uint8_t next;
} nrfnet_hdr_t;

/**
 * @brief   Calculate the address of a direct child given node address and the
 *          pipe index to send via.
 */
nrfnet_addr_t* nrfnet_pipe_to_addr(nrfnet_addr_t* result,
                                   nrfnet_addr_info_t* addr_info,
                                   nrfnet_pipe_t pipe);

/**
 * @brief   Calculate the pipe index given node address and the child node.
 */
nrfnet_pipe_t nrfnet_pipe_from_addr(nrfnet_addr_info_t* addr_info,
                                    nrfnet_addr_t* addr);

/**
 * @brief   Calculate the physical pipe address given an address and pipe.
 */
nrfnet_pipe_addr_t nrfnet_pipe_addr_from_addr(nrfnet_addr_t* node,
                                              nrfnet_pipe_t pipe);


/**
 * @brief   Dump the header fields to stdout.
 * @note    For debugging purposes.
 */
void nrfnet_hdr_dump(nrfnet_hdr_t* hdr);

/**
 * @brief   Calculate the number of fragments from a header.
 */
int nrfnet_hdr_fragments(nrfnet_hdr_t* hdr);

/**
 * @brief   Return true if a given nRFNet header indicates that the payload is
 *          fragmented.
 */
bool nrfnet_hdr_is_fragmented(nrfnet_hdr_t* hdr);

/**
 * @brief   Check if a given nRFNet header is valid.
 */
bool nrfnet_hdr_is_valid(nrfnet_hdr_t* hdr);

/**
 * @brief   Uncompress a nRFNet header.
 */
void nrfnet_hdr_uncompress(nrfnet_hdr_t *dest, nrfnet_comp_hdr_t *src);

/**
 * @brief   Compress a nRFNet header.
 */
void nrfnet_hdr_compress(nrfnet_comp_hdr_t *dest, nrfnet_hdr_t *src);


/**
 * @brief   Initialize nRFNet address information relative to given address, to
 *          offload future computations.
 *
 * @param[out] result   Pointer to address information result.
 * @param[in]  addr     Relative address to compute information for.
 *
 * @return  Pointer to @p result if success
 * @return  NULL if computation failed.
 */
nrfnet_addr_info_t* nrfnet_addr_info_init(nrfnet_addr_info_t* result,
                                          nrfnet_addr_t* addr);

/**
 * @brief   Return true if the given descendant address is an actual
 *          descendant, given the node address and mask.
 */
bool nrfnet_addr_info_is_descendant(nrfnet_addr_info_t* addr_info,
                                    nrfnet_addr_t* addr);

/**
 * @brief   Return true if the given child address is an actual child, given
 *          the node address and mask.
 */
bool nrfnet_addr_info_is_child(nrfnet_addr_info_t* addr_info,
                               nrfnet_addr_t* addr);

/**
 * @brief   Calculate the direct child address, given a descendant address.
 * @note    This method assumes that the given address is a descendant.
 *
 * Given a descendant address and relative address information, compute the
 * address of the first direct child. For instance, if the relative address
 * information is for node 021, given descendant 04321, the resulting direct
 * child address is 0321.
 *
 * @param[out] result   The resulting address.
 * @param[in] addr_info Relative address information of the current node.
 * @param[out] addr     The descendant address.
 *
 * @result  The resulting address.
 */
nrfnet_addr_t* nrfnet_addr_info_child(nrfnet_addr_t* result,
                                      nrfnet_addr_info_t* addr_info,
                                      nrfnet_addr_t* addr);


/**
 * @brief   Checks if two nRFNet addresses are equal.
 *
 * @param[in] a         A nRFNet address.
 * @param[in] b         Another nRFNet address.
 *
 * @return  true, if @p a and @p b are not null and equal
 * @return  false, otherwise.
 */
bool nrfnet_addr_equal(nrfnet_addr_t* a, nrfnet_addr_t* b);

/**
 * @brief   Checks if a nRFNet address is valid.
 *
 * @param[in] addr      A nRFNet address.
 *
 * @return  true, if @p addr is a valid address.
 * @return  false, otherwise.
 */
bool nrfnet_addr_is_valid(nrfnet_addr_t* addr);

/**
 * @brief   Converts a nRFNet address to its string representation.
 *
 * @param[out] result   The resulting string representation of at least
 *                      @ref NRFNET_ADDR_MAX_STR_LEN.
 * @param[in] addr      A nRFNet address
 * @param[in] len       Length of @p result
 *
 * @return  @p result, on success
 * @return  NULL, if @p result_len was lesser than NRFNET_ADDR_MAX_STR_LEN
 * @return  NULL, if @p result or @p addr was NULL
 */
char *nrfnet_addr_to_str(char *result, const nrfnet_addr_t *addr, uint8_t len);

/**
 * @brief   Convert a nRFNet address string representation to a
 *          byte-represented nRFNet address.
 *
 * @param[in] result    The resulting byte representation
 * @param[in] addr      A nRFNet address string representation
 *
 * @return  @p result, on success
 * @return  NULL, if @p addr was malformed
 * @return  NULL, if @p result or @p addr was NULL
 */
nrfnet_addr_t *nrfnet_addr_from_str(nrfnet_addr_t *result, const char *addr);

#ifdef __cplusplus
}
#endif

#endif /* NRFNET_H_ */
