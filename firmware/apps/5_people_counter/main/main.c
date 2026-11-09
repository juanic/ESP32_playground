/*! @mainpage People counter with ESP32-CAM
 *
 * @section genDesc General Description
 *
 * Captures a UXGA (1600x1200) JPEG every few minutes, stores it on the SD
 * card, uploads it to a local detection server and records the number of
 * people in a daily CSV. A small web UI shows the latest photo with the
 * detected people highlighted, exposes the sensor image controls and lets
 * the user download the stored files.
 *
 * There is no live streaming: the camera is only used for the periodic
 * captures and for the "capture now" button.
 *
 * Layers:
 *
 * - `drivers_hal`: camera (`camera_hal`), SD card (`sd_hal`) and Wi-Fi/SNTP
 *   (`wifi_hal`).
 * - `board_support`: AI-Thinker pinout (`camera_ai_thinker_bsp`).
 * - `middleware`: storage and CSV (`photo_repository`), detection upload
 *   (`detector_client`) and web UI (`monitor_web_server`).
 * - `apps`: this file, which only orchestrates the layers above.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral   |   ESP32-CAM   |
 * |:---------------:|:-------------:|
 * |    Power        |      5V       |
 * |    GND          |      GND      |
 * |    TX (FTDI)    |   GPIO1 (U0TXD) |
 * |    RX (FTDI)    |   GPIO3 (U0RXD) |
 *
 * The microSD card uses the SDMMC host in 1-line mode (CLK=14, CMD=15,
 * D0=2). To flash, connect GPIO0 to GND and reset the module; remove the
 * jumper and reset again to run.
 *
 * @section changelog Changelog
 *
 * |   Date     | Description                                            |
 * |:----------:|:-------------------------------------------------------|
 * | 09/11/2026 | Document creation                                      |
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "camera_ai_thinker_bsp.h"
#include "camera_hal.h"
#include "detector_client.h"
#include "esp_err.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "monitor_web_server.h"
#include "nvs_hal.h"
#include "photo_repository.h"
#include "sdkconfig.h"
#include "sd_hal.h"
#include "wifi_hal.h"
/*==================[macros and definitions]=================================*/

#define PEOPLE_WIFI_TIMEOUT_MS	30000
#define PEOPLE_TIME_LOG_MS		15000
#define PEOPLE_CAPTURE_BIT		BIT0
#define PEOPLE_JPEG_PATH_MAX	192
#define PEOPLE_DET_JSON_MAX		2048
#define PEOPLE_CAPTURE_STACK	8192
#define PEOPLE_CAPTURE_PRIO		5

/*==================[internal data definition]===============================*/

static const char *TAG = "people_counter";

static camera_hal_image_settings_t s_image = {
	.brightness = 0,
	.contrast = 0,
	.saturation = 0,
	.wb_mode = 0,
	.exposure_auto = 1,
	.exposure_level = 0,
	.gain_auto = 1,
	.gain_ceiling = 0,
	.hmirror = 0,
	.vflip = 0,
	.quality = CONFIG_PEOPLE_JPEG_QUALITY,
};

static uint8_t s_threshold_pct = CONFIG_PEOPLE_CONFIDENCE_PCT;
static EventGroupHandle_t s_events = NULL;

/*==================[internal functions declaration]=========================*/

static void capture_task(void *arg);
static void request_capture(void);
static esp_err_t ensure_sd(void);
static esp_err_t capture_photo(void);
static esp_err_t analyse_photo(const char *jpg_path, int64_t ts,
							   const uint8_t *jpeg, size_t jpeg_len);
static void retry_pending(void);
static uint8_t *read_file(const char *path, size_t *out_len);
static int64_t now_ms(void);

/*==================[internal functions definition]==========================*/

static int64_t now_ms(void) {
	return esp_timer_get_time() / 1000;
}

static void request_capture(void) {
	if (s_events != NULL) {
		xEventGroupSetBits(s_events, PEOPLE_CAPTURE_BIT);
	}
}

