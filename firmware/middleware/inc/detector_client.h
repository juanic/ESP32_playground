#ifndef DETECTOR_CLIENT_H
#define DETECTOR_CLIENT_H

/** @defgroup middleware Middleware
 *  @brief Reusable middleware services.
 *  @{
 *  @defgroup detector_client Detector client
 *  @brief HTTP client for the external people detection service.
 *  @{
 *
 * @section genDesc General Description
 *
 * Uploads a JPEG capture to a detection server (normally a PC on the same
 * LAN) and parses its response. The wire protocol is deliberately simple so
 * the firmware does not need a JSON parser:
 *
 * @code
 * POST /detect
 * Content-Type: image/jpeg
 * X-Threshold: 0.50
 *
 * <JPEG bytes>
 * @endcode
 *
 * The server answers with one line per detection:
 *
 * @code
 * count=2
 * box=0.12,0.30,0.08,0.42,0.91
 * box=0.55,0.28,0.10,0.45,0.87
 * @endcode
 *
 * Box coordinates are normalised (0..1) as `x,y,width,height,confidence`, so
 * the browser can scale them to any image size.
 *
 * The component is only compiled when `CONFIG_CAMERA_HAL_ENABLED` is set.
 *
 * @section changelog Changelog
 *
 * |   Date     | Description                                            |
 * |:----------:|:-------------------------------------------------------|
 * | 09/11/2026 | Document creation                                      |
 *
 **/

/*==================[inclusions]=============================================*/
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/**
 * @brief Detector response summary.
 */
typedef struct {
	uint32_t count;				/**< Number of detections above the threshold */
	float confidence_avg;		/**< Mean confidence, 0..1 */
	float confidence_min;		/**< Lowest confidence, 0..1 */
	float confidence_max;		/**< Highest confidence, 0..1 */
} detector_client_summary_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Send a JPEG to the detection server.
 *
 * @param url        Full endpoint URL, e.g. "http://192.168.0.50:8080/detect".
 * @param jpeg       JPEG bytes.
 * @param jpeg_len   JPEG length.
 * @param threshold  Confidence threshold in 0..1.
 * @param json_out   Buffer that receives the detections as compact JSON.
 * @param json_len   Size of @p json_out.
 * @param summary    Optional output summary used for the CSV row.
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG on bad arguments,
 *         ESP_ERR_INVALID_RESPONSE if the server answered with an error.
 */
esp_err_t detector_client_detect(const char *url, const uint8_t *jpeg,
								 size_t jpeg_len, float threshold,
								 char *json_out, size_t json_len,
								 detector_client_summary_t *summary);

/** @} */
/** @} */

#endif /* #ifndef DETECTOR_CLIENT_H */

/*==================[end of file]============================================*/
