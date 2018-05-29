/*
 * Copyright (C) 2018-2020 Bas Stottelaar <basstottelaar@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @defgroup    drivers_ncn5120 NCN5120 Link Layer driver for KNX
 * @ingroup     drivers_netdev
 * @brief       Driver for ON Semiconductor NCN5120 transceiver
 *
 * @{
 * @file
 * @brief       Public interface for NCN5120 driver
 *
 * @author      Bas Stottelaar <basstottelaar@gmail.com>
 */

#ifndef NCN5120_H
#define NCN5120_H

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
typedef void (*ncn5120_cb_t)(uint8_t event, void *data, void *arg);

/**
 * @name NCN5120 configuration options
 * @{
 */
#define NCN5120_CONFIGURE_AUTO_POLLING     (0x04)
#define NCN5120_CONFIGURE_CRC_CTIT         (0x02)
#define NCN5120_CONFIGURE_FRAME_END_MARKER (0x01)
#define NCN5120_CONFIGURE_NONE             (0x00)
/** @} */

/**
 * @name NCN5120 register definitions
 * @{
 */
#define NCN5120_REG_EXT_WATCHDOG_CTRL (0x00)
#define NCN5120_REG_ANA_CTRL0         (0x01)
#define NCN5120_REG_ANA_CTRL1         (0x02)
#define NCN5120_REG_ANA_STAT          (0x03)
/** @} */

/**
 * @brief   Error code definition.
 */
enum {
    NCN5120_OK          = 0,        /**< No error */
    NCN5120_ERR_INIT    = -1,       /**< Initialization error */
    NCN5120_ERR_REQ     = -2,       /**< Request error (e.g. timeout) */
    NCN5120_ERR_RESP    = -3,       /**< Response error */
    NCN5120_ERR_ARGS    = -4,       /**< Argument error */
};

/**
 * @brief   NCN5120 device events
 */
enum {
    NCN5120_EVENT_TELEGRAM,             /**< Telegram received */
    NCN5120_EVENT_TELEGRAM_INCOMPLETE,  /**< Incomplete telegram received */
    NCN5120_EVENT_STATE,                /**< State indication received */
    NCN5120_EVENT_SYSTEM_STATE,         /**< System state indication received */
    NCN5120_EVENT_SAVE,                 /**< SAVE pin triggered */
};

/**
 * @brief   NCN5120 device states
 */
enum {
    NCN5120_STATE_IDLE,                 /**< Idle state */
    NCN5120_STATE_SENDING,              /**< Sending telegram state */
    NCN5120_STATE_RECEIVING,            /**< Receiving data state */
    NCN5120_STATE_WAIT_FOR_RESPONSE,    /**< Waiting for response state */
    NCN5120_STATE_COMPLETED,            /**< Telegram completed state */
};

/**
 * @brief Device initialization parameters
 */
typedef struct {
    uart_t uart_dev;            /**< UART bus transceiver is connected to */
    gpio_t save;                /**< GPIO connected to SAVE pin (optional) */
} ncn5120_params_t;

/**
 * @brief Device structure
 */
typedef struct {
    ncn5120_params_t params;    /**< NCN5120 initialization parameters */

    ncn5120_cb_t cb;            /**< Event callback */
    void *cb_arg;               /**< Optional event callback argument */

    uint8_t state;              /**< Current NCN5120 state */

    uint16_t address;           /**< Current address */
    bool monitoring;            /**< True if monitoring mode is set */
    bool crc;                   /**< True if CRC checksums are enabled */
    bool busy;                  /**< True if busy mode is toggled */
    bool stop;                  /**< Whether stop mode is active */

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
} ncn5120_t;

/**
 * @brief   Initialize the NCN5120 device
 *
 * @param[in] dev           Device descriptor
 * @param[in] params        Parameters for device initialization
 *
 * @return                  NCN5120_OK on success
 * @return                  NCN5120_ERR_INIT on initialization error
 */
