/*! @mainpage USB HID Consumer Control example
 *
 * @section genDesc General Description
 *
 * This example turns the ESP32-S3 native USB-OTG peripheral into a USB HID
 * device (Consumer Control / media keys) using the drivers_hal @c usb_hid_kb
 * driver (built on top of the ESP-IDF @c esp_tinyusb / TinyUSB component).
 *
 * Every 2 seconds, while a USB host is mounted, it sends one media key in a
 * round-robin sequence (play/pause, next, previous, volume up/down, stop) so
 * you can verify the link end-to-end. Mount/unmount and suspend/resume events
 * are logged by the driver.
 *
 * @section hardConn Hardware Connection
 *
 * | Peripheral | ESP32-S3 |
 * |:-----------|:---------|
 * | USB D- (GPIO_19) | connected to the board USB connector D- |
 * | USB D+ (GPIO_20) | connected to the board USB connector D+ |
 *
 * On an ESP32-S3-DevKitC the USB data lines are already wired to the native
 * USB connector, so just plug the board into the host with a USB data cable.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 28/08/2026 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "usb_hid_kb.h"
/*==================[macros and definitions]=================================*/
#define LOOP_PERIOD_MS     2000

static const char *TAG = "USB_HID_APP";

/* Round-robin sequence of media keys to demonstrate. */
static const media_key_t s_keys[] = {
    MEDIA_KEY_PLAY_PAUSE,
    MEDIA_KEY_NEXT,
    MEDIA_KEY_PREV,
    MEDIA_KEY_VOL_UP,
    MEDIA_KEY_VOL_DOWN,
    MEDIA_KEY_STOP,
};

/*==================[external functions definition]==========================*/
void app_main(void)
{
    ESP_ERROR_CHECK(usb_hid_driver_init());

    size_t idx = 0;
    while (true) {
        if (usb_hid_driver_is_connected()) {
            ESP_LOGI(TAG, "sending media key %u", (unsigned)idx);
            usb_hid_driver_send_key(s_keys[idx]);
            idx = (idx + 1) % (sizeof(s_keys) / sizeof(s_keys[0]));
        } else {
            ESP_LOGI(TAG, "waiting for a USB host (connect the cable)...");
        }
        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}
