/*! @file camera_ai_thinker_bsp.c
 *  @brief Camera BSP for the AI-Thinker ESP32-CAM board.
 *  @defgroup camera_ai_thinker_bsp impl.
 *  @{
 *
 * El módulo AI-Thinker utiliza un sensor OV2640 conectado por DVP. El pinout
 * está fijado por la placa y se describe en el encabezado del BSP.
 */

/*==================[inclusions]=============================================*/
#include "camera_ai_thinker_bsp.h"
#include "esp_log.h"
#if CONFIG_SD_HAL_ENABLED
#include "sd_hal.h"
#endif
/*==================[macros and definitions]=================================*/

#define AI_THINKER_PIN_PWDN		32
#define AI_THINKER_PIN_RESET	-1
#define AI_THINKER_PIN_XCLK		0
#define AI_THINKER_PIN_SIOD		26
#define AI_THINKER_PIN_SIOC		27
#define AI_THINKER_PIN_D7		35
#define AI_THINKER_PIN_D6		34
#define AI_THINKER_PIN_D5		39
#define AI_THINKER_PIN_D4		36
#define AI_THINKER_PIN_D3		21
#define AI_THINKER_PIN_D2		19
#define AI_THINKER_PIN_D1		18
#define AI_THINKER_PIN_D0		5
#define AI_THINKER_PIN_VSYNC	25
#define AI_THINKER_PIN_HREF		23
#define AI_THINKER_PIN_PCLK		22

#define AI_THINKER_XCLK_HZ		20000000

/* microSD slot of the AI-Thinker module (SDMMC, 1-line mode) */
#define AI_THINKER_SD_CLK		14
#define AI_THINKER_SD_CMD		15
#define AI_THINKER_SD_D0		2
#define AI_THINKER_SD_MOUNT		"/sdcard"

/*==================[internal data definition]===============================*/

static const char *TAG = "camera_bsp";

/*==================[external functions definition]==========================*/

esp_err_t camera_ai_thinker_init(camera_hal_framesize_t frame_size,
								 uint8_t jpeg_quality, uint8_t fb_count) {
	const camera_hal_config_t config = {
		.pin_pwdn = AI_THINKER_PIN_PWDN,
		.pin_reset = AI_THINKER_PIN_RESET,
		.pin_xclk = AI_THINKER_PIN_XCLK,
		.pin_sccb_sda = AI_THINKER_PIN_SIOD,
		.pin_sccb_scl = AI_THINKER_PIN_SIOC,
		.pin_d7 = AI_THINKER_PIN_D7,
		.pin_d6 = AI_THINKER_PIN_D6,
		.pin_d5 = AI_THINKER_PIN_D5,
		.pin_d4 = AI_THINKER_PIN_D4,
		.pin_d3 = AI_THINKER_PIN_D3,
		.pin_d2 = AI_THINKER_PIN_D2,
		.pin_d1 = AI_THINKER_PIN_D1,
		.pin_d0 = AI_THINKER_PIN_D0,
		.pin_vsync = AI_THINKER_PIN_VSYNC,
		.pin_href = AI_THINKER_PIN_HREF,
		.pin_pclk = AI_THINKER_PIN_PCLK,
		.xclk_freq_hz = AI_THINKER_XCLK_HZ,
		.frame_size = frame_size,
		.pixel_format = CAMERA_HAL_PIXFORMAT_JPEG,
		.jpeg_quality = jpeg_quality,
		.fb_count = fb_count,
		.fb_in_psram = true,
		.grab_latest = true,
	};

	esp_err_t err = camera_hal_init(&config);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo inicializar la cámara del ESP32-CAM: %s",
				 esp_err_to_name(err));
	}
	return err;
}

#if CONFIG_SD_HAL_ENABLED
esp_err_t camera_ai_thinker_sd_init(void) {
	const sd_hal_config_t config = {
		.pin_clk = AI_THINKER_SD_CLK,
		.pin_cmd = AI_THINKER_SD_CMD,
		.pin_d0 = AI_THINKER_SD_D0,
		.pin_d1 = -1,
		.pin_d2 = -1,
		.pin_d3 = -1,
		.one_bit = true,
		.mount_point = AI_THINKER_SD_MOUNT,
	};

	esp_err_t err = sd_hal_mount(&config);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo montar la microSD: %s", esp_err_to_name(err));
	}
	return err;
}
#endif

/*==================[end of file]============================================*/
