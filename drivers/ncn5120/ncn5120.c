/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ncn5120
 *
 * @{
 * @file
 * @brief       Implementation of the NCN5120 driver
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 * @}
 */

#include <string.h>

#include "byteorder.h"
#include "checksum/crc16_ccitt.h"
#include "errno.h"
#include "ncn5120_internals.h"
#include "ncn5120.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/**
 * @brief Perform a request and wait for a response.
 */
int _request(ncn5120_t *dev, uint8_t *buf, size_t out_size, size_t in_size)
{
    int result = NCN5120_OK;

    /* lock device */
    mutex_lock(&(dev->lock));

    /* set state */
    dev->state = NCN5120_STATE_WAIT_FOR_RESPONSE;

    /* write data to device */
    uart_write(dev->params.uart_dev, buf, out_size);

    /* wait for response */
    if (isrpipe_read_all_timeout(&(dev->pipe), buf, in_size, NCN5120_TIMEOUT_WAIT_FOR_RESPONSE) == -ETIMEDOUT) {
        result = NCN5120_ERR_REQ;
    }

    /* reset state */
    dev->state = NCN5120_STATE_IDLE;

    /* unlock device */
    mutex_unlock(&(dev->lock));

    return result;
}

/**
 * @brief Complete a telegram and raise an event.
 */
static void _complete_telegram(ncn5120_t *dev, size_t telegram_size)
{
    /* set the telegram size */
    dev->telegram_size = telegram_size;

    /* invoke callback depending on whether the telegram looks/is complete */
    if (dev->telegram_size < dev->min_telegram_size || dev->telegram_size > dev->max_telegram_size) {
        DEBUG("[ncn5120] _complete_telegram: telegram incomplete (%d/%d bytes)\n", dev->telegram_size, dev->min_telegram_size);

        if (dev->cb != NULL) {
            dev->cb(NCN5120_EVENT_TELEGRAM_INCOMPLETE, NULL, (void *)dev->cb_arg);
        }
    }
    else if (dev->crc && !dev->crc_ready) {
        DEBUG("[ncn5120] _complete_telegram: telegram corrupt (%d bytes)\n", dev->telegram_size);

        if (dev->cb != NULL) {
            dev->cb(NCN5120_EVENT_TELEGRAM_INCOMPLETE, NULL, (void *)dev->cb_arg);
        }
    }
    else {
        DEBUG("[ncn5120] _complete_telegram: telegram complete (%d bytes)\n", dev->telegram_size);

        if (dev->cb != NULL) {
            dev->cb(NCN5120_EVENT_TELEGRAM, NULL, (void *)dev->cb_arg);
        }
    }

    /* mark as completed */
    dev->state = NCN5120_STATE_COMPLETED;
}

/**
 * @brief Stop receiving and return to idle state.
 */
static void _receive_stop(ncn5120_t *dev)
{
    /* complete telegram if not completed before */
    if (dev->state == NCN5120_STATE_RECEIVING) {
        _complete_telegram(dev, dev->buf_size);
    }

    /* reset state */
    dev->state = NCN5120_STATE_IDLE;

    /* unlock device */
    mutex_unlock(&(dev->lock));
}

/**
 * @brief Receive a single byte.
 */