static esp_err_t ensure_sd(void) {
	if (sd_hal_is_mounted()) {
		return ESP_OK;
	}

	ESP_LOGW(TAG, "La microSD no está montada; reintentando");
	esp_err_t err = camera_ai_thinker_sd_init();
	if (err == ESP_OK) {
		err = photo_repo_init(sd_hal_mount_point());
	}
	return err;
}

static uint8_t *read_file(const char *path, size_t *out_len) {
	struct stat st;
	if (stat(path, &st) != 0 || st.st_size <= 0) {
		return NULL;
	}

	size_t size = (size_t)st.st_size;
	uint8_t *buffer = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
	if (buffer == NULL) {
		buffer = malloc(size);
	}
	if (buffer == NULL) {
		ESP_LOGE(TAG, "Sin memoria para leer '%s' (%u bytes)",
				 path, (unsigned)size);
		return NULL;
	}

	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		free(buffer);
		return NULL;
	}

	size_t read = fread(buffer, 1, size, file);
	fclose(file);
	if (read != size) {
		ESP_LOGW(TAG, "Lectura incompleta de '%s' (%u/%u)",
				 path, (unsigned)read, (unsigned)size);
		free(buffer);
		return NULL;
	}

	*out_len = size;
	return buffer;
}

static esp_err_t analyse_photo(const char *jpg_path, int64_t ts,
							   const uint8_t *jpeg, size_t jpeg_len) {
	char det_path[PEOPLE_JPEG_PATH_MAX];
	char csv_path[PEOPLE_JPEG_PATH_MAX];
	char tmp_jpg[PEOPLE_JPEG_PATH_MAX];
	if (photo_repo_build_paths(ts, tmp_jpg, sizeof(tmp_jpg), NULL, 0,
							   csv_path, sizeof(csv_path)) != ESP_OK) {
		return ESP_FAIL;
	}
	if (snprintf(det_path, sizeof(det_path), "%s", jpg_path) >=
		(int)sizeof(det_path)) {
		return ESP_FAIL;
	}
	char *dot = strrchr(det_path, '.');
	if (dot == NULL) {
		return ESP_FAIL;
	}
	snprintf(dot, sizeof(det_path) - (size_t)(dot - det_path), ".det");

	char json[PEOPLE_DET_JSON_MAX];
	detector_client_summary_t summary = {0};
	float threshold = (float)s_threshold_pct / 100.0f;

	esp_err_t err = detector_client_detect(CONFIG_PEOPLE_DETECTOR_URL,
										   jpeg, jpeg_len, threshold,
										   json, sizeof(json), &summary);
	if (err != ESP_OK) {
		ESP_LOGW(TAG, "Detección no disponible (%s); se reintentará",
				 esp_err_to_name(err));
		return err;
	}

	err = photo_repo_save_detection(det_path, json);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo guardar '%s'", det_path);
		return err;
	}

	photo_repo_detection_t det = {
		.count = summary.count,
		.confidence_avg = summary.confidence_avg,
		.confidence_min = summary.confidence_min,
		.confidence_max = summary.confidence_max,
	};
	if (photo_repo_upsert_csv(csv_path, ts, jpg_path, &det) != ESP_OK) {
		ESP_LOGW(TAG, "No se pudo actualizar '%s'", csv_path);
	}

	ESP_LOGI(TAG, "Personas: %u (confianza media %.0f%%)",
			 (unsigned)summary.count, summary.confidence_avg * 100.0f);
	return ESP_OK;
}

