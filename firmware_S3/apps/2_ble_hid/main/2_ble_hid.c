/*! @mainpage BLE HID Consumer Control example
 *
 * @section genDesc General Description
 *
 * This example turns the ESP32-S3 into a Bluetooth LE HID device (Consumer
 * Control / media keys) using the drivers_hal @c ble_hid_kb driver (which is
 * built on top of the ESP-IDF @c esp_hid component, HOGP profile).
 *
 * Every 2 seconds, while a host is connected, it sends one media key in a
 * round-robin sequence (play/pause, next, previous, volume up/down, stop) so
 * you can verify the link end-to-end.
 *
 * Pairing uses Secure Connections + bonding; the driver logs connection,
 * disconnection and authentication/bonding events.
 *
 * @section hardConn Hardware Connection
 *
 * | Peripheral | ESP32-S3 |
 * |:-----------|:---------|
 * | No extra wiring required for BLE. Power the board over USB (for the
 *   console / flashing only — BLE does not use the USB data lines). | |
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
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ble_hid_kb.h"
/*==================[macros and definitions]=================================*/
#define LOOP_PERIOD_MS     2000

static const char *TAG = "BLE_HID_APP";

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
    /* NVS is required by Bluedroid to store the bond/pairing information. */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(ble_hid_driver_init("ESP32-S3 HID"));

    size_t idx = 0;
    while (true) {
        if (ble_hid_driver_is_connected()) {
            ESP_LOGI(TAG, "sending media key %u", (unsigned)idx);
            ble_hid_driver_send_key(s_keys[idx]);
            idx = (idx + 1) % (sizeof(s_keys) / sizeof(s_keys[0]));
        } else {
            ESP_LOGI(TAG, "waiting for a BLE host to connect...");
        }
        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}