static void _receive(ncn5120_t *dev, uint8_t byte)
{
    /* reset end-of-telegram gap timer */
    xtimer_set(&(dev->timer), NCN5120_TIMEOUT_END_OF_TELEGRAM);

    /* receive byte */
    if (dev->buf_size == sizeof(dev->buf)) {
        DEBUG("[ncn5120] _receive: buffer full, dropping\n");
        return;
    }

    dev->buf[dev->buf_size++] = byte;

    /* if CRC checksum is enabled, it is possible to check if the telegram
       is/looks complete without waiting for the end-of-telegram gap first
       (there might be a false positive, but in that case the telegram is
       completed again, as long as the end-of-telegram gap has not been
       observed) */
    if (dev->crc) {
        /* update the CRC checksum (skipping the CRC checksum bytes) */
        if (dev->buf_size > 2) {
            uint8_t last_byte = dev->buf[dev->buf_size - 3];

            dev->crc_checksum = crc16_ccitt_update(dev->crc_checksum, &last_byte, 1);
        }

        /* evaluate checksum when enough data is received */
        if (dev->buf_size >= (dev->min_telegram_size + 2)) {
            network_uint16_t *tmp = (network_uint16_t *)&dev->buf[dev->buf_size - 2];
            uint16_t crc_checksum = byteorder_ntohs(*tmp);

            // DEBUG("[ncn5120] _receive: expected %04x, received %04x\n", dev->crc_checksum, crc_checksum);

            if (crc_checksum == dev->crc_checksum) {
                /* mark CRC as ready */
                dev->crc_ready = true;

                /* complete telegram */
                _complete_telegram(dev, dev->buf_size - 2);
            }
            else {
                dev->crc_ready = false;
            }
        }
    }
}

/**
 * @brief Start receiving data.
 */
static void _receive_start(ncn5120_t *dev, uint8_t byte, size_t min_telegram_size, size_t max_telegram_size)
{
    if (mutex_trylock(&(dev->lock)) == 0) {
        DEBUG("[ncn5120] _receive_start: cannot lock, dropping\n");
        return;
    }

    /* prepare for receiving */
    dev->state = NCN5120_STATE_RECEIVING;
    dev->buf_size = 0;
    dev->telegram_size = 0;
    dev->min_telegram_size = min_telegram_size;
    dev->max_telegram_size = max_telegram_size;
    dev->crc_checksum = 0xFFFF;
    dev->crc_ready = false;

    /* receive the first byte */
    _receive(dev, byte);
}

/**
 * @brief Continue receiving.
 */
static void _receive_continue(ncn5120_t *dev, uint8_t byte)
{
    /* more data arrived, so continue */
    dev->state = NCN5120_STATE_RECEIVING;

    /* process the byte */
    _receive(dev, byte);
}

/**
 * @brief UART callback.
 */
static void _cb_uart(void *arg, uint8_t byte)
{
    ncn5120_t *dev = arg;

    /* handle byte based on state */
    // DEBUG("[ncn5120] _cb_uart: incoming %02x\n", byte);

    switch (dev->state) {
        case NCN5120_STATE_IDLE:
            if (NCN5120_MATCHES(byte, NCN5120_SERVICE_L_DATA_STANDARD_INDICATION)) {
                _receive_start(dev, byte, 8, 23);
            }
            else if (NCN5120_MATCHES(byte, NCN5120_SERVICE_L_DATA_EXTENDED_INDICATION)) {
                _receive_start(dev, byte, 9, 263);
            }
            else if (NCN5120_MATCHES(byte, NCN5120_SERVICE_L_POLL_DATA_INDICATION)) {
                _receive_start(dev, byte, 6, 6);
            }
            else if (byte & NCN5120_SERVICE_U_STATE_INDICATION) {
                if (dev->cb != NULL) {
                    dev->cb(NCN5120_EVENT_STATE, &byte, (void *)dev->cb_arg);
                }
            }
            else if (byte & NCN5120_SERVICE_U_SYSTEM_STATE_INDICATION) {
                if (dev->cb != NULL) {
                    dev->cb(NCN5120_EVENT_SYSTEM_STATE, &byte, (void *)dev->cb_arg);
                }
            }
            else {
                DEBUG("[ncn5120] _cb_uart: unexpected control byte: 0x%02x\n", byte);
            }

            break;
        case NCN5120_STATE_RECEIVING:
            _receive(dev, byte);
            break;
        case NCN5120_STATE_COMPLETED:
            _receive_continue(dev, byte);
            break;
        case NCN5120_STATE_SENDING:
        case NCN5120_STATE_WAIT_FOR_RESPONSE:
            if (isrpipe_write_one(&(dev->pipe), byte) != 0) {
                DEBUG("[ncn5120] _cb_uart: unable to write byte to pipe\n");
            }
            break;
    }
}

