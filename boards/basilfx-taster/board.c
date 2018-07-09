/*
 * Copyright (C) 2018-2021 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_basilfx-taster
 * @{
 *
 * @file
 * @brief       Board specific implementations BasilFX Taster KNX board
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include "board.h"

#include "periph/gpio.h"
#include "periph/spi.h"

//#include "vfs.h"
//#include "fs/devfs.h"
#include "mtd_spi_nor.h"
#include "timex.h"

#ifdef MODULE_MTD
static const mtd_spi_nor_params_t _basilfx_taster_nor_params = {
    .opcode = &mtd_spi_nor_opcode_default,
    .wait_chip_erase = 30LU * US_PER_SEC,
    .wait_32k_erase = 1750LU * US_PER_MS,
    .wait_sector_erase = 240LU * US_PER_MS,
    .wait_chip_wake_up = 1LU * US_PER_MS,
    .clk = BASILFX_TASTER_NOR_SPI_CLK,
    .flag = BASILFX_TASTER_NOR_FLAGS,
    .spi = BASILFX_TASTER_NOR_SPI_DEV,
    .mode = BASILFX_TASTER_NOR_SPI_MODE,
    .cs = BASILFX_TASTER_NOR_SPI_CS,
    .wp = GPIO_UNDEF,
    .hold = GPIO_UNDEF,
    .addr_width = 3,
};

static mtd_spi_nor_t basilfx_taster_nor_dev = {
    .base = {
        .driver = &mtd_spi_nor_driver,
        .page_size = BASILFX_TASTER_NOR_PAGE_SIZE,
        .pages_per_sector = BASILFX_TASTER_NOR_PAGES_PER_SECTOR,
        .sector_count = BASILFX_TASTER_NOR_SECTOR_COUNT,
    },
    .params = &_basilfx_taster_nor_params,
};

mtd_dev_t *mtd0 = (mtd_dev_t *)&basilfx_taster_nor_dev;
#endif /* MODULE_MTD */

/*static devfs_t board_nor_devfs = {
    .path = "/mtd0",
    .f_op = &mtd_vfs_ops,
    .private_data = &mtd0,
};*/

//static int board_nor_init(void)
//{
//    int res = mtd_init(mtd0);
//
//    if (res >= 0) {
//        devfs_register(&board_nor_devfs);
//    }
//
//    /* configure SPI flash to run at higher speed */
//    uint8_t buf0[] = { 0x06 };
//
//    spi_acquire(basilfx_taster_nor_dev.params->spi, basilfx_taster_nor_dev.params->cs, basilfx_taster_nor_dev.params->mode, basilfx_taster_nor_dev.params->clk);
//    spi_transfer_bytes(basilfx_taster_nor_dev.params->spi, basilfx_taster_nor_dev.params->cs, false, &buf0, &buf0, 1);
//    spi_release(basilfx_taster_nor_dev.params->spi);
//
//    uint8_t buf1[] = { 0x01, 0x00, 0x00, 0x02 };
//
//    spi_acquire(basilfx_taster_nor_dev.params->spi, basilfx_taster_nor_dev.params->cs, basilfx_taster_nor_dev.params->mode, basilfx_taster_nor_dev.params->clk);
//    spi_transfer_bytes(basilfx_taster_nor_dev.params->spi, basilfx_taster_nor_dev.params->cs, false, &buf1, &buf1, 4);
//    spi_release(basilfx_taster_nor_dev.params->spi);
//
//    uint8_t buf2[] = { 0x15, 0x00, 0x00 };
//
//    spi_acquire(basilfx_taster_nor_dev.params->spi, basilfx_taster_nor_dev.params->cs, basilfx_taster_nor_dev.params->mode, basilfx_taster_nor_dev.params->clk);
//    spi_transfer_bytes(basilfx_taster_nor_dev.params->spi, basilfx_taster_nor_dev.params->cs, false, &buf2, &buf2, 3);
//    spi_release(basilfx_taster_nor_dev.params->spi);
//
//    return res;
//}

void board_init(void)
{
    /* initialize the CPU */
    cpu_init();

    gpio_init(PB0_PIN, GPIO_IN_PU);
    gpio_init(LED0_PIN, GPIO_OUT);

    gpio_init(IF0_PIN, GPIO_OUT);
    gpio_init(IF1_PIN, GPIO_OUT);
    gpio_init(IF2_PIN, GPIO_OUT);
    gpio_init(IF3_PIN, GPIO_OUT);
    gpio_init(IF4_PIN, GPIO_OUT);
    gpio_init(IF5_PIN, GPIO_OUT);

    gpio_write(LED0_PIN, 0);

    gpio_write(IF0_PIN, 0);
    gpio_write(IF1_PIN, 0);
    gpio_write(IF2_PIN, 0);
    gpio_write(IF3_PIN, 0);
    gpio_write(IF4_PIN, 0);
    gpio_write(IF5_PIN, 0);

    gpio_init(CS_PIN, GPIO_OUT);

    //board_nor_init();
}
