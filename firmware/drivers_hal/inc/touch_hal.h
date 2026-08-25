#ifndef TOUCH_HAL_H
#define TOUCH_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup touch_hal Touch HAL
 *  @brief Touch pad driver for ESP32.
 *  @{
 *
 * @section genDesc General Description
 *
 * This driver provides functions to configure and read the ESP32 capacitive
 * touch sensor channels (T0-T9). It wraps the ESP-IDF `esp_driver_touch_sens`
 * component (touch sensor hardware version 1, used by the classic ESP32).
 *
 * | Touch Channel |  ESP32 GPIO  |
 * |:-------------:|:------------:|
 * |   TOUCH_PAD_0 |    GPIO_4    |
 * |   TOUCH_PAD_1 |    GPIO_0    |
 * |   TOUCH_PAD_2 |    GPIO_2    |
 * |   TOUCH_PAD_3 |    GPIO_15   |
 * |   TOUCH_PAD_4 |    GPIO_13   |
 * |   TOUCH_PAD_5 |    GPIO_12   |
 * |   TOUCH_PAD_6 |    GPIO_14   |
 * |   TOUCH_PAD_7 |    GPIO_27   |
 * |   TOUCH_PAD_8 |    GPIO_33   |
 * |   TOUCH_PAD_9 |    GPIO_32   |
 *
 * @note TOUCH_PAD_1, TOUCH_PAD_2, TOUCH_PAD_3 and TOUCH_PAD_5 share pins with
 * strapping pins (GPIO_0, GPIO_2, GPIO_15, GPIO_12). Avoid using them if the
 * board relies on those pins during boot.
 *
 * @author Albano Peñalva
 *
 * @section changelog
 *
 * |   Date	    | Description                                    						|
 * |:----------:|:----------------------------------------------------------------------|
 * | 24/08/2026 | Document creation		                         						|
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief ESP32 available touch pad channels.
 *
 */
typedef enum {
	TOUCH_PAD_0 = 0,	/**< GPIO_4 */
	TOUCH_PAD_1,		/**< GPIO_0 */
	TOUCH_PAD_2,		/**< GPIO_2 */
	TOUCH_PAD_3,		/**< GPIO_15 */
	TOUCH_PAD_4,		/**< GPIO_13 */
	TOUCH_PAD_5,		/**< GPIO_12 */
	TOUCH_PAD_6,		/**< GPIO_14 */
	TOUCH_PAD_7,		/**< GPIO_27 */
	TOUCH_PAD_8,		/**< GPIO_33 */
	TOUCH_PAD_9,		/**< GPIO_32 */
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
 * @param threshold Absolute value below which the channel is considered
 * touched (lower raw/smooth values mean a bigger capacitance change).
 * A typical starting point is 2/3 of the untouched reading, adjust
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
 * @param pad Touch channel to check.
 * @return true if the channel value is below its configured threshold.
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
