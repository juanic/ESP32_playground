/**
 * @file audio_relay_sink_bsp.c
 * @brief Board support for audio relay sink (A2DP → I2S TX).
 */

/*==================[inclusions]=============================================*/
#include "audio_relay_sink_bsp.h"
#include "i2s_hal.h"
#include "bt_classic_hal.h"
#include "esp_log.h"
/*==================[macros and definitions]=================================*/

#define DEFAULT_DEVICE_NAME "Audio Relay Sink"

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
static const char *TAG = "audio_relay_sink_bsp";

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static void default_on_sink_data(const uint8_t *data, uint32_t len) {
    I2sHalWrite(data, len, 1000);
}

/*==================[external functions definition]==========================*/

bool AudioRelaySinkBspInit(const char *device_name, bt_classic_sink_data_cb_t data_cb) {
    const char *name = device_name ? device_name : DEFAULT_DEVICE_NAME;

    ESP_LOGI(TAG, "Initializing Audio Relay Sink BSP");

    /* Initialize I2S TX */
    i2s_hal_config_t i2s_cfg = {
        .bclk_pin     = BSP_SINK_I2S_BCLK_PIN,
        .ws_pin       = BSP_SINK_I2S_WS_PIN,
        .data_pin     = BSP_SINK_I2S_DATA_PIN,
        .sample_rate  = BSP_SINK_I2S_SAMPLE_RATE,
        .bits_per_sample = 16,
        .is_stereo    = true,
        .is_master    = true,
    };

    if (!I2sHalInitTx(&i2s_cfg)) {
        ESP_LOGE(TAG, "I2S TX init failed");
        return false;
    }

    /* Initialize BT Classic stack + A2DP Sink */
    if (!bt_classic_hal_init(name)) {
        ESP_LOGE(TAG, "BT Classic init failed");
        return false;
    }

    bt_classic_sink_data_cb_t sink_cb = data_cb ? data_cb : default_on_sink_data;
    if (!bt_classic_hal_init_a2dp_sink(sink_cb)) {
        ESP_LOGE(TAG, "A2DP Sink init failed");
        return false;
    }

    ESP_LOGI(TAG, "Audio Relay Sink BSP ready: \"%s\"", name);
    return true;
}

/*==================[end of file]============================================*/
