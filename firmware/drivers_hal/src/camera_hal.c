/*! @file camera_hal.c
 *  @brief Camera HAL implementation for ESP-IDF 6.x.
 *  @defgroup camera_hal impl.
 *  @{
 *
 * Envuelve el componente `esp32-camera` y traduce los tipos públicos del HAL
 * a los tipos internos del driver. La selección de sensor la realiza el
 * propio componente durante esp_camera_init().
 */

/*==================[inclusions]=============================================*/
#include "camera_hal.h"
#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <string.h>
/*==================[macros and definitions]=================================*/

#define CAMERA_HAL_DEFAULT_XCLK_HZ	20000000
#define CAMERA_HAL_DEFAULT_FB_COUNT	2

/*==================[internal data definition]===============================*/

static const char *TAG = "camera_hal";
static bool s_initialized = false;
static const char *s_sensor_name = NULL;

/*==================[internal functions declaration]=========================*/

static framesize_t to_framesize(camera_hal_framesize_t size);
static pixformat_t to_pixformat(camera_hal_pixformat_t format);
static camera_hal_pixformat_t from_pixformat(pixformat_t format);

/*==================[internal functions definition]==========================*/

static framesize_t to_framesize(camera_hal_framesize_t size) {
	switch (size) {
		case CAMERA_HAL_FRAMESIZE_QVGA:	return FRAMESIZE_QVGA;
		case CAMERA_HAL_FRAMESIZE_CIF:	return FRAMESIZE_CIF;
		case CAMERA_HAL_FRAMESIZE_VGA:	return FRAMESIZE_VGA;
		case CAMERA_HAL_FRAMESIZE_SVGA:	return FRAMESIZE_SVGA;
		case CAMERA_HAL_FRAMESIZE_XGA:	return FRAMESIZE_XGA;
		case CAMERA_HAL_FRAMESIZE_HD:	return FRAMESIZE_HD;
		case CAMERA_HAL_FRAMESIZE_SXGA:	return FRAMESIZE_SXGA;
		case CAMERA_HAL_FRAMESIZE_UXGA:	return FRAMESIZE_UXGA;
		default:						return FRAMESIZE_VGA;
	}
}

static pixformat_t to_pixformat(camera_hal_pixformat_t format) {
	switch (format) {
		case CAMERA_HAL_PIXFORMAT_RGB565:	return PIXFORMAT_RGB565;
		case CAMERA_HAL_PIXFORMAT_GRAYSCALE:return PIXFORMAT_GRAYSCALE;
		case CAMERA_HAL_PIXFORMAT_JPEG:
		default:							return PIXFORMAT_JPEG;
	}
}

static camera_hal_pixformat_t from_pixformat(pixformat_t format) {
	switch (format) {
		case PIXFORMAT_RGB565:	return CAMERA_HAL_PIXFORMAT_RGB565;
		case PIXFORMAT_GRAYSCALE:return CAMERA_HAL_PIXFORMAT_GRAYSCALE;
		case PIXFORMAT_JPEG:
		default:				return CAMERA_HAL_PIXFORMAT_JPEG;
	}
}

/*==================[external functions definition]==========================*/

esp_err_t camera_hal_init(const camera_hal_config_t *config) {
	if (config == NULL) {
		return ESP_ERR_INVALID_ARG;
	}
	if (s_initialized) {
		return ESP_ERR_INVALID_STATE;
	}

	camera_config_t camera_config = {0};
	camera_config.pin_pwdn = config->pin_pwdn;
	camera_config.pin_reset = config->pin_reset;
	camera_config.pin_xclk = config->pin_xclk;
	camera_config.pin_sccb_sda = config->pin_sccb_sda;
	camera_config.pin_sccb_scl = config->pin_sccb_scl;
	camera_config.pin_d7 = config->pin_d7;
	camera_config.pin_d6 = config->pin_d6;
	camera_config.pin_d5 = config->pin_d5;
	camera_config.pin_d4 = config->pin_d4;
	camera_config.pin_d3 = config->pin_d3;
	camera_config.pin_d2 = config->pin_d2;
	camera_config.pin_d1 = config->pin_d1;
	camera_config.pin_d0 = config->pin_d0;
	camera_config.pin_vsync = config->pin_vsync;
	camera_config.pin_href = config->pin_href;
	camera_config.pin_pclk = config->pin_pclk;

	camera_config.xclk_freq_hz = config->xclk_freq_hz;
	if (camera_config.xclk_freq_hz == 0) {
		camera_config.xclk_freq_hz = CAMERA_HAL_DEFAULT_XCLK_HZ;
	}
	camera_config.ledc_timer = LEDC_TIMER_0;
	camera_config.ledc_channel = LEDC_CHANNEL_0;
	camera_config.pixel_format = to_pixformat(config->pixel_format);
	camera_config.frame_size = to_framesize(config->frame_size);
	camera_config.jpeg_quality = config->jpeg_quality;
	camera_config.fb_count = config->fb_count ? config->fb_count : CAMERA_HAL_DEFAULT_FB_COUNT;
	camera_config.grab_mode = config->grab_latest ? CAMERA_GRAB_LATEST : CAMERA_GRAB_WHEN_EMPTY;
	camera_config.sccb_i2c_port = -1;

	bool has_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM) > 0;
	if (config->fb_in_psram && has_psram) {
		camera_config.fb_location = CAMERA_FB_IN_PSRAM;
	} else {
		if (config->fb_in_psram) {
			ESP_LOGW(TAG, "PSRAM no disponible; los frame buffers se asignan en DRAM");
		}
		camera_config.fb_location = CAMERA_FB_IN_DRAM;
	}

	esp_err_t err = esp_camera_init(&camera_config);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "esp_camera_init falló: %s", esp_err_to_name(err));
		return err;
	}

	sensor_t *sensor = esp_camera_sensor_get();
	if (sensor != NULL) {
		camera_sensor_info_t *info = esp_camera_sensor_get_info(&sensor->id);
		s_sensor_name = info != NULL ? info->name : NULL;
		ESP_LOGI(TAG, "Sensor detectado: %s (PID=0x%04x VER=0x%02x MID=0x%02x%02x)",
				 s_sensor_name != NULL ? s_sensor_name : "desconocido",
				 sensor->id.PID, sensor->id.VER,
				 sensor->id.MIDH, sensor->id.MIDL);
	} else {
		ESP_LOGW(TAG, "No se pudo leer la identificación del sensor");
	}

	s_initialized = true;
	ESP_LOGI(TAG, "Cámara inicializada (resolución=%d, %u frame buffers, fb en %s)",
			 (int)config->frame_size, (unsigned)camera_config.fb_count,
			 camera_config.fb_location == CAMERA_FB_IN_PSRAM ? "PSRAM" : "DRAM");
	return ESP_OK;
}

