/**
 * @file audio_relay_source_bsp.c
 * @brief Board support for audio relay source (I2S RX → A2DP).
 */

/*==================[inclusions]=============================================*/
#include "audio_relay_source_bsp.h"
#include "i2s_hal.h"
#include "bt_classic_hal.h"
#include "esp_log.h"
/*==================[macros and definitions]=================================*/

#define DEFAULT_DEVICE_NAME "Audio Relay Src"

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
static const char *TAG = "audio_relay_source_bsp";

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

bool AudioRelaySourceBspInit(const char *device_name,
                             bt_classic_source_state_cb_t state_cb,
                             bt_classic_source_data_cb_t data_cb) {
    const char *name = device_name ? device_name : DEFAULT_DEVICE_NAME;

    ESP_LOGI(TAG, "Initializing Audio Relay Source BSP");

    /* Initialize I2S RX */
    i2s_hal_config_t i2s_cfg = {
        .bclk_pin     = BSP_SOURCE_I2S_BCLK_PIN,
        .ws_pin       = BSP_SOURCE_I2S_WS_PIN,
        .data_pin     = BSP_SOURCE_I2S_DATA_PIN,
        .sample_rate  = BSP_SOURCE_I2S_SAMPLE_RATE,
        .bits_per_sample = 16,
        .is_stereo    = true,
        /* The sink drives BCLK and WS across the board-to-board I2S link. */
        .is_master    = false,
    };

    if (!I2sHalInitRx(&i2s_cfg)) {
        ESP_LOGE(TAG, "I2S RX init failed");
        return false;
    }

    /* Initialize BT Classic stack + A2DP Source */
    if (!bt_classic_hal_init(name)) {
        ESP_LOGE(TAG, "BT Classic init failed");
        return false;
    }

    if (!bt_classic_hal_init_a2dp_source(state_cb, data_cb)) {
        ESP_LOGE(TAG, "A2DP Source init failed");
        return false;
    }

    ESP_LOGI(TAG, "Audio Relay Source BSP ready: \"%s\"", name);
    return true;
}

/*==================[end of file]============================================*/
