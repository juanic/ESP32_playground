#ifndef UART_LINK_HAL_H
#define UART_LINK_HAL_H

#include <stdbool.h>
#include <stdint.h>

#define UART_LINK_HAL_MAX_LINE_LEN 160

typedef void (*uart_link_hal_line_cb_t)(const char *line, void *ctx);

typedef struct {
    int tx_pin;
    int rx_pin;
    int baud_rate;
} uart_link_hal_config_t;

bool UartLinkHalInit(const uart_link_hal_config_t *config,
                     uart_link_hal_line_cb_t line_cb, void *ctx);
bool UartLinkHalSendLine(const char *line);

#endif