/**
 * @brief SAVE pin callback.
 */
static void _cb_save(void *arg)
{
    ncn5120_t *dev = arg;

    if (dev->cb != NULL) {
        dev->cb(NCN5120_EVENT_SAVE, NULL, (void *)dev->cb_arg);
    }
}

/**
 * @brief Timer callback.
 */
static void _cb_timer(void *arg)
{
    ncn5120_t *dev = arg;

    /* stop receiving */
    DEBUG("[ncn5120] _cb_timer: end-of-telegram gap\n");
    _receive_stop(dev);
}

int ncn5120_init(ncn5120_t *dev, const ncn5120_params_t *params)
{
    dev->params = *params;

    /* initialize mutex */
    mutex_init(&(dev->lock));

    /* initialize isrpipe */
    isrpipe_init(&(dev->pipe), dev->buf, 16);

    /* setup descriptor */
    dev->state = NCN5120_STATE_IDLE;
    dev->address = 0;
    dev->crc = false;
    dev->monitoring = false;
    dev->busy = false;
    dev->stop = false;

    /* setup gap timer */
    dev->timer.callback = _cb_timer;
    dev->timer.arg = (void *)dev;
    dev->timer.target = 0;
    dev->timer.long_target = 0;

    /* initialize UART */
    uart_poweron(dev->params.uart_dev);

    if (uart_init(dev->params.uart_dev, 19200, _cb_uart, dev) != UART_OK) {
        DEBUG("[ncn5120] init: error initializing UART\n");
        return NCN5120_ERR_INIT;
    }

    if (uart_mode(dev->params.uart_dev, UART_DATA_BITS_8, UART_PARITY_EVEN, UART_STOP_BITS_1) != UART_OK) {
        DEBUG("[tpuart] init: error setting mode\n");
        return NCN5120_ERR_INIT;
    }

    /* initialize SAVE pin */
    if (dev->params.save != GPIO_UNDEF) {
        if (gpio_init_int(dev->params.save, GPIO_IN, GPIO_FALLING, _cb_save, dev) != 0) {
            DEBUG("[ncn5120] init: error initializing SAVE pin\n");
            return NCN5120_ERR_INIT;
        }
    }

    DEBUG("[ncn5120] init: driver initialized\n");
    return NCN5120_OK;
}

int ncn5120_busmon(ncn5120_t *dev)
{
    uint8_t buf[] = { NCN5120_SERVICE_U_BUSMON_REQUEST };

    if (_request(dev, buf, 1, 0) != 0) {
        return NCN5120_ERR_REQ;
    }

    /* enable bus monitoring */
    dev->monitoring = true;

    return NCN5120_OK;
}

int ncn5120_configure(ncn5120_t *dev, uint8_t opts)
{
    if (opts > 7) {
        return NCN5120_ERR_ARGS;
    }

    uint8_t buf[] = { NCN5120_SERVICE_U_CONFIGURE_REQUEST | opts };

    if (_request(dev, buf, 1, 1) != 0) {
        return NCN5120_ERR_REQ;
    }

    if (!(buf[0] & NCN5120_SERVICE_U_CONFIGURE_INDICATION)) {
        return NCN5120_ERR_RESP;
    }

    /* enable crc */
    dev->crc = (opts & NCN5120_CONFIGURE_CRC_CTIT);

    return NCN5120_OK;
}

int ncn5120_set_busy_mode(ncn5120_t *dev)
{
    uint8_t buf[] = { NCN5120_SERVICE_U_SET_BUSY_REQUEST };

    if (_request(dev, buf, 1, 0) != 0) {
        return NCN5120_ERR_REQ;
    }

    /* enable busy mode */
    dev->busy = true;

    return NCN5120_OK;
}

int ncn5120_quit_busy(ncn5120_t *dev)
{
    uint8_t buf[] = { NCN5120_SERVICE_U_QUIT_BUSY_REQUEST };

    if (_request(dev, buf, 1, 0) != 0) {
        return NCN5120_ERR_REQ;
    }

    /* enable busy mode */
    dev->busy = false;

    return NCN5120_OK;
}

