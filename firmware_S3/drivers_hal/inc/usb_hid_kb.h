#ifndef USB_HID_KB_H
#define USB_HID_KB_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup usb_hid_kb USB HID Keyboard HAL
 *  @brief USB HID (TinyUSB) Consumer Control driver for ESP32-S3.
 *  @{
 *
 * @section genDesc General Description
 *
 * This driver turns the ESP32-S3 native USB-OTG peripheral into a USB HID
 * device using the first-party @c esp_tinyusb (TinyUSB) component. It exposes
 * the same transport-agnostic HAL API as the BLE HID driver, so the two are
 * interchangeable from the application point of view.
 *
 * The device is configured as a @b Consumer @b Control HID class device using
 * the standard TinyUSB consumer-control report descriptor
 * (@c TUD_HID_REPORT_DESC_CONSUMER), i.e. it sends media keys (play/pause,
 * next, previous, volume up/down, stop).
 *
 * Connection state is taken directly from TinyUSB: @ref usb_hid_driver_is_connected
 * returns @c tud_mounted(). Mount/unmount and suspend/resume events are logged
 * through the TinyUSB callbacks @c tud_mount_cb / @c tud_umount_cb /
 * @c tud_suspend_cb / @c tud_resume_cb.
 *
 * @note This driver is only compiled when the HID transport includes USB
 *       (CONFIG_HID_TRANSPORT_USB or CONFIG_HID_TRANSPORT_BOTH, see the
 *       drivers_hal Kconfig) and requires CONFIG_TINYUSB_HID_ENABLED=y.
 *
 * @note The application must enable TinyUSB in its sdkconfig
 *       (CONFIG_TINYUSB_ENABLED=y, CONFIG_TINYUSB_HID_ENABLED=y). No touch-pad
 *       or business logic is present — this is a pure HID/USB transport driver.
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
#include "hid_common.h"   /* shared media_key_t */

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[internal functions declaration]=========================*/
/**
 * @brief Initialise the USB HID driver (installs TinyUSB as a HID device).
 *
 * Configures the ESP32-S3 USB-OTG as a Consumer Control HID device, installs
 * the TinyUSB stack and starts the USB task. The device enumerates on the host
 * as a media/consumer HID device.
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t usb_hid_driver_init(void);

/**
 * @brief Send a Consumer Control (media) key press and release it.
 *
 * The 16-bit Consumer usage code is sent as the HID report and, after a short
 * debounce, released (zero report) so the host sees a single key stroke.
 * Does nothing if the USB host is not mounted.
 *
 * @param key Media key to send (see @ref media_key_t).
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t usb_hid_driver_send_key(media_key_t key);

/**
 * @brief Returns true if a USB host is currently mounted (enumerated).
 *
 * @return true  a USB host is connected,
 * @return false otherwise.
 */
bool usb_hid_driver_is_connected(void);

/**
 * @brief De-initialise the USB HID driver (uninstalls TinyUSB).
 *
 * @return ESP_OK on success, otherwise an ESP-IDF error code.
 */
esp_err_t usb_hid_driver_deinit(void);

/** @} */
/** @} */

#endif /* #ifndef USB_HID_KB_H */

/*==================[end of file]============================================*/
