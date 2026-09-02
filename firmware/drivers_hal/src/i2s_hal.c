/**
 * @file i2s_hal.c
 * @brief I2S HAL driver for ESP-IDF 6.x (channel-based API).
 */

/*==================[inclusions]=============================================*/
#include "i2s_hal.h"
#include "driver/i2s_std.h"
#include "esp_log.h"
#include <string.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
static const char *TAG = "i2s_hal";

static i2s_chan_handle_t s_tx_handle = NULL;
static i2s_chan_handle_t s_rx_handle = NULL;
static bool s_initialized = false;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static bool i2s_hal_init_common(const i2s_hal_config_t *config,
                                i2s_chan_handle_t *tx_handle,
                                i2s_chan_handle_t *rx_handle) {
    if (s_initialized) {
        ESP_LOGW(TAG, "I2S already initialized");
        return false;
    }
    if (!config) {
        ESP_LOGE(TAG, "config is NULL");
        return false;
    }

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(
        I2S_NUM_AUTO, config->is_master ? I2S_ROLE_MASTER : I2S_ROLE_SLAVE);
    chan_cfg.auto_clear_after_cb = true;

    esp_err_t ret = i2s_new_channel(&chan_cfg, tx_handle, rx_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_new_channel failed: %s", esp_err_to_name(ret));
        return false;
    }

    i2s_data_bit_width_t bit_width = (config->bits_per_sample == 32)
                                     ? I2S_DATA_BIT_WIDTH_32BIT
                                     : I2S_DATA_BIT_WIDTH_16BIT;
    i2s_slot_mode_t slot_mode = config->is_stereo
                                ? I2S_SLOT_MODE_STEREO
                                : I2S_SLOT_MODE_MONO;

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(config->sample_rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(bit_width, slot_mode),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = config->bclk_pin,
            .ws   = config->ws_pin,
            .dout = (tx_handle && *tx_handle) ? config->data_pin : I2S_GPIO_UNUSED,
            .din  = (rx_handle && *rx_handle) ? config->data_pin : I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    i2s_chan_handle_t init_handle = (tx_handle && *tx_handle) ? *tx_handle
                                    : ((rx_handle) ? *rx_handle : NULL);
    ret = i2s_channel_init_std_mode(init_handle, &std_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_channel_init_std_mode failed: %s", esp_err_to_name(ret));
        i2s_del_channel(init_handle);
        *tx_handle = NULL;
        *rx_handle = NULL;
        return false;
    }

    ret = i2s_channel_enable(init_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_channel_enable failed: %s", esp_err_to_name(ret));
        i2s_del_channel(init_handle);
        *tx_handle = NULL;
        *rx_handle = NULL;
        return false;
    }

    s_initialized = true;
    ESP_LOGI(TAG, "I2S initialized: rate=%lu bits=%d %s",
             config->sample_rate, config->bits_per_sample,
             config->is_stereo ? "stereo" : "mono");
    return true;
}

/*==================[external functions definition]==========================*/

bool I2sHalInitTx(const i2s_hal_config_t *config) {
    return i2s_hal_init_common(config, &s_tx_handle, NULL);
}

bool I2sHalInitRx(const i2s_hal_config_t *config) {
    return i2s_hal_init_common(config, NULL, &s_rx_handle);
}

int I2sHalWrite(const uint8_t *buf, size_t len, uint32_t timeout_ms) {
    if (!s_initialized || !s_tx_handle || !buf) {
        return -1;
    }

    size_t bytes_written = 0;
    esp_err_t ret = i2s_channel_write(s_tx_handle, buf, len,
                                      &bytes_written, timeout_ms);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_channel_write failed: %s", esp_err_to_name(ret));
        return -1;
    }
    return (int)bytes_written;
}

int I2sHalRead(uint8_t *buf, size_t len, uint32_t timeout_ms) {
    if (!s_initialized || !s_rx_handle || !buf) {
        return -1;
    }

    size_t bytes_read = 0;
    esp_err_t ret = i2s_channel_read(s_rx_handle, buf, len,
                                     &bytes_read, timeout_ms);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_channel_read failed: %s", esp_err_to_name(ret));
        return -1;
    }
    return (int)bytes_read;
}

void I2sHalDeinit(void) {
    if (!s_initialized) {
        return;
    }

    if (s_tx_handle) {
        i2s_channel_disable(s_tx_handle);
        i2s_del_channel(s_tx_handle);
        s_tx_handle = NULL;
    }
    if (s_rx_handle) {
        i2s_channel_disable(s_rx_handle);
        i2s_del_channel(s_rx_handle);
        s_rx_handle = NULL;
    }

    s_initialized = false;
    ESP_LOGI(TAG, "I2S deinitialized");
}

/*==================[end of file]============================================*/