int ncn5120_stop_mode(ncn5120_t *dev)
{
    uint8_t buf[] = { NCN5120_SERVICE_U_STOP_MODE };

    if (_request(dev, buf, 1, 1) != 0) {
        return NCN5120_ERR_REQ;
    }

    if (!(buf[0] & NCN5120_SERVICE_U_STOP_MODE_INDICATION)) {
        return NCN5120_ERR_RESP;
    }

    /* enable stop mode */
    dev->stop = true;

    return NCN5120_OK;
}

int ncn5120_exit_stop_mode(ncn5120_t *dev)
{
    uint8_t buf[] = { NCN5120_SERVICE_U_EXIT_STOP_MODE_REQUEST };

    if (_request(dev, buf, 1, 1) != 0) {
        return NCN5120_ERR_REQ;
    }

    if (!(buf[0] & NCN5120_SERVICE_U_RESET_INDICATION)) {
        return NCN5120_ERR_RESP;
    }

    /* enable stop mode */
    dev->stop = false;

    return NCN5120_OK;
}

int ncn5120_set_address(ncn5120_t *dev, uint16_t address)
{
    uint8_t buf[] = {
        NCN5120_SERVICE_U_SET_ADDRESS_REQUEST,
        address & 0xff,
        (address >> 8) & 0xff,
        0x00
    };

    if (_request(dev, buf, 4, 1) != 0) {
        return NCN5120_ERR_REQ;
    }

    if (!(buf[0] & NCN5120_SERVICE_U_CONFIGURE_INDICATION)) {
        return NCN5120_ERR_RESP;
    }

    /* store address */
    dev->address = address;

    return NCN5120_OK;
}

int ncn5120_set_repetition(ncn5120_t *dev, uint8_t busy_count, uint8_t nack_count)
{
    if (busy_count > 7 || nack_count > 7) {
        return NCN5120_ERR_ARGS;
    }

    uint8_t buf[] = {
        NCN5120_SERVICE_U_SET_REPETITION_REQUEST,
        ((busy_count & 0b111) << 5) + (nack_count & 0b111),
        0x00,
        0x00
    };

    if (_request(dev, buf, 4, 0) != 0) {
        return NCN5120_ERR_REQ;
    }

    return NCN5120_OK;
}

int ncn5120_reset(ncn5120_t *dev)
{
    uint8_t buf[] = { NCN5120_SERVICE_U_RESET_REQUEST, 0x00 };

    if (_request(dev, buf, 1, 1) != 0) {
        return NCN5120_ERR_REQ;
    }

    if (!(buf[0] & NCN5120_SERVICE_U_RESET_INDICATION)) {
        return NCN5120_ERR_RESP;
    }

    /* need to clear internal state after reset */
    dev->address = 0;
    dev->crc = false;
    dev->monitoring = false;
    dev->busy = false;
    dev->stop = false;

    return NCN5120_OK;
}

int ncn5120_get_state(ncn5120_t *dev)
{
    uint8_t buf[] = { NCN5120_SERVICE_U_STATE_REQUEST };

    if (_request(dev, buf, 1, 1) != 0) {
        return NCN5120_ERR_REQ;
    }

    if (!(buf[0] & NCN5120_SERVICE_U_STATE_INDICATION)) {
        return NCN5120_ERR_RESP;
    }

    return buf[0];
}

int ncn5120_get_system_state(ncn5120_t *dev)
{
    uint8_t buf[] = { NCN5120_SERVICE_U_SYSTEM_STATE_REQUEST, 0x00 };

    if (_request(dev, buf, 1, 2) != 0) {
        return NCN5120_ERR_REQ;
    }

    if (buf[0] != NCN5120_SERVICE_U_SYSTEM_STATE_INDICATION) {
        return NCN5120_ERR_RESP;
    }

    return buf[1];
}