static esp_err_t capture_photo(void) {
	if (ensure_sd() != ESP_OK) {
		ESP_LOGW(TAG, "Captura omitida: no hay tarjeta SD");
		return ESP_ERR_INVALID_STATE;
	}

	int64_t ts = (int64_t)time(NULL);
	if (!wifi_hal_time_synced() || ts < 1700000000LL) {
		ESP_LOGW(TAG, "Captura omitida: la hora todavía no está sincronizada");
		return ESP_ERR_INVALID_STATE;
	}

	camera_hal_frame_t frame;
	esp_err_t err = camera_hal_get_frame(&frame);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo capturar la imagen");
		return err;
	}

	char jpg_path[PEOPLE_JPEG_PATH_MAX];
	char csv_path[PEOPLE_JPEG_PATH_MAX];
	err = photo_repo_build_paths(ts, jpg_path, sizeof(jpg_path),
								 NULL, 0, csv_path, sizeof(csv_path));
	if (err == ESP_OK) {
		err = photo_repo_save_jpeg(jpg_path, frame.data, frame.len);
	}
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo guardar la captura");
		camera_hal_return_frame(&frame);
		return err;
	}

	/* Fila provisional: si el detector no responde, el CSV igual conserva
	 * la captura y la fila se corrige cuando el reintento funcione. */
	photo_repo_append_csv(csv_path, ts, jpg_path, NULL, false);

	ESP_LOGI(TAG, "Foto guardada en %s (%u bytes, %ux%u)",
			 jpg_path, (unsigned)frame.len,
			 (unsigned)frame.width, (unsigned)frame.height);

	err = analyse_photo(jpg_path, ts, frame.data, frame.len);
	camera_hal_return_frame(&frame);
	return err;
}

static void retry_pending(void) {
	if (!sd_hal_is_mounted()) {
		return;
	}

	for (int i = 0; i < CONFIG_PEOPLE_PENDING_RETRY_MAX; i++) {
		char jpg_path[PEOPLE_JPEG_PATH_MAX];
		int64_t ts = 0;
		if (photo_repo_find_pending(jpg_path, sizeof(jpg_path), &ts) != ESP_OK) {
			return;
		}

		size_t jpeg_len = 0;
		uint8_t *jpeg = read_file(jpg_path, &jpeg_len);
		if (jpeg == NULL) {
			return;
		}

		ESP_LOGI(TAG, "Reintentando detección de %s", jpg_path);
		esp_err_t err = analyse_photo(jpg_path, ts, jpeg, jpeg_len);
		free(jpeg);
		if (err != ESP_OK) {
			return;
		}
	}
}

static bool is_work_time(void) {
	if (!wifi_hal_time_synced()) {
		return false;
	}

	time_t now = time(NULL);
	struct tm tm;
	localtime_r(&now, &tm);

#if CONFIG_PEOPLE_WORK_DAYS_ONLY
	/* 0 = domingo, 6 = sábado */
	if (tm.tm_wday == 0 || tm.tm_wday == 6) {
		return false;
	}
#endif

	return (tm.tm_hour >= CONFIG_PEOPLE_WORK_START_HOUR &&
			tm.tm_hour < CONFIG_PEOPLE_WORK_END_HOUR);
}

static void capture_task(void *arg) {
	(void)arg;

	while (!wifi_hal_time_synced()) {
		ESP_LOGW(TAG, "Esperando sincronización de hora (SNTP)...");
		vTaskDelay(pdMS_TO_TICKS(PEOPLE_TIME_LOG_MS));
	}
	ESP_LOGI(TAG, "Hora sincronizada; capturas automáticas configuradas de %02d:00 a %02d:00%s",
			 CONFIG_PEOPLE_WORK_START_HOUR, CONFIG_PEOPLE_WORK_END_HOUR,
#if CONFIG_PEOPLE_WORK_DAYS_ONLY
			 " (Lun a Vie)");
#else
			 "");
