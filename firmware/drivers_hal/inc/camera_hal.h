#ifndef CAMERA_HAL_H
#define CAMERA_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup camera_hal Camera HAL
 *  @brief Driver for parallel camera sensors (DVP) on ESP32.
 *  @{
 *
 * @section genDesc General Description
 *
 * This driver wraps the Espressif `esp32-camera` component and exposes a
 * hardware-independent API to initialize a DVP camera and acquire JPEG
 * frames. The camera pinout and sensor selection belong to the board support
 * layer (BSP), not to this HAL.
 *
 * The component is only compiled when `CONFIG_CAMERA_HAL_ENABLED` is set.
 *
 * @note For streaming, use JPEG pixel format. Raw formats (RGB565,
 * grayscale) require much more memory and are not handled by the example
 * HTTP server.
 *
 * @section changelog Changelog
 *
 * |   Date     | Description                                            |
 * |:----------:|:-------------------------------------------------------|
 * | 06/11/2026 | Document creation                                      |
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief Supported output resolutions.
 *
 * The values are mapped internally to the `framesize_t` enum of the
 * `esp32-camera` component.
 */
typedef enum {
	CAMERA_HAL_FRAMESIZE_QVGA = 0,	/**< 320x240 */
	CAMERA_HAL_FRAMESIZE_CIF,		/**< 400x296 */
	CAMERA_HAL_FRAMESIZE_VGA,		/**< 640x480 */
	CAMERA_HAL_FRAMESIZE_SVGA,		/**< 800x600 */
	CAMERA_HAL_FRAMESIZE_XGA,		/**< 1024x768 */
	CAMERA_HAL_FRAMESIZE_HD,		/**< 1280x720 */
	CAMERA_HAL_FRAMESIZE_SXGA,		/**< 1280x1024 */
	CAMERA_HAL_FRAMESIZE_UXGA		/**< 1600x1200 */
} camera_hal_framesize_t;

/**
 * @brief Supported pixel formats.
 */
typedef enum {
	CAMERA_HAL_PIXFORMAT_JPEG = 0,	/**< JPEG (recommended for streaming) */
	CAMERA_HAL_PIXFORMAT_RGB565,	/**< RGB565 */
	CAMERA_HAL_PIXFORMAT_GRAYSCALE	/**< 8-bit grayscale */
} camera_hal_pixformat_t;

/**
 * @brief Camera initialization configuration.
 *
 * Pins set to -1 are not connected. The BSP is responsible for filling this
 * structure with the pinout of the board.
 */
typedef struct {
	int pin_pwdn;					/**< Power-down pin, -1 if unused */
	int pin_reset;					/**< Reset pin, -1 if unused */
	int pin_xclk;					/**< Master clock output pin */
	int pin_sccb_sda;				/**< SCCB (I2C) data pin */
	int pin_sccb_scl;				/**< SCCB (I2C) clock pin */
	int pin_d7;						/**< Data bit 7 */
	int pin_d6;						/**< Data bit 6 */
	int pin_d5;						/**< Data bit 5 */
	int pin_d4;						/**< Data bit 4 */
	int pin_d3;						/**< Data bit 3 */
	int pin_d2;						/**< Data bit 2 */
	int pin_d1;						/**< Data bit 1 */
	int pin_d0;						/**< Data bit 0 */
	int pin_vsync;					/**< Vertical sync */
	int pin_href;					/**< Horizontal reference */
	int pin_pclk;					/**< Pixel clock */
	uint32_t xclk_freq_hz;			/**< XCLK frequency in Hz (typically 20 MHz) */
	camera_hal_framesize_t frame_size;	/**< Output resolution */
	camera_hal_pixformat_t pixel_format;	/**< Output pixel format */
	uint8_t jpeg_quality;			/**< JPEG quality, 0-63 (lower is better) */
	uint8_t fb_count;				/**< Number of frame buffers (1-3) */
	bool fb_in_psram;				/**< Allocate frame buffers in PSRAM when available */
	bool grab_latest;				/**< Prefer the newest frame (lower latency) */
} camera_hal_config_t;

/**
 * @brief Acquired frame.
 *
 * The application must call camera_hal_return_frame() when it finishes using
 * the data. The `internal` field is owned by the HAL.
 */
typedef struct {
	uint8_t *data;					/**< Pointer to the encoded/captured data */
	size_t len;						/**< Data length in bytes */
	uint16_t width;					/**< Frame width in pixels */
	uint16_t height;				/**< Frame height in pixels */
	camera_hal_pixformat_t format;	/**< Pixel format of this frame */
	void *internal;					/**< Driver frame handle (do not modify) */
} camera_hal_frame_t;

/**
 * @brief Image controls applied to the sensor.
 *
 * Only the controls marked as "manual" take effect; the rest stay in auto
 * mode. Values outside the sensor range are clamped by the driver.
 */
typedef struct {
	int8_t brightness;			/**< -2..2 */
	int8_t contrast;			/**< -2..2 */
	int8_t saturation;			/**< -2..2 */
	uint8_t wb_mode;			/**< 0 auto, 1 sunny, 2 cloudy, 3 office, 4 home */
	uint8_t exposure_auto;		/**< 1 = auto exposure, 0 = manual */
	int16_t exposure_level;		/**< Manual AEC value, 0..1200 */
	uint8_t gain_auto;			/**< 1 = auto gain, 0 = manual */
	uint8_t gain_ceiling;		/**< 0 = 2x, 1 = 4x, ... 6 = 128x */
	uint8_t hmirror;			/**< 1 = horizontal mirror */
	uint8_t vflip;				/**< 1 = vertical flip */
	uint8_t quality;			/**< JPEG quality, 0..63 (lower is better) */
} camera_hal_image_settings_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Initialize the camera driver and detect the sensor.
 *
 * @param config Camera pinout and acquisition parameters.
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if config is NULL,
 *         ESP_ERR_INVALID_STATE if the driver is already initialized, or the
 *         error reported by esp_camera_init().
 */
esp_err_t camera_hal_init(const camera_hal_config_t *config);

/**
 * @brief Deinitialize the camera driver and release its buffers.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if not initialized.
 */
esp_err_t camera_hal_deinit(void);

/**
 * @brief Acquire the next frame from the camera.
 *
 * This function blocks until a frame is available.
 *
 * @param frame Output frame descriptor.
 * @return ESP_OK on success, ESP_FAIL if no frame could be acquired.
 */
esp_err_t camera_hal_get_frame(camera_hal_frame_t *frame);

/**
 * @brief Return a frame to the camera driver.
 *
 * @param frame Frame previously obtained with camera_hal_get_frame().
 */
void camera_hal_return_frame(camera_hal_frame_t *frame);

/**
 * @brief Apply image controls to the detected sensor.
 *
 * @param settings Controls to apply. NULL restores the sensor defaults.
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if the camera is not
 *         initialized, or ESP_FAIL if the driver rejected a value.
 */
esp_err_t camera_hal_apply_settings(const camera_hal_image_settings_t *settings);

/**
 * @brief Get the name of the detected sensor.
 *
 * @return Sensor name ("OV2640", ...) or NULL if not initialized.
 */
const char *camera_hal_sensor_name(void);

/** @} */
/** @} */

#endif /* #ifndef CAMERA_HAL_H */

/*==================[end of file]============================================*/
