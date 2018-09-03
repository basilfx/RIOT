/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Hardware test application for BasilFX-Taster board.
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "periph/rtc.h"
#include "periph/gpio.h"
#include "periph/pm.h"
#include "periph/spi.h"

#include "em_cmu.h"
#include "em_gpio.h"

#include "thread.h"
#include "shell.h"
#include "board.h"

#include "ncn5120.h"
#include "ncn5120_params.h"

/**
 * @brief   The NCN5120 device
 */
static ncn5120_t dev;

/**
 * @brief   Current running test
 */
static uint8_t test;

/**
 * @brief   Stack for background task
 */
static char _stack[THREAD_STACKSIZE_MAIN];

/**
 * @brief   Callback for NCN5120 device
 */
static void _cb_ncn5120(uint8_t event, void *data, void *arg)
{
    (void)arg;
    (void)data;

    if (test == 6) {
        switch (event) {
            case NCN5120_EVENT_TELEGRAM:
                puts("Telegram received");
                break;
            case NCN5120_EVENT_TELEGRAM_INCOMPLETE:
                puts("Incomplete telegram received");
                break;
            case NCN5120_EVENT_SAVE:
                puts("Bus voltage dropping");
                break;
        }
    }
}

/**
 * @brief   Background thread
 */
static void *_background_thread(void *args)
{
    (void)args;

    while (1) {
        if (test == 1) {
            gpio_write(LED0_PIN,
                       gpio_read(IF0_PIN) |
                       gpio_read(IF1_PIN) |
                       gpio_read(IF2_PIN) |
                       gpio_read(IF3_PIN) |
                       gpio_read(IF4_PIN) |
                       gpio_read(IF5_PIN));
        }
        else if (test == 2) {
            gpio_write(LED0_PIN, gpio_read(SDA_PIN) | gpio_read(SCL_PIN));
        }
        else if (test == 3) {
            gpio_write(LED0_PIN, !gpio_read(PB0_PIN));
        }
    }

    return NULL;
}

static int cmd_out(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("Connect jumper pins 1-2.");
    puts("The LEDs must burn.");

    test = 0;

    gpio_init(IF0_PIN, GPIO_OUT);
    gpio_init(IF1_PIN, GPIO_OUT);
    gpio_init(IF2_PIN, GPIO_OUT);
    gpio_init(IF3_PIN, GPIO_OUT);
    gpio_init(IF4_PIN, GPIO_OUT);
    gpio_init(IF5_PIN, GPIO_OUT);

    gpio_write(IF0_PIN, 1);
    gpio_write(IF1_PIN, 1);
    gpio_write(IF2_PIN, 1);
    gpio_write(IF3_PIN, 1);
    gpio_write(IF4_PIN, 1);
    gpio_write(IF5_PIN, 1);

    return 0;
}

static int cmd_in(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("Connect jumper pins 2-3.");
    puts("Press any of the black buttons to toggle the PROG LED.");

    test = 1;

    gpio_init(IF0_PIN, GPIO_IN_PD);
    gpio_init(IF1_PIN, GPIO_IN_PD);
    gpio_init(IF2_PIN, GPIO_IN_PD);
    gpio_init(IF3_PIN, GPIO_IN_PD);
    gpio_init(IF4_PIN, GPIO_IN_PD);
    gpio_init(IF5_PIN, GPIO_IN_PD);

    return 0;
}

static int cmd_i2c(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("Connect jumper pins 2-3.");
    puts("Press white buttons to toggle the PROG LED.");

    test = 2;

    gpio_init(SDA_PIN, GPIO_IN_PD);
    gpio_init(SCL_PIN, GPIO_IN_PD);

    return 0;
}

static int cmd_prog(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("Press the PROG button to toggle the PROG LED.");

    test = 3;

    gpio_init(PB0_PIN, GPIO_IN_PU);

    return 0;
}

static int cmd_clk(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("HFXO available on PA1. LFXO available on PA0.");

    test = 4;

    CMU->CTRL = (CMU->CTRL & ~_CMU_CTRL_CLKOUTSEL0_MASK) | CMU_CTRL_CLKOUTSEL0_LFXO;
    CMU->CTRL = (CMU->CTRL & ~_CMU_CTRL_CLKOUTSEL1_MASK) | CMU_CTRL_CLKOUTSEL1_HFXO;

    CMU->ROUTEPEN = CMU_ROUTEPEN_CLKOUT0PEN | CMU_ROUTEPEN_CLKOUT1PEN;
    CMU->ROUTELOC0 = CMU_ROUTELOC0_CLKOUT0LOC_LOC0 | CMU_ROUTELOC0_CLKOUT1LOC_LOC0;

    GPIO_PinModeSet(gpioPortA, 0, gpioModePushPull, 0);
    GPIO_PinModeSet(gpioPortA, 1, gpioModePushPull, 0);

    return 0;
}