#endif

	if (is_work_time()) {
		retry_pending();
		capture_photo();
	} else {
		ESP_LOGI(TAG, "Fuera de horario laboral (%02d:00 a %02d:00); esperando ventana activa",
				 CONFIG_PEOPLE_WORK_START_HOUR, CONFIG_PEOPLE_WORK_END_HOUR);
	}

	int64_t interval_ms = (int64_t)CONFIG_PEOPLE_CAPTURE_INTERVAL_S * 1000;
	int64_t next_ms = now_ms() + interval_ms;

	while (true) {
		int64_t remaining = next_ms - now_ms();
		TickType_t ticks = remaining > 0
						   ? pdMS_TO_TICKS((uint32_t)remaining)
						   : 0;

		EventBits_t bits = xEventGroupWaitBits(s_events, PEOPLE_CAPTURE_BIT,
											   pdTRUE, pdFALSE, ticks);
		bool triggered = (bits & PEOPLE_CAPTURE_BIT) != 0;

		if (triggered) {
			ESP_LOGI(TAG, "Captura manual solicitada desde la web");
			capture_photo();
			next_ms = now_ms() + interval_ms;
			continue;
		}

		if (is_work_time()) {
			retry_pending();
			capture_photo();
		} else {
			ESP_LOGD(TAG, "Fuera de horario laboral; captura automática omitida");
		}

		next_ms += interval_ms;
		if (next_ms <= now_ms()) {
			/* Recuperación tras un fallo o una suspensión larga. */
			next_ms = now_ms() + interval_ms;
		}
	}
}

/*==================[external functions definition]==========================*/

void app_main(void) {
	ESP_LOGI(TAG, "Contador de personas con ESP32-CAM");

	setenv("TZ", CONFIG_PEOPLE_TIMEZONE, 1);
	tzset();

	if (!NvsHalInit()) {
		ESP_LOGE(TAG, "No se pudo inicializar NVS");
		return;
	}

	esp_err_t err = camera_ai_thinker_init(CAMERA_HAL_FRAMESIZE_UXGA,
										   CONFIG_PEOPLE_JPEG_QUALITY,
										   CONFIG_PEOPLE_FB_COUNT);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "La cámara no pudo inicializarse (%s). "
				 "Revisá el conector y la alimentación de 5 V.",
				 esp_err_to_name(err));
		return;
	}

	if (ensure_sd() != ESP_OK) {
		ESP_LOGW(TAG, "No hay tarjeta SD; las capturas se reintentarán "
				 "cuando se inserte una");
	}

	camera_hal_apply_settings(&s_image);

	err = wifi_hal_init_sta(CONFIG_PEOPLE_WIFI_SSID,
							CONFIG_PEOPLE_WIFI_PASSWORD);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo iniciar Wi-Fi: %s", esp_err_to_name(err));
		return;
	}

	while (true) {
		err = wifi_hal_wait_connected(PEOPLE_WIFI_TIMEOUT_MS);
		if (err == ESP_OK) {
			break;
		}
		if (err != ESP_ERR_TIMEOUT) {
			ESP_LOGE(TAG, "Error esperando Wi-Fi: %s", esp_err_to_name(err));
			return;
		}
		ESP_LOGW(TAG, "Wi-Fi aún sin IP; reintentando...");
	}

	char ip[16];
	if (wifi_hal_get_ip(ip, sizeof(ip)) != ESP_OK) {
		snprintf(ip, sizeof(ip), "?.?.?.?");
	}

	s_events = xEventGroupCreate();
	if (s_events == NULL) {
		ESP_LOGE(TAG, "No se pudo crear el grupo de eventos");
		return;
	}

	wifi_hal_start_sntp(CONFIG_PEOPLE_NTP_SERVER);

	const monitor_web_server_config_t web_config = {
		.port = CONFIG_PEOPLE_HTTP_PORT,
		.image = &s_image,
		.threshold_pct = &s_threshold_pct,
		.request_capture = request_capture,
	};
	err = monitor_web_server_start(&web_config);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo iniciar el servidor web: %s",
				 esp_err_to_name(err));
		return;
	}

	ESP_LOGI(TAG, "Panel disponible en http://%s/", ip);
	ESP_LOGI(TAG, "Detector configurado en %s", CONFIG_PEOPLE_DETECTOR_URL);

	if (xTaskCreate(capture_task, "capture", PEOPLE_CAPTURE_STACK, NULL,
					PEOPLE_CAPTURE_PRIO, NULL) != pdPASS) {
		ESP_LOGE(TAG, "No se pudo crear la tarea de captura");
	}
}

/*==================[end of file]============================================*/
