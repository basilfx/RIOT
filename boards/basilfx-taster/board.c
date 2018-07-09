/*
 * Copyright (C) 2015-2018 Freie Universität Berlin
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

#include "em_cmu.h"
#include "em_gpio.h"

#include "periph/gpio.h"
#include "periph/spi.h"

#include "vfs.h"
#include "fs/devfs.h"
#include "mtd_spi_nor.h"

static mtd_spi_nor_t board_nor_dev = {
    .base = {
        .driver = &mtd_spi_nor_driver,
        .page_size = 256,
        .pages_per_sector = 256,
        .sector_count = 16,
    },
    .opcode = &mtd_spi_nor_opcode_default,
    .spi = SPI_DEV(0),
    .cs = GPIO_PIN(PD, 15),
    .addr_width = 3,
    .mode = SPI_MODE_0,
    .clk = SPI_CLK_10MHZ,
};

mtd_dev_t *mtd0 = (mtd_dev_t *)&board_nor_dev;

static devfs_t board_nor_devfs = {
    .path = "/mtd0",
    .f_op = &mtd_vfs_ops,
    .private_data = &board_nor_dev,
};

static int board_nor_init(void)
{
    int res = mtd_init(mtd0);

    if (res >= 0) {
        devfs_register(&board_nor_devfs);
    }

    /* configure SPI flash to run at higher speed */
    uint8_t buf0[] = { 0x06 };

    spi_acquire(board_nor_dev.spi, board_nor_dev.cs, board_nor_dev.mode, board_nor_dev.clk);
    spi_transfer_bytes(board_nor_dev.spi, board_nor_dev.cs, false, &buf0, &buf0, 1);
    spi_release(board_nor_dev.spi);

    uint8_t buf1[] = { 0x01, 0x00, 0x00, 0x02 };

    spi_acquire(board_nor_dev.spi, board_nor_dev.cs, board_nor_dev.mode, board_nor_dev.clk);
    spi_transfer_bytes(board_nor_dev.spi, board_nor_dev.cs, false, &buf1, &buf1, 4);
    spi_release(board_nor_dev.spi);

    uint8_t buf2[] = { 0x15, 0x00, 0x00 };

    spi_acquire(board_nor_dev.spi, board_nor_dev.cs, board_nor_dev.mode, board_nor_dev.clk);
    spi_transfer_bytes(board_nor_dev.spi, board_nor_dev.cs, false, &buf2, &buf2, 3);
    spi_release(board_nor_dev.spi);

    return res;
}

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

    board_nor_init();
}
