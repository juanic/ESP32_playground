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
#include "inter_board_link.h"
#include "nvs_hal.h"

/*==================[macros and definitions]=================================*/

#define DEVICE_NAME         "Audio Relay Src"
#define I2S_READ_BUF_SIZE   1024
#define I2S_READ_TIMEOUT_MS 100
#define RECONNECT_DELAY_MS  3000
#define PCM_BUFFER_SIZE     (32 * 1024)
#define TARGET_MAX_LEN      32
#define NVS_NAMESPACE       "relay"
#define NVS_KEY_TARGET      "target"

/* FAST_TEST: hardcodes the target speaker for I2S wiring bench tests,
 * bypassing NVS/SET_TARGET. Remove/comment out once wiring is validated. */
#define FAST_TEST 0
#if FAST_TEST
#define FAST_TEST_TARGET_NAME "BT-WUZHI"
#endif

/*==================[internal data definition]===============================*/

static const char *TAG = "audio_relay_source";
static bool s_source_connected = false;
static StaticStreamBuffer_t s_pcm_buffer_storage;
static uint8_t s_pcm_buffer_memory[PCM_BUFFER_SIZE];
static StreamBufferHandle_t s_pcm_buffer;
static char s_target[TARGET_MAX_LEN];

/*==================[internal functions declaration]=========================*/

static void on_source_state(bool connected);
static int32_t on_source_data(uint8_t *data, int32_t len);
static void on_speaker_found(const char *name, const uint8_t *bda);
static void on_link_line(const char *line, void *ctx);
static bool parse_mac(const char *text, uint8_t *bda);
static void format_mac(const uint8_t *bda, char *out);

/*==================[external functions definition]==========================*/

static void on_source_state(bool connected) {
    s_source_connected = connected;
    uint8_t bda[6];
    char mac[18] = "-";
    if (connected && bt_classic_hal_get_peer_bda(bda)) format_mac(bda, mac);
    char line[96];
    snprintf(line, sizeof(line), "%s name=%s mac=%s",
             connected ? "SPEAKER_CONNECTED" : "SPEAKER_DISCONNECTED",
             parse_mac(s_target, bda) ? "-" : (s_target[0] ? s_target : "-"), mac);
    InterBoardLinkSend(line);
    if (connected) {
        ESP_LOGI(TAG, "A2DP source connected to amplifier");
    } else {
        ESP_LOGW(TAG, "A2DP source disconnected, will reconnect...");
    }
}

static bool parse_mac(const char *text, uint8_t *bda) {
    unsigned int value[6];
    if (!text || sscanf(text, "%2x:%2x:%2x:%2x:%2x:%2x", &value[0], &value[1],
                        &value[2], &value[3], &value[4], &value[5]) != 6) return false;
    for (int i = 0; i < 6; i++) bda[i] = (uint8_t)value[i];
    return true;
}

static void format_mac(const uint8_t *bda, char *out) {
    sprintf(out, "%02x:%02x:%02x:%02x:%02x:%02x", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
}

static void connect_configured_target(void) {
    uint8_t bda[6];
    if (!s_target[0]) return;
    if (parse_mac(s_target, bda)) {
        bt_classic_hal_source_connect(bda);
    } else {
        bt_classic_hal_source_start_discovery(s_target);
    }
}

static void on_speaker_found(const char *name, const uint8_t *bda) {
    char mac[18];
    format_mac(bda, mac);
    char line[96];
    snprintf(line, sizeof(line), "SPEAKER_FOUND name=%s mac=%s", name, mac);
    InterBoardLinkSend(line);
}

static void send_target(void) {
    uint8_t bda[6];
    char line[96];
    if (parse_mac(s_target, bda)) {
        snprintf(line, sizeof(line), "TARGET name=- mac=%s connected=%d", s_target,
                 s_source_connected ? 1 : 0);
    } else {
        snprintf(line, sizeof(line), "TARGET name=%s mac=- connected=%d",
                 s_target[0] ? s_target : "-", s_source_connected ? 1 : 0);
    }
    InterBoardLinkSend(line);
}

static void on_link_line(const char *line, void *ctx) {
    (void)ctx;
    if (strcmp(line, "SCAN_TARGET") == 0) {
        InterBoardLinkSend(bt_classic_hal_source_start_scan(on_speaker_found) ? "OK" : "ERR BUSY");
    } else if (strncmp(line, "SET_TARGET ", 11) == 0 && line[11]) {
        snprintf(s_target, sizeof(s_target), "%s", line + 11);
        if (!NvsHalSetStr(NVS_NAMESPACE, NVS_KEY_TARGET, s_target)) {
            InterBoardLinkSend("ERR TARGET");
            return;
        }
        uint8_t bda[6];
        char response[96];
        snprintf(response, sizeof(response), "TARGET OK name=%s mac=%s",
                 parse_mac(s_target, bda) ? "-" : s_target,
                 parse_mac(s_target, bda) ? s_target : "-");
        InterBoardLinkSend(response);
        if (s_source_connected) bt_classic_hal_disconnect();
        else connect_configured_target();
    } else if (strcmp(line, "GET_TARGET") == 0) {
        send_target();
    } else {
        InterBoardLinkSend("ERR UNKNOWN");
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

    if (!NvsHalInit()) {
        ESP_LOGE(TAG, "NVS init failed");
        return;
    }
    NvsHalGetStr(NVS_NAMESPACE, NVS_KEY_TARGET, s_target, sizeof(s_target));

#if FAST_TEST
    snprintf(s_target, sizeof(s_target), "%s", FAST_TEST_TARGET_NAME);
    ESP_LOGW(TAG, "FAST_TEST enabled: hardcoded target speaker \"%s\"", s_target);
#endif

    if (!InterBoardLinkInit(BSP_SOURCE_UART_TX_PIN, BSP_SOURCE_UART_RX_PIN, on_link_line, NULL)) {
        ESP_LOGE(TAG, "Inter-board UART init failed");
        return;
    }

    /* Initialize BSP (I2S RX + A2DP Source) */
    if (!AudioRelaySourceBspInit(DEVICE_NAME, on_source_state, on_source_data)) {
        ESP_LOGE(TAG, "BSP init failed!");
        return;
    }

    if (s_target[0]) {
        ESP_LOGI(TAG, "Connecting to saved target: %s", s_target);
        connect_configured_target();
    } else {
        ESP_LOGI(TAG, "No target configured; waiting for SET_TARGET");
    }

    /* I2S read buffer */
    uint8_t i2s_buf[I2S_READ_BUF_SIZE];

    /* Main loop: read I2S → forward to A2DP source */
    while (true) {
        if (!s_source_connected) {
            /* Wait for connection, retry discovery periodically */
            vTaskDelay(pdMS_TO_TICKS(RECONNECT_DELAY_MS));
            if (!bt_classic_hal_is_connected() && s_target[0]) {
                ESP_LOGI(TAG, "Retrying target connection...");
                connect_configured_target();
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
