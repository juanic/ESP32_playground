#ifndef CAMERA_STREAM_SERVER_H
#define CAMERA_STREAM_SERVER_H

/** @defgroup middleware Middleware
 *  @brief Reusable middleware services.
 *  @{
 *  @defgroup camera_stream_server Camera stream server
 *  @brief HTTP MJPEG streaming server for camera frames.
 *  @{
 *
 * @section genDesc General Description
 *
 * Starts an HTTP server that exposes:
 *
 * | Endpoint   | Method | Description                                  |
 * |:----------:|:------:|:---------------------------------------------|
 * | `/`        | GET    | HTML viewer                                  |
 * | `/stream`  | GET    | MJPEG stream (`multipart/x-mixed-replace`)   |
 * | `/capture` | GET    | Single JPEG snapshot                         |
 *
 * The frames are obtained through the camera HAL; this module does not touch
 * the sensor or the DVP interface directly.
 *
 * Only one client can use the camera at a time. While a stream is open, any
 * other request to `/stream` or `/capture` is answered with
 * `503 Service Unavailable` instead of blocking the HTTP server.
 *
 * The component is only compiled when `CONFIG_CAMERA_HAL_ENABLED` is set.
 *
 * @section changelog Changelog
 *
 * |   Date     | Description                                            |
 * |:----------:|:-------------------------------------------------------|
 * | 06/11/2026 | Document creation                                      |
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include "esp_err.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Start the HTTP camera server.
 *
 * @param port TCP port to listen on (normally 80).
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t camera_stream_server_start(uint16_t port);

/**
 * @brief Stop the HTTP camera server.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if it is not running.
 */
esp_err_t camera_stream_server_stop(void);

/** @} */
/** @} */

#endif /* #ifndef CAMERA_STREAM_SERVER_H */

/*==================[end of file]============================================*/
