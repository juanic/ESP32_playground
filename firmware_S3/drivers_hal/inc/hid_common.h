#ifndef HID_COMMON_H
#define HID_COMMON_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup hid_common HID Common
 *  @brief Types and helpers shared by the BLE and USB HID transports.
 *  @{
 *
 * @section genDesc General Description
 *
 * Both HID transports (BLE/HOGP via @c esp_hid and USB via TinyUSB) expose the
 * same media-key abstraction, so the @ref media_key_t type and the mapping to
 * the 16-bit USB HID Consumer-page usage code live here to avoid duplicating
 * them in each driver.
 *
 * @author Juan Cerrudo
 *
 * @section changelog
 *
 * |   Date	    | Description                                    						|
 * |:----------:|:----------------------------------------------------------------------|
 * | 28/08/2026 | Document creation		                         						|
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdint.h>

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief Consumer Control (media) keys supported by the HID drivers.
 *
 * Each value maps to its USB HID Consumer-page usage code (16-bit) through
 * @ref hid_media_key_to_usage, so the host interprets it as a standard media
 * key regardless of the transport (BLE or USB).
 */
typedef enum {
	MEDIA_KEY_PLAY_PAUSE,	/**< Play / Pause */
	MEDIA_KEY_NEXT,			/**< Next track */
	MEDIA_KEY_PREV,			/**< Previous track */
	MEDIA_KEY_VOL_UP,		/**< Volume up */
	MEDIA_KEY_VOL_DOWN,		/**< Volume down */
	MEDIA_KEY_STOP			/**< Stop */
} media_key_t;

/*==================[internal functions definition]=========================*/

/**
 * @brief Map a @ref media_key_t to its 16-bit USB HID Consumer-page usage code.
 *
 * @param key media key to translate.
 * @return 16-bit Consumer-page usage code (0 if the key is unknown).
 */
static inline uint16_t hid_media_key_to_usage(media_key_t key)
{
	switch (key) {
	case MEDIA_KEY_PLAY_PAUSE: return 0x00CD; /* HID_USAGE_CONSUMER_PLAY_PAUSE */
	case MEDIA_KEY_NEXT:       return 0x00B5; /* HID_USAGE_CONSUMER_SCAN_NEXT_TRACK */
	case MEDIA_KEY_PREV:       return 0x00B6; /* HID_USAGE_CONSUMER_SCAN_PREVIOUS_TRACK */
	case MEDIA_KEY_VOL_UP:     return 0x00E9; /* HID_USAGE_CONSUMER_VOLUME_INCREMENT */
	case MEDIA_KEY_VOL_DOWN:   return 0x00EA; /* HID_USAGE_CONSUMER_VOLUME_DECREMENT */
	case MEDIA_KEY_STOP:       return 0x00B7; /* HID_USAGE_CONSUMER_STOP */
	default:                   return 0x0000;
	}
}

/** @} */
/** @} */

#endif /* #ifndef HID_COMMON_H */

/*==================[end of file]============================================*/
