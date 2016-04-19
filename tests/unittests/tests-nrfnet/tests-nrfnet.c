/*
 * Copyright (C) 2016 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/* clib includes */

#include "embUnit.h"

#include "net/nrfnet.h"

#include "tests-nrfnet.h"

void tests_nrfnet(void)
{
    TESTS_RUN(tests_nrfnet_addr_info_tests());
    TESTS_RUN(tests_nrfnet_addr_tests());
    TESTS_RUN(tests_nrfnet_hdr_tests());
    TESTS_RUN(tests_nrfnet_rbuf_tests());
}