esp_err_t camera_hal_deinit(void) {
	if (!s_initialized) {
		return ESP_ERR_INVALID_STATE;
	}
	esp_err_t err = esp_camera_deinit();
	if (err == ESP_OK) {
		s_initialized = false;
	}
	return err;
}

esp_err_t camera_hal_get_frame(camera_hal_frame_t *frame) {
	if (!s_initialized || frame == NULL) {
		return ESP_ERR_INVALID_STATE;
	}

	memset(frame, 0, sizeof(*frame));

	camera_fb_t *fb = esp_camera_fb_get();
	if (fb == NULL) {
		ESP_LOGW(TAG, "No se pudo obtener un frame");
		return ESP_FAIL;
	}

	frame->data = fb->buf;
	frame->len = fb->len;
	frame->width = (uint16_t)fb->width;
	frame->height = (uint16_t)fb->height;
	frame->format = from_pixformat(fb->format);
	frame->internal = fb;
	return ESP_OK;
}

void camera_hal_return_frame(camera_hal_frame_t *frame) {
	if (frame == NULL || frame->internal == NULL) {
		return;
	}
	esp_camera_fb_return((camera_fb_t *)frame->internal);
	frame->internal = NULL;
	frame->data = NULL;
	frame->len = 0;
}

static int clamp_int(int value, int min, int max) {
	if (value < min) {
		return min;
	}
	if (value > max) {
		return max;
	}
	return value;
}

esp_err_t camera_hal_apply_settings(const camera_hal_image_settings_t *settings) {
	if (!s_initialized) {
		return ESP_ERR_INVALID_STATE;
	}

	sensor_t *sensor = esp_camera_sensor_get();
	if (sensor == NULL) {
		return ESP_ERR_INVALID_STATE;
	}
	if (settings == NULL) {
		return ESP_OK;
	}

	int result = 0;
	result |= sensor->set_brightness(sensor, clamp_int(settings->brightness, -2, 2));
	result |= sensor->set_contrast(sensor, clamp_int(settings->contrast, -2, 2));
	result |= sensor->set_saturation(sensor, clamp_int(settings->saturation, -2, 2));
	result |= sensor->set_wb_mode(sensor, clamp_int(settings->wb_mode, 0, 4));
	result |= sensor->set_exposure_ctrl(sensor, settings->exposure_auto ? 1 : 0);
	if (!settings->exposure_auto) {
		result |= sensor->set_aec_value(sensor,
										clamp_int(settings->exposure_level, 0, 1200));
	}
	result |= sensor->set_gain_ctrl(sensor, settings->gain_auto ? 1 : 0);
	if (!settings->gain_auto) {
		result |= sensor->set_gainceiling(sensor,
										  (gainceiling_t)clamp_int(settings->gain_ceiling, 0, 6));
	}
	result |= sensor->set_hmirror(sensor, settings->hmirror ? 1 : 0);
	result |= sensor->set_vflip(sensor, settings->vflip ? 1 : 0);
	result |= sensor->set_quality(sensor, clamp_int(settings->quality, 0, 63));

	if (result != 0) {
		ESP_LOGW(TAG, "El sensor rechazó parte de la configuración de imagen");
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "Imagen: brillo=%d contraste=%d saturación=%d wb=%u calidad=%u",
			 (int)settings->brightness, (int)settings->contrast,
			 (int)settings->saturation, (unsigned)settings->wb_mode,
			 (unsigned)settings->quality);
	return ESP_OK;
}

const char *camera_hal_sensor_name(void) {
	return s_sensor_name;
}

/*==================[end of file]============================================*/
