#ifndef TOUCH_HAL_H
#define TOUCH_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup touch_hal Touch HAL
 *  @brief Touch pad driver for ESP32-S3.
 *  @{
 * 
 * @section genDesc General Description
 *
 * This driver provides functions to configure and read the ESP32-S3 capacitive
 * touch sensor channels (T0-T13). It wraps the ESP-IDF `esp_driver_touch_sens`
 * component (touch sensor hardware version 2, used by the ESP32-S3).
 *
 * On the ESP32-S3 the channel value INCREASES when the pad is touched, so a
 * channel is considered "touched" when its (smooth - benchmark) value is ABOVE
 * the configured relative threshold.
 *
 * | Touch Channel |  ESP32-S3 GPIO  |
 * |:-------------:|:---------------:|
 * |   TOUCH_PAD_0 |    GPIO_1       |
 * |   TOUCH_PAD_1 |    GPIO_2       |
 * |   TOUCH_PAD_2 |    GPIO_3       |
 * |   TOUCH_PAD_3 |    GPIO_4       |
 * |   TOUCH_PAD_4 |    GPIO_5       |
 * |   TOUCH_PAD_5 |    GPIO_6       |
 * |   TOUCH_PAD_6 |    GPIO_7       |
 * |   TOUCH_PAD_7 |    GPIO_8       |
 * |   TOUCH_PAD_8 |    GPIO_9       |
 * |   TOUCH_PAD_9 |    GPIO_10      |
 * |   TOUCH_PAD_10|    GPIO_11      |
 * |   TOUCH_PAD_11|    GPIO_12      |
 * |   TOUCH_PAD_12|    GPIO_13      |
 * |   TOUCH_PAD_13|    GPIO_14      |
 *
 * @note TOUCH_PAD_0, TOUCH_PAD_2 and TOUCH_PAD_3 share pins with strapping
 * pins (GPIO_1, GPIO_3, GPIO_4). Avoid using them if the board relies on
 * those pins during boot.
 *
 * @author Juan Cerrudo
 *
 * @section changelog
 *
 * |   Date	    | Description                                    						|
 * |:----------:|:----------------------------------------------------------------------|
 * | 24/08/2026 | Document creation		                         						|
 * | 28/08/2026 | Ported from ESP32 (touch v1) to ESP32-S3 (touch v2)			|
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
#include "driver/touch_sens.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief ESP32-S3 available touch pad channels.
 *
 */
typedef enum {
	TOUCH_PAD_0 = 0,	/**< GPIO_1 */
	TOUCH_PAD_1,		/**< GPIO_2 */
	TOUCH_PAD_2,		/**< GPIO_3 */
	TOUCH_PAD_3,		/**< GPIO_4 */
	TOUCH_PAD_4,		/**< GPIO_5 */
	TOUCH_PAD_5,		/**< GPIO_6 */
	TOUCH_PAD_6,		/**< GPIO_7 */
	TOUCH_PAD_7,		/**< GPIO_8 */
	TOUCH_PAD_8,		/**< GPIO_9 */
	TOUCH_PAD_9,		/**< GPIO_10 */
	TOUCH_PAD_10,		/**< GPIO_11 */
	TOUCH_PAD_11,		/**< GPIO_12 */
	TOUCH_PAD_12,		/**< GPIO_13 */
	TOUCH_PAD_13,		/**< GPIO_14 */
} touch_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Touch sensor controller initialization.
 *
 * Must be called once, before configuring any channel.
 *
 * @return true on success, false otherwise.
 */
bool TouchHalInit(void);

/**
 * @brief Configure a touch pad channel.
 *
 * Must be called after TouchHalInit() and before TouchHalStart().
 *
 * @param pad Touch channel to configure.
 * @param threshold Relative threshold (smooth - benchmark) above which the
 * channel is considered touched. A typical starting point is ~200, adjust
 * empirically for your setup.
 * @return true on success, false otherwise.
 */
bool TouchHalChannelConfig(touch_t pad, uint32_t threshold);

/**
 * @brief Enable the touch sensor controller and start continuously scanning
 * all the configured channels.
 *
 * Must be called after configuring the desired channels with
 * TouchHalChannelConfig().
 *
 * @return true on success, false otherwise.
 */
bool TouchHalStart(void);

/**
 * @brief Read the filtered (smoothed) value of a touch channel.
 *
 * @param pad Touch channel to read.
 * @param value Pointer to store the read value.
 * @return true on success, false otherwise.
 */
bool TouchHalRead(touch_t pad, uint32_t *value);

/**
 * @brief Check whether a touch channel is currently being touched.
 *
 * On the ESP32-S3 the channel value increases when touched, so a channel is
 * considered touched when its value is above its configured threshold.
 *
 * @param pad Touch channel to check.
 * @return true if the channel value is above its configured threshold.
 */
bool TouchHalIsTouched(touch_t pad);

/**
 * @brief Stop scanning, disable and release all the touch sensor resources.
 *
 */
void TouchHalDeinit(void);

/** @} */
/** @} */

#endif /* #ifndef TOUCH_HAL_H */

/*==================[end of file]============================================*/
