/**
 * @file main.c
 * @brief Audio Relay Source — reads I2S and transmits via A2DP.
 *
 * This app reads PCM audio from I2S input (connected to the sink board's
 * I2S output) and transmits it as A2DP source to a Bluetooth amplifier.
 * Includes auto-reconnection logic.
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/stream_buffer.h"
#include "esp_log.h"
#include "audio_relay_source_bsp.h"
#include "bt_classic_hal.h"
#include "i2s_hal.h"

/*==================[macros and definitions]=================================*/

#define DEVICE_NAME         "Audio Relay Src"
#define TARGET_BT_NAME      "BT-WUZHI"
#define I2S_READ_BUF_SIZE   1024
#define I2S_READ_TIMEOUT_MS 100
#define RECONNECT_DELAY_MS  3000
#define PCM_BUFFER_SIZE     (32 * 1024)

/*==================[internal data definition]===============================*/

static const char *TAG = "audio_relay_source";
static bool s_source_connected = false;
static StaticStreamBuffer_t s_pcm_buffer_storage;
static uint8_t s_pcm_buffer_memory[PCM_BUFFER_SIZE];
static StreamBufferHandle_t s_pcm_buffer;

/*==================[internal functions declaration]=========================*/

static void on_source_state(bool connected);
static int32_t on_source_data(uint8_t *data, int32_t len);

/*==================[external functions definition]==========================*/

static void on_source_state(bool connected) {
    s_source_connected = connected;
    if (connected) {
        ESP_LOGI(TAG, "A2DP source connected to amplifier");
    } else {
        ESP_LOGW(TAG, "A2DP source disconnected, will reconnect...");
    }
}

/* Called by Bluedroid when it needs PCM for the outgoing A2DP frame. */
static int32_t on_source_data(uint8_t *data, int32_t len) {
    if (!s_source_connected || !s_pcm_buffer || !data || len <= 0) {
        return 0;
    }
    return (int32_t)xStreamBufferReceive(s_pcm_buffer, data, (size_t)len, 0);
}

void app_main(void) {
    ESP_LOGI(TAG, "Audio Relay Source starting...");

    s_pcm_buffer = xStreamBufferCreateStatic(PCM_BUFFER_SIZE, 1,
                                             s_pcm_buffer_memory,
                                             &s_pcm_buffer_storage);
    if (!s_pcm_buffer) {
        ESP_LOGE(TAG, "PCM buffer creation failed");
        return;
    }

    /* Initialize BSP (I2S RX + A2DP Source) */
    if (!AudioRelaySourceBspInit(DEVICE_NAME, on_source_state, on_source_data)) {
        ESP_LOGE(TAG, "BSP init failed!");
        return;
    }

    /* Start discovery for target amplifier */
    ESP_LOGI(TAG, "Looking for target: %s", TARGET_BT_NAME);
    bt_classic_hal_source_start_discovery(TARGET_BT_NAME);

    /* I2S read buffer */
    uint8_t i2s_buf[I2S_READ_BUF_SIZE];

    /* Main loop: read I2S → forward to A2DP source */
    while (true) {
        if (!s_source_connected) {
            /* Wait for connection, retry discovery periodically */
            vTaskDelay(pdMS_TO_TICKS(RECONNECT_DELAY_MS));
            if (!bt_classic_hal_is_connected()) {
                ESP_LOGI(TAG, "Retrying discovery...");
                bt_classic_hal_source_start_discovery(TARGET_BT_NAME);
            }
            continue;
        }

        /* Read audio from I2S */
        int bytes_read = I2sHalRead(i2s_buf, I2S_READ_BUF_SIZE, I2S_READ_TIMEOUT_MS);
        if (bytes_read > 0) {
            size_t queued = xStreamBufferSend(s_pcm_buffer, i2s_buf,
                                              (size_t)bytes_read, 0);
            if (queued != (size_t)bytes_read) {
                ESP_LOGW(TAG, "PCM buffer full; dropped %u bytes",
                         (unsigned)((size_t)bytes_read - queued));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/*==================[end of file]============================================*/
