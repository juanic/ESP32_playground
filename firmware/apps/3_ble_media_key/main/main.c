/**
 * @file main.c
 * @brief BLE Media Key — control multimedia keys via 6 capacitive touch pads.
 *
 * BLE HID ("ESP32 HID KB") must be connected to a host before keys are sent.
 * The 6 touch pads map to:
 *   BTN1 (T8/GPIO33) = Play/Pause    BTN4 (T4/GPIO13) = Previous Track
 *   BTN2 (T9/GPIO32) = Stop          BTN5 (T6/GPIO14) = Volume Up
 *   BTN3 (T5/GPIO12) = Next Track    BTN6 (T7/GPIO27) = Volume Down
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ble_hid_kb.h"
#include "touch_hal.h"

static const char *TAG = "main";

#define TOUCH_BTN_1     TOUCH_PAD_8   /* GPIO_33 → Play/Pause     */
#define TOUCH_BTN_2     TOUCH_PAD_9   /* GPIO_32 → Stop           */
#define TOUCH_BTN_3     TOUCH_PAD_5   /* GPIO_12 → Next Track     */
#define TOUCH_BTN_4     TOUCH_PAD_4   /* GPIO_13 → Previous Track */
#define TOUCH_BTN_5     TOUCH_PAD_6   /* GPIO_14 → Volume Up      */
#define TOUCH_BTN_6     TOUCH_PAD_7   /* GPIO_27 → Volume Down    */

#define TOUCH_PAD_COUNT 6
#define CALIB_SAMPLES   10
#define CALIB_MARGIN    500
#define LOOP_PERIOD_MS  10

static const touch_t touch_pads[TOUCH_PAD_COUNT] = {
    TOUCH_BTN_1, TOUCH_BTN_2, TOUCH_BTN_3,
    TOUCH_BTN_4, TOUCH_BTN_5, TOUCH_BTN_6
};

void app_main(void) {
    ESP_LOGI(TAG, "BLE Media Key + Touch");

    /* --- BLE HID init --- */
    ble_hid_kb_config_t cfg = BLE_HID_KB_DEFAULT_CONFIG();
    cfg.device_name = "ESP32 HID KB";

    if (!ble_hid_kb_init(&cfg)) {
        ESP_LOGE(TAG, "BLE HID init failed!");
        return;
    }
    if (!ble_hid_kb_start()) {
        ESP_LOGE(TAG, "BLE start failed!");
        return;
    }

    /* --- Touch init --- */
    TouchHalInit();
    for (int i = 0; i < TOUCH_PAD_COUNT; i++) {
        TouchHalChannelConfig(touch_pads[i], 0);
    }
    TouchHalStart();

    uint32_t accum[TOUCH_PAD_COUNT] = {0};
    ESP_LOGI(TAG, "Calibrating touch pads (%d samples)...", CALIB_SAMPLES);
    for (int s = 0; s < CALIB_SAMPLES; s++) {
        vTaskDelay(pdMS_TO_TICKS(50));
        for (int i = 0; i < TOUCH_PAD_COUNT; i++) {
            uint32_t val = 0;
            if (TouchHalRead(touch_pads[i], &val)) {
                accum[i] += val;
            }
        }
    }
    for (int i = 0; i < TOUCH_PAD_COUNT; i++) {
        uint32_t avg = accum[i] / CALIB_SAMPLES;
        uint32_t thresh = (avg > CALIB_MARGIN) ? (avg - CALIB_MARGIN) : 1;
        TouchHalSetThreshold(touch_pads[i], thresh);
        ESP_LOGI(TAG, "  Pad %d: baseline=%4" PRIu32 "  threshold=%4" PRIu32, i + 1, avg, thresh);
    }
    ESP_LOGI(TAG, "Calibration done! Waiting for BLE connection...");

    /* --- Wait for host connection --- */
    ble_hid_kb_wait_connection(0);
    ESP_LOGI(TAG, "Connected! Touch pads active.");

    /* --- Edge-detection state --- */
    bool last_touched[TOUCH_PAD_COUNT] = {false};

    /* --- Main loop --- */
    while (true) {
        if (!ble_hid_kb_is_connected()) {
            ESP_LOGW(TAG, "Disconnected, waiting...");
            ble_hid_kb_wait_connection(0);
            ESP_LOGW(TAG, "Reconnected!");
        }

        bool touched[TOUCH_PAD_COUNT];
        for (int i = 0; i < TOUCH_PAD_COUNT; i++) {
            touched[i] = TouchHalIsTouched(touch_pads[i]);
        }

        bool press[TOUCH_PAD_COUNT];
        for (int i = 0; i < TOUCH_PAD_COUNT; i++) {
            press[i] = touched[i] && !last_touched[i];
            last_touched[i] = touched[i];
        }

        if (press[0]) {
            ESP_LOGI(TAG, "Play/Pause");
            ble_hid_kb_send_media(MEDIA_KEY_PLAY_PAUSE);
        } else if (press[1]) {
            ESP_LOGI(TAG, "Stop");
            ble_hid_kb_send_media(MEDIA_KEY_STOP);
        } else if (press[2]) {
            ESP_LOGI(TAG, "Next Track");
            ble_hid_kb_send_media(MEDIA_KEY_NEXT_TRACK);
        } else if (press[3]) {
            ESP_LOGI(TAG, "Previous Track");
            ble_hid_kb_send_media(MEDIA_KEY_PREVIOUS_TRACK);
        } else if (press[4]) {
            ESP_LOGI(TAG, "Volume Up");
            ble_hid_kb_send_media(MEDIA_KEY_VOLUME_UP);
        } else if (press[5]) {
            ESP_LOGI(TAG, "Volume Down");
            ble_hid_kb_send_media(MEDIA_KEY_VOLUME_DOWN);
        }

        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}
