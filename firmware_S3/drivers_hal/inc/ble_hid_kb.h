#ifndef BLE_HID_KB_H
#define BLE_HID_KB_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup ble_hid_kb BLE HID Keyboard HAL
 *  @brief Bluetooth LE HID (HOGP) driver for ESP32-S3.
 *  @{
 *
 * @section genDesc General Description
 *
 * This driver turns the ESP32-S3 into a Bluetooth Low Energy HID device using
 * the first-party @c esp_hid component (HID-over-GATT, HOGP) bundled with the
 * ESP-IDF. It exposes a transport-agnostic HAL API so the application does not
 * need to know anything about GATT, HID report descriptors or the Bluetooth
 * stack.
 *
 * The driver registers two HID collections:
 *   - a @b Consumer @b Control collection (media keys: play/pause, next,
 *     previous, volume up/down, stop), and
 *   - an optional @b boot @b keyboard collection (6-key roll-over).
 *
 * Pairing uses Secure Connections + bonding (MITM). The device auto-accepts
 * the security request and auto-confirms the numeric-comparison passkey, so it
 * pairs head-less ("Just Works" with bonding). All connection, disconnection
 * and authentication/bonding events are logged.
 *
 * @note This driver targets the Bluedroid Bluetooth controller
 *       (@c CONFIG_BT_BLE_ENABLED and @c CONFIG_BT_BLUEDROID_ENABLED). It does
 *       not contain any touch-pad or application (business) logic.
 *
 * @note Before calling any function, the application must initialise NVS
 *       (@c nvs_flash_init) and enable the Bluetooth controller through
 *       @ref ble_hid_driver_init (which performs the controller/Bluedroid
 *       bring-up internally).
 *
 * @note The consuming application must enable Bluetooth in its sdkconfig, e.g.:
 *       @code
 *       CONFIG_BT_ENABLED=y
 *       CONFIG_BTDM_CTRL_MODE_BLE_ONLY=y
 *       CONFIG_BT_BLUEDROID_ENABLED=y
 *       CONFIG_BT_BLE_ENABLED=y
 *       CONFIG_BT_HID_DEVICE_ENABLED=y
 *       CONFIG_BT_GATTS_ENABLE=y
 *       CONFIG_BT_GATT_BLE_MESH_COEX_SUPPORT=n
 *       @endcode
 *       (the @c esp_hid component and the @c bt component are pulled in
 *       automatically by the drivers_hal component via PRIV_REQUIRES).
 *
 * @author Juan Cerrudo
 *
 * @section changelog
 *
 * |   Date	    | Description                                    						|
 * |:----------:|:----------------------------------------------------------------------|
 * | 28/08/2026 | Document creation		                         						|
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include "esp_err.h"
#include "hid_common.h"   /* shared media_key_t + usage mapping */

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief The media-key type @ref media_key_t and its usage mapping are defined
 *        in hid_common.h so they can be shared with the USB HID transport.
 */

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/
/**
 * @brief Initialise the BLE HID driver and start advertising.
 *
 * Brings up the Bluetooth controller (BLE only) and Bluedroid, registers the
 * HID-over-GATT service, configures Secure-Connections bonding and starts
 * advertising under the given device name. The device is then discoverable
 * and pairable.
 *
 * @param device_name NULL-terminated advertising / device name (max 24 chars).
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ble_hid_driver_init(const char *device_name);

/**
 * @brief Send a Consumer Control (media) key press and release it.
 *
 * The key is reported as pressed and, after a short debounce, released, so the
 * host sees a single key stroke. Does nothing if no host is connected.
 *
 * @param key Media key to send (see @ref media_key_t).
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ble_hid_driver_send_key(media_key_t key);

/**
 * @brief Send a boot-keyboard report (modifier + up to 6 keycodes) and release.
 *
 * Optional helper that uses the boot-keyboard collection. The report is sent
 * pressed and then released (all zeros) shortly after.
 *
 * @param modifier 8-bit modifier byte (bit0 = Left Ctrl ... bit7 = Right GUI).
 * @param keycodes pointer to 6 bytes with the USB keycode array (may be NULL
 *        for an empty/report-only release).
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ble_hid_driver_send_keyboard(uint8_t modifier, const uint8_t keycodes[6]);

/**
 * @brief Returns true if a host is currently connected.
 *
 * @return true  a BLE HID host is connected,
 * @return false otherwise.
 */
bool ble_hid_driver_is_connected(void);

/**
 * @brief Remove all bonded (paired) devices from the Bluetooth store.
 *
 * Useful to force a new pairing after changing the host. The stack will log
 * the number of bonds removed.
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ble_hid_driver_remove_bonds(void);

/**
 * @brief De-initialise the driver and stop advertising.
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t ble_hid_driver_deinit(void);

/** @} */
/** @} */

#endif /* #ifndef BLE_HID_KB_H */

/*==================[end of file]============================================*/