static int cmd_rtc(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("RTC time test.");

    test = 5;

    struct tm time;
    rtc_get_time(&time);

    printf("%04d-%02d-%02d %02d:%02d:%02d\n",
           time.tm_year,
           time.tm_mon + 1,
           time.tm_mday,
           time.tm_hour,
           time.tm_min,
           time.tm_sec);

    return 0;
}

static int cmd_knx(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("Testing KNX interface.");

    test = 6;

    int result;

    dev.cb = _cb_ncn5120;

    /* test init */
    result = ncn5120_init(&dev, &ncn5120_params[0]);

    if (result != NCN5120_OK) {
        printf("Unable to initialize driver: %d\n", result);
        return 1;
    }

    /* test reset */
    result = ncn5120_reset(&dev);

    if (result != NCN5120_OK) {
        printf("Unable to reset: %d\n", result);
        return 1;
    }

    /* test state */
    result = ncn5120_get_state(&dev);

    if (result < NCN5120_OK) {
        printf("Unable to retrieve state: %d\n", result);
        return 1;
    }

    printf("State is 0x%02x\n", result);

    /* activate CRC */
    result = ncn5120_configure(&dev, NCN5120_CONFIGURE_CRC_CTIT);

    if (result < NCN5120_OK) {
        printf("Unable to configure: %d\n", result);
        return 1;
    }

    /* send a telegram */
    uint8_t telegram[] = {
        0xbc, 0x11, 0x03, 0x01, 0x01, 0xE1, 0x00, 0x80, 0x30
    };
    iolist_t iolist = {
        .iol_next = NULL,
        .iol_base = &telegram,
        .iol_len = sizeof(telegram)
    };

    result = ncn5120_send(&dev, &iolist);

    if (result < NCN5120_OK) {
        printf("Unable to send telegram: %d\n", result);
        return 1;
    }

    return 0;
}

static int cmd_flash(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("Identifying flash chip.");

    test = 7;

    /* read status register */
    uint8_t buf1[] = { 0x05, 0x00, 0x00 };

    spi_acquire(SPI_DEV(0), SPI_CS_UNDEF, SPI_MODE_0, SPI_CLK_10MHZ);
    gpio_clear(CS_PIN);
    spi_transfer_bytes(SPI_DEV(0), GPIO_UNDEF, false, &buf1, &buf1, 3);
    gpio_set(CS_PIN);
    spi_release(SPI_DEV(0));

    printf("Status: %x %x\n", buf1[1], buf1[2]);

    /* read configuration register */
    uint8_t buf2[] = { 0x15, 0x00, 0x00 };

    spi_acquire(SPI_DEV(0), SPI_CS_UNDEF, SPI_MODE_0, SPI_CLK_10MHZ);
    gpio_clear(CS_PIN);
    spi_transfer_bytes(SPI_DEV(0), GPIO_UNDEF, false, &buf2, &buf2, 3);
    gpio_set(CS_PIN);
    spi_release(SPI_DEV(0));

    printf("Configuration: %x %x\n", buf2[1], buf2[2]);

    /* read flash identifier */
    uint8_t buf3[] = { 0x9F, 0x00, 0x00, 0x00 };

    spi_acquire(SPI_DEV(0), SPI_CS_UNDEF, SPI_MODE_0, SPI_CLK_10MHZ);
    gpio_clear(CS_PIN);
    spi_transfer_bytes(SPI_DEV(0), GPIO_UNDEF, false, &buf3, &buf3, 4);
    gpio_set(CS_PIN);
    spi_release(SPI_DEV(0));

    printf("JEDEC ID: %x %x %x\n", buf3[1], buf3[2], buf3[3]);

    return 0;
}

static int cmd_while(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("Run while loop, indefinitely.");

    test = 8;

    while (1) {}

    return 0;
}

static int cmd_off(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    puts("Will turn off.");

    test = 9;

    pm_off();

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "if-out", "interface output tests", cmd_out },
    { "if-in", "interface input tests", cmd_in },
    { "i2c", "i2c input tests", cmd_i2c },
    { "prog", "prog button test", cmd_prog },
    { "clk", "clk output tests", cmd_clk },
    { "rtc", "clk output tests", cmd_rtc },
    { "knx", "KNX interface test", cmd_knx },
    { "flash", "flash chip test", cmd_flash },
    { "while", "run while loop", cmd_while },
    { "off", "turn off", cmd_off },
    { NULL, NULL, NULL }
};

int main(void)
{
    /* create background thread */
    thread_create(_stack, sizeof(_stack), THREAD_PRIORITY_MAIN + 1,
                  THREAD_CREATE_STACKTEST,
                  _background_thread, NULL, "tester");

    /* spawn shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
