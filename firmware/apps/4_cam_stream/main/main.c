/*! @mainpage ESP32-CAM MJPEG stream
 *
 * @section genDesc General Description
 *
 * Este ejemplo inicializa la cámara OV2640 de un módulo AI-Thinker
 * ESP32-CAM, se conecta a una red Wi-Fi y publica un stream MJPEG en un
 * servidor HTTP embebido. Se puede visualizar desde cualquier navegador
 * conectado a la misma red:
 *
 * @code
 * http://<IP_DEL_ESP32>/
 * @endcode
 *
 * El ejemplo respeta la arquitectura por capas del repositorio:
 *
 * - `drivers_hal`: acceso a la cámara (`camera_hal`) y al Wi-Fi (`wifi_hal`).
 * - `board_support`: pinout del módulo AI-Thinker (`camera_ai_thinker_bsp`).
 * - `middleware`: servidor HTTP y formato MJPEG (`camera_stream_server`).
 * - `apps`: este archivo, que sólo orquesta las capas anteriores.
 *
 * El componente `espressif/esp32-camera` se descarga automáticamente con el
 * component manager de ESP-IDF al compilar.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral   |   ESP32-CAM   |
 * |:---------------:|:-------------:|
 * |    Alimentación |      5V       |
 * |    GND          |      GND      |
 * |    TX (FTDI)    |   GPIO1 (U0TXD) |
 * |    RX (FTDI)    |   GPIO3 (U0RXD) |
 *
 * Para flashear, conectar GPIO0 a GND y resetear el módulo. Para ejecutar,
 * retirar el jumper de GPIO0 y resetear nuevamente.
 *
 * @section changelog Changelog
 *
 * |   Date     | Description                                            |
 * |:----------:|:-------------------------------------------------------|
 * | 06/11/2026 | Document creation                                      |
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "camera_ai_thinker_bsp.h"
#include "camera_stream_server.h"
#include "nvs_hal.h"
#include "wifi_hal.h"
/*==================[macros and definitions]=================================*/

#define CAM_STREAM_HTTP_PORT	80
#define WIFI_CONNECT_TIMEOUT_MS	30000

/*==================[internal data definition]===============================*/

static const char *TAG = "cam_stream";

/*==================[internal functions declaration]=========================*/

static camera_hal_framesize_t selected_frame_size(void);

/*==================[internal functions definition]==========================*/

static camera_hal_framesize_t selected_frame_size(void) {
#if CONFIG_CAM_STREAM_FRAME_SIZE_QVGA
	return CAMERA_HAL_FRAMESIZE_QVGA;
#elif CONFIG_CAM_STREAM_FRAME_SIZE_SVGA
	return CAMERA_HAL_FRAMESIZE_SVGA;
#elif CONFIG_CAM_STREAM_FRAME_SIZE_XGA
	return CAMERA_HAL_FRAMESIZE_XGA;
#else
	return CAMERA_HAL_FRAMESIZE_VGA;
#endif
}

/*==================[external functions definition]==========================*/

void app_main(void) {
	ESP_LOGI(TAG, "ESP32-CAM: ejemplo de stream MJPEG con ESP-IDF");

	/* El stack Wi-Fi guarda la calibración PHY en NVS: debe estar inicializado
	 * antes de esp_wifi_init(). */
	if (!NvsHalInit()) {
		ESP_LOGE(TAG, "No se pudo inicializar NVS");
		return;
	}

	esp_err_t err = camera_ai_thinker_init(selected_frame_size(),
										   CONFIG_CAM_STREAM_JPEG_QUALITY,
										   CONFIG_CAM_STREAM_FB_COUNT);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "La cámara no pudo inicializarse (%s). "
				 "Revisá el conector y la alimentación de 5 V.",
				 esp_err_to_name(err));
		return;
	}

	err = wifi_hal_init_sta(CONFIG_CAM_STREAM_WIFI_SSID,
							CONFIG_CAM_STREAM_WIFI_PASSWORD);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo iniciar Wi-Fi: %s", esp_err_to_name(err));
		return;
	}

	while (true) {
		err = wifi_hal_wait_connected(WIFI_CONNECT_TIMEOUT_MS);
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

	ESP_LOGI(TAG, "Visor listo en http://%s/", ip);
	ESP_LOGI(TAG, "Stream directo en http://%s/stream", ip);
	ESP_LOGI(TAG, "Captura JPEG en http://%s/capture", ip);

	err = camera_stream_server_start(CAM_STREAM_HTTP_PORT);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo iniciar el servidor de streaming: %s",
				 esp_err_to_name(err));
		return;
	}

	ESP_LOGI(TAG, "Servidor iniciado. Abrí la URL desde el navegador.");
}

/*==================[end of file]============================================*/
