/**
 * @file main.c
 * @brief Audio Relay Sink — receives A2DP audio and outputs via I2S.
 *
 * This app acts as a Bluetooth A2DP sink. A phone/PC connects to it,
 * streams audio, and the PCM data is forwarded to I2S output.
 * Access control (whitelist/blacklist) is applied on connection.
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "audio_relay_sink_bsp.h"
#include "bt_access_control.h"
#include "bt_classic_hal.h"
#include "relay_control.h"
#include "i2s_hal.h"
#include "inter_board_link.h"

/*==================[macros and definitions]=================================*/

#define DEVICE_NAME         "Audio Relay Sink"
#define ACL_CHECK_INTERVAL  5000

/*==================[internal data definition]===============================*/

static const char *TAG = "audio_relay_sink";

/*==================[internal functions declaration]=========================*/

/*==================[internal functions definition]==========================*/

/* Audio data path: honor TX enable + access control + time grant. */
static void on_audio_data(const uint8_t *data, uint32_t len) {
    uint8_t peer_bda[6];
    if (!RelayControlIsTxEnabled()) {
        return;
    }
    if (!bt_classic_hal_get_peer_bda(peer_bda) ||
        !RelayControlIsAllowedToPlay(peer_bda)) {
        return;
    }
    I2sHalWrite(data, len, 1000);
}

/*==================[external functions definition]==========================*/

void app_main(void) {
    ESP_LOGI(TAG, "Audio Relay Sink starting...");

    /* Initialize access control */
    AccessControlInit();

    /* Initialize BSP (I2S TX + A2DP Sink) */
    if (!AudioRelaySinkBspInit(DEVICE_NAME, on_audio_data)) {
        ESP_LOGE(TAG, "BSP init failed!");
        return;
    }

    /* Initialize control-plane (BLE control + AVRCP metadata + password).
     * Must run after A2DP init because AVRCP CT depends on A2DP. */
    if (!InterBoardLinkInit(BSP_SINK_UART_TX_PIN, BSP_SINK_UART_RX_PIN, NULL, NULL)) {
        ESP_LOGE(TAG, "Inter-board UART init failed!");
        return;
    }
    RelayControlInit();

    ESP_LOGI(TAG, "Sink ready. Waiting for A2DP connection...");

    /* Main loop: runtime enforcement of access control + time grants */
    while (true) {
        if (bt_classic_hal_is_connected()) {
            uint8_t peer_bda[6];
            if (bt_classic_hal_get_peer_bda(peer_bda)) {
                if (!RelayControlIsAllowedToPlay(peer_bda)) {
                    ESP_LOGW(TAG, "Connected device not allowed, disconnecting...");
                    bt_classic_hal_disconnect();
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(ACL_CHECK_INTERVAL));
    }
}

/*==================[end of file]============================================*/
