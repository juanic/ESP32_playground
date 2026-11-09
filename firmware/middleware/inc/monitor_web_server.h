#ifndef MONITOR_WEB_SERVER_H
#define MONITOR_WEB_SERVER_H

/** @defgroup middleware Middleware
 *  @brief Reusable middleware services.
 *  @{
 *  @defgroup monitor_web_server Monitor web server
 *  @brief Web UI and REST API for the people counter.
 *  @{
 *
 * @section genDesc General Description
 *
 * Serves a single-page application with:
 *
 * | Endpoint              | Method | Description                              |
 * |:---------------------:|:------:|:-----------------------------------------|
 * | `/`                   | GET    | Viewer with the last photo and controls  |
 * | `/api/status`         | GET    | Wi-Fi, SD, sensor and clock status       |
 * | `/api/latest`         | GET    | Last photo URL and its detections        |
 * | `/api/capture`        | POST   | Request an immediate capture             |
 * | `/api/settings`       | POST   | Update image controls and threshold      |
 * | `/api/days`           | GET    | Days stored on the SD card               |
 * | `/api/files?day=...`  | GET    | Files of one day                         |
 * | `/files/...`          | GET    | Download a stored file (`?dl=1`)         |
 *
 * There is no live streaming: the browser only reloads the last capture.
 *
 * @section changelog Changelog
 *
 * |   Date     | Description                                            |
 * |:----------:|:-------------------------------------------------------|
 * | 09/11/2026 | Document creation                                      |
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include "camera_hal.h"
#include "esp_err.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/**
 * @brief Web server configuration.
 *
 * The image settings and the threshold are owned by the application; the
 * server reads and updates them in place when the user changes a control.
 */
typedef struct {
	uint16_t port;						/**< TCP port, normally 80 */
	camera_hal_image_settings_t *image;	/**< Image controls (may be NULL) */
	uint8_t *threshold_pct;				/**< Detection threshold in percent */
	void (*request_capture)(void);		/**< Callback for "capture now" (may be NULL) */
} monitor_web_server_config_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Start the monitoring web server.
 *
 * @param config Server configuration. The pointed-to settings must outlive
 *               the server.
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if already started.
 */
esp_err_t monitor_web_server_start(const monitor_web_server_config_t *config);

/**
 * @brief Stop the monitoring web server.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if not running.
 */
esp_err_t monitor_web_server_stop(void);

/** @} */
/** @} */

#endif /* #ifndef MONITOR_WEB_SERVER_H */

/*==================[end of file]============================================*/
