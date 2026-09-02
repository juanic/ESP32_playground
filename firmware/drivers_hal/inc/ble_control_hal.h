#ifndef BLE_CONTROL_HAL_H
#define BLE_CONTROL_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup ble_control_hal BLE Control HAL
 *  @brief BLE GATT server text-line transport for device control.
 *  @{
 *
 * @section genDesc General Description
 *
 * Exposes a small Bluetooth Low Energy (BLE) GATT server used as a
 * control channel. The App (GATT client) writes ASCII command lines to
 * the CMD characteristic; the firmware replies with text lines on the
 * RSP characteristic and pushes unsolicited events on the EVT
 * characteristic.
 *
 * This HAL only handles the BLE transport (line framing + notifications).
 * It contains no command logic — the application/middleware parses the
 * received lines via the registered callback.
 *
 * Service UUID: 0xFFC0
 * | Characteristic | UUID | Properties  |
 * |----------------|------|-------------|
 * | CMD            | 0xFFC1 | Write, Write No Response |
 * | RSP            | 0xFFC2 | Notify                    |
 * | EVT            | 0xFFC3 | Notify                    |
 *
 * @note Control lines are terminated with '\n' (or '\r').
 *
 * @author Audio Relay Project
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
/*==================[macros]=================================================*/

#define BLE_CONTROL_SERVICE_UUID      0xFFC0
#define BLE_CONTROL_CMD_UUID          0xFFC1
#define BLE_CONTROL_RSP_UUID          0xFFC2
#define BLE_CONTROL_EVT_UUID          0xFFC3

#define BLE_CONTROL_MAX_LINE_LEN      128
#define BLE_CONTROL_MAX_PKT_LEN       244

/*==================[typedef]================================================*/

/**
 * @brief Callback invoked when a complete command line is received.
 *
 * @param line  Null-terminated command line (without trailing newline).
 * @param len   Length of the command line in bytes.
 * @param ctx   User context passed to ble_control_hal_init.
 */
typedef void (*ble_control_cmd_cb_t)(const char *line, uint16_t len, void *ctx);

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialize the BLE GATT control server.
 *
 * Registers GATT callbacks, creates the control service and prepares
 * advertising. Must be called AFTER bt_classic_hal_init() because the
 * BT controller and Bluedroid are shared and bootstrapped there.
 *
 * @param device_name  BLE device name (NULL for default).
 * @param cmd_cb       Callback for received command lines.
 * @param ctx          Opaque user context passed to the callback.
 * @return true if successful.
 */
bool ble_control_hal_init(const char *device_name,
                          ble_control_cmd_cb_t cmd_cb,
                          void *ctx);

/**
 * @brief Start advertising the control service.
 */
void ble_control_hal_start(void);

/**
 * @brief Send a response line back to the connected client.
 *
 * A trailing '\n' is added automatically.
 *
 * @param line  Text line to send.
 */
void ble_control_hal_send_rsp(const char *line);

/**
 * @brief Send an unsolicited event line to the connected client.
 *
 * A trailing '\n' is added automatically.
 *
 * @param line  Text line to send (e.g. "EVT METADATA ...").
 */
void ble_control_hal_send_evt(const char *line);

/**
 * @brief Check if a BLE client is currently connected.
 *
 * @return true if connected.
 */
bool ble_control_hal_is_connected(void);

/**
 * @brief Disconnect the current BLE client (if any).
 */
void ble_control_hal_disconnect(void);

/** @} */
/** @} */

#endif /* #ifndef BLE_CONTROL_HAL_H */

/*==================[end of file]============================================*/