int ncn5120_reg_write(ncn5120_t *dev, uint8_t addr, uint8_t value)
{
    if (addr > 3) {
        return NCN5120_ERR_ARGS;
    }

    uint8_t buf[] = { NCN5120_SERVICE_U_INT_REG_WR_REQUEST | addr, value };

    if (_request(dev, buf, 2, 0) != 0) {
        return NCN5120_ERR_REQ;
    }

    return NCN5120_OK;
}

int ncn5120_reg_read(ncn5120_t *dev, uint8_t addr)
{
    if (addr > 3) {
        return NCN5120_ERR_ARGS;
    }

    uint8_t buf[] = { NCN5120_SERVICE_U_INT_REG_RD_REQUEST | addr, 0x00 };

    if (_request(dev, buf, 1, 1) != 0) {
        return NCN5120_ERR_REQ;
    }

    return buf[0];
}

int ncn5120_send(ncn5120_t *dev, const iolist_t *iolist)
{
    int result;
    int bytes = 0;

    size_t size = iolist_size(iolist);

    DEBUG("[ncn5120] ncn5120_send: sending telegram of size %d\n", size);

    /* max telegram supported is 263 bytes */
    if (size > 263) {
        return NCN5120_ERR_ARGS;
    }

    /* lock device */
    mutex_lock(&(dev->lock));

    /* set state */
    dev->state = NCN5120_STATE_SENDING;

    /* write data to device */
    uint8_t buf[2];
    uint8_t offset = 0;

    for (const iolist_t *iol = iolist; iol; iol = iol->iol_next) {
        for (unsigned i = 0; i < iol->iol_len; i++) {
            uint16_t byte = (bytes + i);

            /* set data offset when the three most significant bits change */
            if (((byte >> 6) & 0x07) != offset) {
                offset = ((byte >> 6) & 0x07);

                buf[0] = NCN5120_SERVICE_U_L_DATA_OFFSET_REQUEST | offset;

                // DEBUG("[ncn5120] ncn5120_send: offset set to %d after %d\n", offset, byte);
                uart_write(dev->params.uart_dev, buf, 1);
            }

            if (i == (iol->iol_len - 1) && iol->iol_next == NULL) {
                buf[0] = NCN5120_SERVICE_U_L_DATA_END_REQUEST | (byte & 0x3f);
                buf[1] = ((uint8_t *)iol->iol_base)[i];
            }
            else {
                buf[0] = NCN5120_SERVICE_U_L_DATA_CONTINUE_REQUEST | (byte & 0x3f);
                buf[1] = ((uint8_t *)iol->iol_base)[i];
            }

            // DEBUG("[ncn5120] ncn5120_send: sending %02x %02x\n", buf[0], buf[1]);
            uart_write(dev->params.uart_dev, buf, 2);
        }

        /* increment count */
        bytes += iol->iol_len;
    }

    /* after sending last byte, the same telegram will be echoed back, so the
       whole telegram is expected, plus the data confirmation byte after the
       end-of-telegram gap */
    int expected = bytes + 1 + (dev->crc ? 2 : 0);

    while (expected) {
        if (isrpipe_read_all_timeout(&(dev->pipe), buf, 1, NCN5120_TIMEOUT_WAIT_FOR_ACK) == -ETIMEDOUT) {
            break;
        }

        expected--;
    }

    /* when number of expected bytes is zero, then the last received byte in
       buf contains the data confirmation byte */
    if (expected == 0 && buf[0] & NCN5120_SERVICE_L_DATA_CONFIRM) {
        if (buf[0] & 0b10000000) {
            DEBUG("[ncn5120] ncn5120_send: ACK received\n");
            result = bytes;
        }
        else {
            DEBUG("[ncn5120] ncn5120_send: NACK received\n");
            result = NCN5120_ERR_RESP;
        }
    }
    else {
        DEBUG("[ncn5120] ncn5120_send: request error, expected %d more bytes\n", expected);
        result = NCN5120_ERR_REQ;
    }

    /* reset state */
    dev->state = NCN5120_STATE_IDLE;

    /* unlock device */
    mutex_unlock(&(dev->lock));

    return result;
}
