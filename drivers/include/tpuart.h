/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @defgroup    drivers_tpuart TPUART Link Layer driver for KNX
 * @ingroup     drivers_netdev
 * @brief       Driver for Siemens TPUART transceiver
 *
 * @{
 * @file
 * @brief       Public interface for TPUART driver
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef TPUART_H
#define TPUART_H

#include <stdint.h>

#include "periph/uart.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "isrpipe.h"
#include "isrpipe/read_timeout.h"
#include "iolist.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Signature for event callback
 *
 * @param[in] event        the event that occurred.
 * @param[in] data          the byte that was received
 * @param[in] arg           context to the callback (optional)
 */
typedef void (*tpuart_cb_t)(uint8_t event, void *data, void *arg);

/**
 * @brief   Error code definition.
 */
enum {
    TPUART_OK       = 0,        /**< No error */
    TPUART_ERR_INIT = -1,       /**< Initialization error */
    TPUART_ERR_REQ  = -2,       /**< Request error (e.g. timeout) */
    TPUART_ERR_RESP = -3,       /**< Response error */
    TPUART_ERR_ARGS = -4,       /**< Argument error */
};

/**
 * @brief   TPUART device events
 */
enum {
    TPUART_EVENT_TELEGRAM,              /**< Telegram received */
    TPUART_EVENT_TELEGRAM_INCOMPLETE,   /**< Incomplete telegram received */
    TPUART_EVENT_STATE,                 /**< State indication received */
    TPUART_EVENT_SAVE,                  /**< SAVE pin triggered */
};

/**
 * @brief   TPUART device states
 */
enum {
    TPUART_STATE_IDLE,                  /**< Idle state */
    TPUART_STATE_SENDING,               /**< Sending telegram state */
    TPUART_STATE_RECEIVING,             /**< Receiving telegram state */
    TPUART_STATE_WAIT_FOR_RESPONSE,     /**< Waiting for response state */
    TPUART_STATE_COMPLETED,             /**< Telegram completed state */
};

/**
 * @brief   Device initialization parameters
 */
typedef struct {
    uart_t uart_dev;            /**< UART bus transceiver is connected to */
    gpio_t save;                /**< GPIO connected to SAVE pin (optional) */
} tpuart_params_t;

/**
 * @brief   Device structure
 */
typedef struct {
    tpuart_params_t params;     /**< TPUART initialization parameters */

    tpuart_cb_t cb;             /**< Event callback */
    void *cb_arg;               /**< Optional event callback argument */

    uint8_t state;              /**< Current TPUART state */

    uint16_t address;           /**< Current address */
    bool monitoring;            /**< True if monitoring mode is set */
    bool crc;                   /**< True if CRC checksums are enabled */
    bool busy;                  /**< True if busy mode is toggled */

    mutex_t lock;               /**< Global device lock */
    xtimer_t timer;             /**< Timer used for resetting state */
    isrpipe_t pipe;             /**< Pipe for receiving ISR data */

    uint8_t buf[265];           /**< General receive buffer (extended frame +
                                     two checksum bytes) */
    size_t buf_size;            /**< Current buffer size */

    size_t telegram_size;       /**< Size of the telegram in buffer */
    size_t min_telegram_size;   /**< Minimum expected telegram size */
    size_t max_telegram_size;   /**< Maximum expected telegram size */

    uint16_t crc_checksum;      /**< Rolling CRC checksum buffer */
    bool crc_ready;             /**< True CRC check passed after last byte */
} tpuart_t;

/**
 * @brief   Initialize the TPUART device
 *
 * @param[in] dev           Device descriptor
 * @param[in] params        Parameters for device initialization
 *
 * @return                  TPUART_OK on success
 * @return                  TPUART_ERR_INIT on initialization error
 */
int tpuart_init(tpuart_t *dev, const tpuart_params_t *params);

/**
 * @brief   Reset the TPUART device
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  TPUART_OK on success
 * @return                  TPUART_ERR_REQ on request error
 * @returns                 TPUART_ERR_RESP on response error
 */
int tpuart_reset(tpuart_t *dev);

/**
 * @brief   Retrieve the product ID from the TPUART device
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  Product identifier
 * @return                  TPUART_ERR_REQ on request error
 * @returns                 TPUART_ERR_RESP on response error
 */
int tpuart_get_product_id(tpuart_t *dev);

/**
 * @brief   Retrieve the state from the TPUART device
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  State indication
 * @return                  TPUART_ERR_REQ on request error
 * @returns                 TPUART_ERR_RESP on response error
 */
int tpuart_get_state(tpuart_t *dev);

/**
 * @brief   Activate bus monitoring mode
 *
 * @note    This mode can only be exited using a reset.
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  TPUART_OK on success
 * @return                  TPUART_ERR_REQ on request error
 * @returns                 TPUART_ERR_RESP on response error
 */
int tpuart_activate_busmon(tpuart_t *dev);

/**
 * @brief   Activate CRC checksums on incoming telegrams
 *
 * @note    This option can only be disabled using a reset. When enabled, the
 *          driver will optimize telegram reception, at the expense of false
 *          positives.
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  TPUART_OK on success
 * @return                  TPUART_ERR_REQ on request error
 * @returns                 TPUART_ERR_RESP on response error
 */
int tpuart_activate_crc(tpuart_t *dev);

/**
 * @brief   Activate busy mode
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  TPUART_OK on success
 * @return                  TPUART_ERR_REQ on request error
 * @returns                 TPUART_ERR_RESP on response error
 */
int tpuart_activate_busy_mode(tpuart_t *dev);

/**
 * @brief   Reset busy mode
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  TPUART_OK on success
 * @return                  TPUART_ERR_REQ on request error
 * @returns                 TPUART_ERR_RESP on response error
 */
int tpuart_reset_busy_mode(tpuart_t *dev);

/**
 * @brief   Send a telegram.
 *
 * @note    The maximum length supported is 64 bytes.
 *
 * @param[in] dev           Device descriptor
 * @param[in] iolist        Telegram to send
 *
 * @return                  TPUART_OK on success
 * @return                  TPUART_ERR_REQ on request error
 * @returns                 TPUART_ERR_RESP on response error
 * @returns                 TPUART_ERR_ARG on argument error
 */
int tpuart_send(tpuart_t *dev, const iolist_t *iolist);

/**
 * @brief   Set the transceiver address
 *
 * @note    When enabled, the transceiver will automatically ACK incoming
 *          telegrams, without the intervention of the CPU.
 *
 * @param[in] dev           Device descriptor
 * @param[in] address       Device address
 *
 * @return                  TPUART_OK on success
 * @return                  TPUART_ERR_REQ on request error
 * @returns                 TPUART_ERR_RESP on response error
 */
int tpuart_set_address(tpuart_t *dev, uint16_t address);

/**
 * @brief   Set the resend count
 *
 * @param[in] dev           Device descriptor
 * @param[in] busy_count    Number of retries on busy indication (<=7)
 * @param[in] nack_count    Number of retries on NACK (<=7)
 *
 * @return                  TPUART_OK on success
 * @return                  TPUART_ERR_REQ on request error
 * @returns                 TPUART_ERR_RESP on response error
 * @returns                 TPUART_ERR_ARG on argument error
 */
int tpuart_set_resend_count(tpuart_t *dev, uint8_t busy_count,
                            uint8_t nack_count);

#ifdef __cplusplus
}
#endif

#endif /* TPUART_H */
/** @} */
