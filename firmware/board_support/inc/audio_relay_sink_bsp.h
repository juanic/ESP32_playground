#ifndef AUDIO_RELAY_SINK_BSP_H
#define AUDIO_RELAY_SINK_BSP_H

/** @defgroup bsp BSP
 *  @brief Board support package layer.
 *  @{
 *  @defgroup audio_relay_sink_bsp Audio Relay Sink BSP
 *  @brief Board support for Bluetooth A2DP sink with I2S output.
 *  @{
 *
 * @section genDesc General Description
 *
 * Board support package for the audio relay sink board.
 * Receives A2DP audio from a phone/PC and outputs PCM via I2S.
 *
 * Pin mapping:
 * | Signal | GPIO |
 * |--------|------|
 * | BCLK   | 26   |
 * | WS     | 25   |
 * | DATA   | 22   |
 *
 * @author Audio Relay Project
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
#include "bt_classic_hal.h"
/*==================[macros]=================================================*/

#define BSP_SINK_I2S_BCLK_PIN   26
#define BSP_SINK_I2S_WS_PIN     25
#define BSP_SINK_I2S_DATA_PIN   22
#define BSP_SINK_I2S_SAMPLE_RATE 44100
#define BSP_SINK_UART_TX_PIN      32
#define BSP_SINK_UART_RX_PIN      33

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialize the audio relay sink board.
 *
 * Sets up I2S TX output and A2DP sink with the configured device name.
 * Incoming A2DP audio is forwarded to @p data_cb (the app decides whether
 * to actually play it).
 *
 * @param device_name  BT device name for this sink (NULL for default).
 * @param data_cb      Callback receiving decoded PCM audio (may be NULL
 *                     to discard audio).
 * @return true if successful.
 */
bool AudioRelaySinkBspInit(const char *device_name, bt_classic_sink_data_cb_t data_cb);

/** @} */
/** @} */

#endif /* #ifndef AUDIO_RELAY_SINK_BSP_H */

/*==================[end of file]============================================*/
