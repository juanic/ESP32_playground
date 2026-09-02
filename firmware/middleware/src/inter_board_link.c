#include "inter_board_link.h"
#include "uart_link_hal.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <string.h>

#define INTER_BOARD_BAUD_RATE 115200
#define INTER_BOARD_TIMEOUT_US 2000000

static const char *TAG = "inter_board_link";
static inter_board_link_line_cb_t s_rx_cb;
static void *s_rx_ctx;
static inter_board_link_line_cb_t s_evt_cb;
static void *s_evt_ctx;
static inter_board_link_line_cb_t s_response_cb;
static void *s_response_ctx;
static esp_timer_handle_t s_timeout_timer;

static void on_timeout(void *arg) {
    (void)arg;
    if (s_response_cb) {
        inter_board_link_line_cb_t cb = s_response_cb;
        void *ctx = s_response_ctx;
        s_response_cb = NULL;
        cb("ERR TARGET_TIMEOUT", ctx);
    }
}

static void on_uart_line(const char *line, void *ctx) {
    (void)ctx;
    if (strncmp(line, "SPEAKER_", 8) == 0) {
        if (s_evt_cb) s_evt_cb(line, s_evt_ctx);
        return;
    }
    if (s_response_cb) {
        inter_board_link_line_cb_t cb = s_response_cb;
        void *cb_ctx = s_response_ctx;
        s_response_cb = NULL;
        esp_timer_stop(s_timeout_timer);
        cb(line, cb_ctx);
    } else if (s_rx_cb) {
        s_rx_cb(line, s_rx_ctx);
    }
}

bool InterBoardLinkInit(int tx_pin, int rx_pin, inter_board_link_line_cb_t line_cb, void *ctx) {
    uart_link_hal_config_t config = {.tx_pin = tx_pin, .rx_pin = rx_pin, .baud_rate = INTER_BOARD_BAUD_RATE};
    if (!UartLinkHalInit(&config, on_uart_line, NULL)) return false;
    s_rx_cb = line_cb;
    s_rx_ctx = ctx;
    esp_timer_create_args_t timer_args = {.callback = on_timeout, .name = "ibl_timeout"};
    if (esp_timer_create(&timer_args, &s_timeout_timer) != ESP_OK) return false;
    ESP_LOGI(TAG, "Inter-board link initialized");
    return true;
}

bool InterBoardLinkSend(const char *line) { return UartLinkHalSendLine(line); }

bool InterBoardLinkRequest(const char *line, inter_board_link_line_cb_t response_cb, void *ctx) {
    if (!line || !response_cb || s_response_cb) return false;
    s_response_cb = response_cb;
    s_response_ctx = ctx;
    if (!UartLinkHalSendLine(line) || esp_timer_start_once(s_timeout_timer, INTER_BOARD_TIMEOUT_US) != ESP_OK) {
        s_response_cb = NULL;
        return false;
    }
    return true;
}

void InterBoardLinkSetEventCallback(inter_board_link_line_cb_t event_cb, void *ctx) {
    s_evt_cb = event_cb;
    s_evt_ctx = ctx;
}
