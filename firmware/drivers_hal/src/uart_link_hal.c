#include "uart_link_hal.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>

#define UART_LINK_PORT UART_NUM_1
#define UART_LINK_RX_BUF_SIZE 512

static const char *TAG = "uart_link_hal";
static uart_link_hal_line_cb_t s_line_cb;
static void *s_ctx;
static bool s_initialized;

static void uart_link_rx_task(void *arg) {
    (void)arg;
    uint8_t rx[64];
    char line[UART_LINK_HAL_MAX_LINE_LEN];
    size_t line_len = 0;
    while (true) {
        int len = uart_read_bytes(UART_LINK_PORT, rx, sizeof(rx), pdMS_TO_TICKS(100));
        for (int i = 0; i < len; i++) {
            if (rx[i] == '\n' || rx[i] == '\r') {
                if (line_len > 0) {
                    line[line_len] = '\0';
                    if (s_line_cb) s_line_cb(line, s_ctx);
                    line_len = 0;
                }
            } else if (line_len < sizeof(line) - 1) {
                line[line_len++] = (char)rx[i];
            } else {
                line_len = 0;
                ESP_LOGW(TAG, "Discarding oversized line");
            }
        }
    }
}

bool UartLinkHalInit(const uart_link_hal_config_t *config,
                     uart_link_hal_line_cb_t line_cb, void *ctx) {
    if (s_initialized || !config || !line_cb) return false;
    uart_config_t uart_config = {
        .baud_rate = config->baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    esp_err_t ret = uart_driver_install(UART_LINK_PORT, UART_LINK_RX_BUF_SIZE, 0, 0, NULL, 0);
    if (ret == ESP_OK) ret = uart_param_config(UART_LINK_PORT, &uart_config);
    if (ret == ESP_OK) ret = uart_set_pin(UART_LINK_PORT, config->tx_pin, config->rx_pin,
                                          UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART init failed: %s", esp_err_to_name(ret));
        uart_driver_delete(UART_LINK_PORT);
        return false;
    }
    s_line_cb = line_cb;
    s_ctx = ctx;
    s_initialized = xTaskCreate(uart_link_rx_task, "uart_link_rx", 3072, NULL, 5, NULL) == pdPASS;
    ESP_LOGI(TAG, "UART link ready TX=%d RX=%d baud=%d", config->tx_pin, config->rx_pin, config->baud_rate);
    return s_initialized;
}

bool UartLinkHalSendLine(const char *line) {
    if (!s_initialized || !line) return false;
    size_t len = strlen(line);
    return uart_write_bytes(UART_LINK_PORT, line, len) == (int)len &&
           uart_write_bytes(UART_LINK_PORT, "\n", 1) == 1;
}
