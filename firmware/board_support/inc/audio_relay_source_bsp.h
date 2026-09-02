#ifndef AUDIO_RELAY_SOURCE_BSP_H
#define AUDIO_RELAY_SOURCE_BSP_H

/** @defgroup bsp BSP
 *  @brief Board support package layer.
 *  @{
 *  @defgroup audio_relay_source_bsp Audio Relay Source BSP
 *  @brief Board support for Bluetooth A2DP source with I2S input.
 *  @{
 *
 * @section genDesc General Description
 *
 * Board support package for the audio relay source board.
 * Reads PCM audio via I2S and transmits it as A2DP source.
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

#define BSP_SOURCE_I2S_BCLK_PIN   26
#define BSP_SOURCE_I2S_WS_PIN     25
#define BSP_SOURCE_I2S_DATA_PIN   22
#define BSP_SOURCE_I2S_SAMPLE_RATE 44100

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialize the audio relay source board.
 *
 * Sets up I2S RX input and A2DP source.
 *
 * @param device_name  BT device name for this source (NULL for default).
 * @return true if successful.
 */
bool AudioRelaySourceBspInit(const char *device_name,
                             bt_classic_source_state_cb_t state_cb,
                             bt_classic_source_data_cb_t data_cb);

/** @} */
/** @} */

#endif /* #ifndef AUDIO_RELAY_SOURCE_BSP_H */

/*==================[end of file]============================================*/
