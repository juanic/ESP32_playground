#ifndef INTER_BOARD_LINK_H
#define INTER_BOARD_LINK_H

#include <stdbool.h>

typedef void (*inter_board_link_line_cb_t)(const char *line, void *ctx);

bool InterBoardLinkInit(int tx_pin, int rx_pin, inter_board_link_line_cb_t line_cb, void *ctx);
bool InterBoardLinkSend(const char *line);

/** Sends one request without blocking. The callback receives the response or ERR TARGET_TIMEOUT. */
bool InterBoardLinkRequest(const char *line, inter_board_link_line_cb_t response_cb, void *ctx);
void InterBoardLinkSetEventCallback(inter_board_link_line_cb_t event_cb, void *ctx);

#endif
