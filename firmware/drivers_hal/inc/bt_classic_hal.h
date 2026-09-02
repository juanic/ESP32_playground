#ifndef BT_CLASSIC_HAL_H
#define BT_CLASSIC_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup bt_classic_hal BT Classic HAL
 *  @brief Bluetooth Classic (BR/EDR) A2DP driver for ESP32.
 *  @{
 *
 * @section genDesc General Description
 *
 * Wrapper over ESP-IDF Bluedroid stack for Bluetooth Classic A2DP
 * Sink and Source roles. Handles BT controller init, Bluedroid init,
 * GAP registration, A2DP profile init and connection management.
 *
 * @note A2DP Sink and Source cannot run simultaneously on the same
 * ESP32 (Bluedroid limitation).
 *
 * @author Audio Relay Project
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==================[macros]=================================================*/

#define BT_CLASSIC_BD_ADDR_LEN  6

/*==================[typedef]================================================*/

/**
 * @brief GAP event callback type.
 *        Called from the BT task context.
 */
typedef void (*bt_classic_gap_cb_t)(uint8_t event, uint8_t *param);

/**
 * @brief A2DP sink data callback type.
 *        Called when decoded PCM audio data is available.
 *
 * @param data  Pointer to PCM data (16-bit stereo, 44.1 kHz by default).
 * @param len   Length of data in bytes.
 */
typedef void (*bt_classic_sink_data_cb_t)(const uint8_t *data, uint32_t len);

/**
 * @brief A2DP source state callback type.
 *
 * @param connected  true if A2DP source is connected, false if disconnected.
 */
typedef void (*bt_classic_source_state_cb_t)(bool connected);

/**
 * @brief Supply PCM to the pull-based A2DP source callback.
 *
 * Called from the Bluetooth task. It may return fewer bytes than requested;
 * the HAL fills the remainder with silence.
 */
typedef int32_t (*bt_classic_source_data_cb_t)(uint8_t *data, int32_t len);

/** Called for every named device found by an explicit source-side scan. */
typedef void (*bt_classic_source_scan_cb_t)(const char *name, const uint8_t *bda);

/**
 * @brief A2DP connection state callback (sink or source role).
 *
 * @param connected  true if a peer is connected, false if disconnected.
 */
typedef void (*bt_classic_conn_state_cb_t)(bool connected, const uint8_t *peer_bda);

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialize the Bluetooth Classic stack (NVS, controller, Bluedroid).
 *        Must be called once before any other bt_classic_hal_* function.
 *
 * @param device_name  Local BT device name (max 32 chars).
 * @return true if successful.
 */
bool bt_classic_hal_init(const char *device_name);

/**
 * @brief Register a connection-state callback.
 *
 * Called when the A2DP connection state changes (sink or source role).
 * The peer BDA pointer is valid only while connected.
 *
 * @param cb  Callback (NULL to clear).
 */
void bt_classic_hal_register_conn_cb(bt_classic_conn_state_cb_t cb);

/**
 * @brief Initialize A2DP sink profile.
 *
 * @param data_cb  Callback for received PCM audio data.
 * @return true if successful.
 */
bool bt_classic_hal_init_a2dp_sink(bt_classic_sink_data_cb_t data_cb);

/**
 * @brief Initialize A2DP source profile.
 *
 * @param state_cb  Callback for connection state changes.
 * @return true if successful.
 */
bool bt_classic_hal_init_a2dp_source(bt_classic_source_state_cb_t state_cb,
                                     bt_classic_source_data_cb_t data_cb);

/**
 * @brief Start A2DP source discovery and connect to a target device.
 *
 * @param target_name  Name of the remote A2DP sink to connect to.
 * @return true if discovery started.
 */
bool bt_classic_hal_source_start_discovery(const char *target_name);

/** Start a finite Bluetooth Classic scan without connecting to a result. */
bool bt_classic_hal_source_start_scan(bt_classic_source_scan_cb_t scan_cb);

/**
 * @brief Connect A2DP source to a specific device by address.
 *
 * @param bda  Bluetooth Device Address (6 bytes).
 * @return true if connection initiated.
 */
bool bt_classic_hal_source_connect(const uint8_t *bda);

/**
 * @brief Write audio data to A2DP source (called from data callback).
 *
 * @param data  PCM data buffer.
 * @param len   Data length in bytes.
 * @return Number of bytes consumed, or -1 on error.
 */
int bt_classic_hal_source_write(const uint8_t *data, int32_t len);

/**
 * @brief Disconnect A2DP from the current peer (sink or source role).
 */
void bt_classic_hal_disconnect(void);

/**
 * @brief Make the device discoverable and connectable (for sink role).
 */
void bt_classic_hal_set_discoverable(void);

/**
 * @brief Check if A2DP is currently connected.
 *
 * @return true if connected.
 */
bool bt_classic_hal_is_connected(void);

/**
 * @brief Get the Bluetooth Device Address of the local device.
 *
 * @param bda  Output buffer (6 bytes).
 */
void bt_classic_hal_get_own_bda(uint8_t *bda);

/**
 * @brief Get the Bluetooth Device Address of the connected peer.
 *
 * @param bda  Output buffer (6 bytes).
 * @return true if a peer is connected.
 */
bool bt_classic_hal_get_peer_bda(uint8_t *bda);

#ifdef __cplusplus
}
#endif

/** @} */
/** @} */

#endif /* #ifndef BT_CLASSIC_HAL_H */

/*==================[end of file]============================================*/