int ncn5120_init(ncn5120_t *dev, const ncn5120_params_t *params);

/**
 * @brief   Activate bus monitoring mode
 *
 * @note    This mode can only be exited using a reset.
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  NCN5120_OK on success
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 */
int ncn5120_busmon(ncn5120_t *dev);

/**
 * @brief   Write configuration options to the NCN5120 device
 *
 * @param[in] dev           Device descriptor
 * @param[in] opts          Configuration options
 *
 * @return                  State indication
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 */
int ncn5120_configure(ncn5120_t *dev, uint8_t opts);

/**
 * @brief   Enter stop mode
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  NCN5120_OK on success
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 */
int ncn5120_stop_mode(ncn5120_t *dev);

/**
 * @brief   Exit stop mode
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  NCN5120_OK on success
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 */
int ncn5120_exit_stop_mode(ncn5120_t *dev);

/**
 * @brief   Retrieve the state from the NCN5120 device
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  State indication
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 */
int ncn5120_get_state(ncn5120_t *dev);

/**
 * @brief   Retrieve the system state from the NCN5120 device
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  System state indication
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 */
int ncn5120_get_system_state(ncn5120_t *dev);

/**
 * @brief   Activate busy mode
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  NCN5120_OK on success
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 */
int ncn5120_set_busy_mode(ncn5120_t *dev);

/**
 * @brief   Quit busy mode
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  NCN5120_OK on success
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 */
int ncn5120_quit_busy(ncn5120_t *dev);

/**
 * @brief   Reset the NCN5120 device
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  NCN5120_OK on success
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 */
int ncn5120_reset(ncn5120_t *dev);

/**
 * @brief   Send a telegram.
 *
 * @note    The maximum length supported is 263 bytes.
 *
 * @param[in] dev           Device descriptor
 * @param[in] iolist        Telegram to send
 *
 * @return                  NCN5120_OK on success
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 * @returns                 NCN5120_ERR_ARG on argument error
 */
int ncn5120_send(ncn5120_t *dev, const iolist_t *iolist);

/**
 * @brief   Set the transceiver address
 *
 * @note    When enabled, the transceiver will automatically ACK incoming
 *          telegrams, without the intervention of the CPU.
 *
 * @param[in] dev           Device descriptor
 * @param[in] address       Device address
 *
 * @return                  NCN5120_OK on success
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 */
int ncn5120_set_address(ncn5120_t *dev, uint16_t address);

/**
 * @brief   Set the resend count
 *
 * @param[in] dev           Device descriptor
 * @param[in] busy_count    Number of retries on busy indication (<=7)
 * @param[in] nack_count    Number of retries on NACK (<=7)
 *
 * @return                  NCN5120_OK on success
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 * @returns                 NCN5120_ERR_ARG on argument error
 */
int ncn5120_set_repetition(ncn5120_t *dev, uint8_t busy_count,
                           uint8_t nack_count);

/**
 * @brief   Write value to register
 *
 * @param[in] dev           Device descriptor
 * @param[in] addr          Register address (<= 3)
 * @param[in] value         Register value
 *
 * @return                  NCN5120_OK on success
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 * @returns                 NCN5120_ERR_ARG on argument error
 */
int ncn5120_reg_write(ncn5120_t *dev, uint8_t addr, uint8_t value);

/**
 * @brief   Read value from register
 *
 * @param[in] dev           Device descriptor
 * @param[in] addr          Register address (<= 3)
 *
 * @return                  Register value
 * @return                  NCN5120_ERR_REQ on request error
 * @returns                 NCN5120_ERR_RESP on response error
 * @returns                 NCN5120_ERR_ARG on argument error
 */
int ncn5120_reg_read(ncn5120_t *dev, uint8_t addr);

#ifdef __cplusplus
}
#endif

#endif /* NCN5120_H */
/** @} */
