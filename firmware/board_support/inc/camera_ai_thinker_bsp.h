#ifndef CAMERA_AI_THINKER_BSP_H
#define CAMERA_AI_THINKER_BSP_H

/** @defgroup bsp BSP
 *  @brief Board support package layer.
 *  @{
 *  @defgroup camera_ai_thinker_bsp AI-Thinker ESP32-CAM BSP
 *  @brief Camera module support for the AI-Thinker ESP32-CAM board.
 *  @{
 *
 * @section genDesc General Description
 *
 * Provides the pinout and default acquisition settings of the AI-Thinker
 * ESP32-CAM module (OV2640 sensor). The board uses a parallel DVP interface
 * and an SCCB (I2C-like) control bus.
 *
 * @section Hardware_connection Hardware connection:
 *
 * |    Camera signal   |    ESP32 GPIO     |
 * |:------------------:|:-----------------:|
 * |       PWDN         |      GPIO32       |
 * |       RESET        |      NC           |
 * |       XCLK         |      GPIO0        |
 * |       SIOD         |      GPIO26       |
 * |       SIOC         |      GPIO27       |
 * |       D7           |      GPIO35       |
 * |       D6           |      GPIO34       |
 * |       D5           |      GPIO39       |
 * |       D4           |      GPIO36       |
 * |       D3           |      GPIO21       |
 * |       D2           |      GPIO19       |
 * |       D1           |      GPIO18       |
 * |       D0           |      GPIO5        |
 * |       VSYNC        |      GPIO25       |
 * |       HREF         |      GPIO23       |
 * |       PCLK         |      GPIO22       |
 *
 * @note The module must be powered from 5 V. The 3.3 V pin of the FTDI
 * adapter is not enough for the camera and produces brownouts.
 *
 * @section changelog Changelog
 *
 * |   Date     | Description                                            |
 * |:----------:|:-------------------------------------------------------|
 * | 06/11/2026 | Document creation                                      |
 *
 **/

/*==================[inclusions]=============================================*/
#include "camera_hal.h"
#include "esp_err.h"
#include "sdkconfig.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Initialize the AI-Thinker ESP32-CAM camera.
 *
 * Fills the HAL configuration with the module pinout and starts the OV2640
 * in JPEG mode.
 *
 * @param frame_size Output resolution.
 * @param jpeg_quality JPEG quality, 0-63 (lower is better).
 * @param fb_count Number of frame buffers (1-3).
 * @return ESP_OK on success, or the error reported by camera_hal_init().
 */
esp_err_t camera_ai_thinker_init(camera_hal_framesize_t frame_size,
								 uint8_t jpeg_quality, uint8_t fb_count);

#if CONFIG_SD_HAL_ENABLED
/**
 * @brief Mount the microSD card of the AI-Thinker module.
 *
 * The card slot is wired to the SDMMC host in 1-line mode (CLK=GPIO14,
 * CMD=GPIO15, D0=GPIO2). One-line mode avoids the GPIO12 strapping pin and
 * the GPIO4 flash LED used by the 4-line wiring.
 *
 * @return ESP_OK on success, or the error reported by sd_hal_mount().
 */
esp_err_t camera_ai_thinker_sd_init(void);
#endif

/** @} */
/** @} */

#endif /* #ifndef CAMERA_AI_THINKER_BSP_H */

/*==================[end of file]============================================*/
