/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_knx_device_system7
 * @{
 *
 * @file
 * @brief       KNX Device communication object association interface
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#ifndef KNX_DEVICE_SYSTEM7_ASSOC_H
#define KNX_DEVICE_SYSTEM7_ASSOC_H

#include "net/knx.h"
#include "knx_device/system7/com_object.h"
#include "knx_device/system7/tables.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Association mapping of a group address to a communication object
 */
typedef struct {
    knx_addr_group_t group_addr;        /**< The group address */
    knx_com_object_t *com_object;       /**< The communication object
                                             referenced by the group address */
} knx_assoc_mapping_t;

/**
 * @brief   Association mapping holder
 */
typedef struct {
    knx_assoc_mapping_t *mappings;      /**< Holds the mappings */
    uint8_t count;                      /**< Number of mappings */
} knx_assoc_t;

/**
 * @brief   Update the associations given the association table, address table
 *          and communication objects table
 *
 * A mapping of group addresses to communications will be constructed, to allow
 * efficient lookups (using binary search) when responding to group address
 * messages.
 *
 * This must be performed after each of the tables changes, because the
 * associations are kept in a ordered fashion.
 *
 * A limit must indicate the maximum number of association that fit in
 * @p target. If the limit is smaller than the number of associations in the
 * association table @p assoc_table, then @p limit will overrule.
 *
 * @param[inout] target     Target associations (must not be null)
 * @param[in] objects       Communication objects
 * @param[in] assoc_table   Association table
 * @param[in] addr_table    Address table
 * @param[in] limit         Maximum number of associations (that fit in @p
 *                          target)
 */
void knx_assoc_update(knx_assoc_t *target, knx_com_object_t *objects, knx_table_assoc_t *assoc_table, const knx_table_addr_t *addr_table, uint8_t limit);

/**
 * @brief   Find the first communication object for a given group address
 *
 * Binary search will be used to find the first communication object.
 *
 * @param[in] target        Associations to query (must nog be null)
 * @param[in] group_addr    Group address to use for lookup (must nog be null)
 *
 * @return  -1 if association not found
 * @return  index to association
 */
int knx_assoc_find_by_group_address(const knx_assoc_t *target, const knx_addr_group_t *group_addr);

/**
 * @brief   Iterate over all associations matching a group address
 *
 * @param[in] target        Associations to query (must nog be null)
 * @param[in] prev          Previous result returned by this method (or @p NULL
 *                          on first iteration)
 * @param[in] group_addr    Group address to use for lookup (must nog be null)
 *
 * @return  reference to association mapping matching @p group_addr
 * @return  @p NULL if no association available
 */
knx_assoc_mapping_t *knx_assoc_iter_by_group_address(const knx_assoc_t *target, knx_assoc_mapping_t *prev, const knx_addr_group_t *group_addr);

/**
 * @brief   Find the first group address for a given communication object
 *
 * Sequential search will be used to find the first group address.
 *
 * @param[in] target        Associations to query (must nog be null)
 * @param[in] com_boject    Communication object to use for lookup (must nog be
 *                          null)
 *
 * @return  -1 if association not found
 * @return  index to association
 */
int knx_assoc_find_by_com_object(const knx_assoc_t *target, const knx_com_object_t *com_object);

/**
 * @brief   Iterate over all associations matching a communication object
 *
 * @param[in] target        Associations to query (must nog be null)
 * @param[in] prev          Previous result returned by this method (or @p NULL
 *                          on first iteration)
 * @param[in] com_boject    Communication object to use for lookup (must nog be
 *                          null)
 *
 * @return  reference to association mapping matching @p com_object
 * @return  @p NULL if no association available
 */
knx_assoc_mapping_t *knx_assoc_iter_by_com_object(const knx_assoc_t *target, knx_assoc_mapping_t *prev, const knx_com_object_t *com_object);

#ifdef __cplusplus
}
#endif

#endif /* KNX_DEVICE_SYSTEM7_ASSOC_H */
/** @} */
