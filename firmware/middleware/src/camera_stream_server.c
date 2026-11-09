/*! @file camera_stream_server.c
 *  @brief HTTP MJPEG streaming server built on esp_http_server.
 *  @defgroup camera_stream_server impl.
 *  @{
 *
 * Implementa el mismo esquema que el ejemplo CameraWebServer de Arduino,
 * pero usando la API de ESP-IDF y la HAL de cámara del repositorio.
 *
 * El stream MJPEG se envía con `Transfer-Encoding: chunked` y el clásico
 * formato `multipart/x-mixed-replace`, que es interpretado directamente por
 * la etiqueta <img> de cualquier navegador.
 */

/*==================[inclusions]=============================================*/
#include "camera_stream_server.h"
#include "camera_hal.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
/*==================[macros and definitions]=================================*/

#define STREAM_BOUNDARY			"123456789000000000000987654321"
#define STREAM_CONTENT_TYPE		"multipart/x-mixed-replace;boundary=" STREAM_BOUNDARY
#define STREAM_BOUNDARY_CHUNK	"\r\n--" STREAM_BOUNDARY "\r\n"
#define STREAM_PART_HEADER		"Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n"

#define STREAM_ERROR_DELAY_MS	50

/*==================[internal data definition]===============================*/

static const char *TAG = "camera_stream";
static httpd_handle_t s_server = NULL;

/* La cámara admite un solo consumidor: esp_camera_fb_get() bloquea sin
 * timeout cuando otro cliente ya está usando los frame buffers. Este mutex
 * garantiza que sólo un handler (stream o captura) toque la cámara a la vez,
 * y evita que una petición deje el servidor HTTP bloqueado para siempre. */
static SemaphoreHandle_t s_camera_lock = NULL;
static StaticSemaphore_t s_camera_lock_storage;

static const char s_index_html[] =
	"<!doctype html>\n"
	"<html lang=\"es\">\n"
	"<head>\n"
	"<meta charset=\"utf-8\">\n"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
	"<title>ESP32-CAM</title>\n"
	"<style>\n"
	"  body { margin: 0; min-height: 100vh; display: flex; flex-direction: column;\n"
	"         align-items: center; justify-content: center; gap: 16px;\n"
	"         background: #101418; color: #e6edf3; font-family: system-ui, sans-serif; }\n"
	"  h1 { font-size: 1.25rem; font-weight: 500; margin: 0; }\n"
	"  img { width: min(96vw, 800px); height: auto; background: #000;\n"
	"        border-radius: 8px; box-shadow: 0 8px 32px rgba(0, 0, 0, .5); }\n"
	"  a { color: #58a6ff; font-size: .9rem; }\n"
	"</style>\n"
	"</head>\n"
	"<body>\n"
	"<h1>ESP32-CAM</h1>\n"
	"<img src=\"/stream\" alt=\"Stream de la cámara\">\n"
	"<a href=\"/capture\">Descargar una captura JPEG</a>\n"
	"</body>\n"
	"</html>\n";

/*==================[internal functions declaration]=========================*/

static esp_err_t index_get_handler(httpd_req_t *req);
static esp_err_t stream_get_handler(httpd_req_t *req);
static esp_err_t capture_get_handler(httpd_req_t *req);
static bool camera_lock_take(void);
static void camera_lock_give(void);
static esp_err_t send_camera_busy(httpd_req_t *req);

/*==================[internal functions definition]==========================*/

static bool camera_lock_take(void) {
	return s_camera_lock != NULL && xSemaphoreTake(s_camera_lock, 0) == pdTRUE;
}

static void camera_lock_give(void) {
	if (s_camera_lock != NULL) {
		xSemaphoreGive(s_camera_lock);
	}
}

static esp_err_t send_camera_busy(httpd_req_t *req) {
	httpd_resp_set_status(req, "503 Service Unavailable");
	httpd_resp_set_type(req, "text/plain; charset=utf-8");
	httpd_resp_set_hdr(req, "Cache-Control", "no-store");
	httpd_resp_set_hdr(req, "Connection", "close");
	return httpd_resp_sendstr(req, "La cámara ya está en uso por otro cliente.\n");
}

static esp_err_t index_get_handler(httpd_req_t *req) {
	esp_err_t err = httpd_resp_set_type(req, "text/html; charset=utf-8");
	if (err != ESP_OK) {
		return err;
	}
	return httpd_resp_send(req, s_index_html, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t stream_get_handler(httpd_req_t *req) {
	if (!camera_lock_take()) {
		ESP_LOGW(TAG, "Stream rechazado: la cámara ya está en uso");
		return send_camera_busy(req);
	}

	esp_err_t err = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
	if (err != ESP_OK) {
		camera_lock_give();
		return err;
	}
	httpd_resp_set_hdr(req, "Cache-Control", "no-store");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

	ESP_LOGI(TAG, "Cliente conectado al stream");

	bool frame_info_logged = false;
	while (true) {
		camera_hal_frame_t frame;
		err = camera_hal_get_frame(&frame);
		if (err != ESP_OK) {
			vTaskDelay(pdMS_TO_TICKS(STREAM_ERROR_DELAY_MS));
			continue;
		}

		if (!frame_info_logged) {
			ESP_LOGI(TAG, "Frame %ux%u, %u bytes",
					 (unsigned)frame.width, (unsigned)frame.height,
					 (unsigned)frame.len);
			frame_info_logged = true;
		}

		char part_header[64];
		int header_len = snprintf(part_header, sizeof(part_header),
								 STREAM_PART_HEADER, (unsigned)frame.len);

		err = httpd_resp_send_chunk(req, STREAM_BOUNDARY_CHUNK,
									strlen(STREAM_BOUNDARY_CHUNK));
		if (err == ESP_OK) {
			err = httpd_resp_send_chunk(req, part_header, header_len);
		}
		if (err == ESP_OK) {
			err = httpd_resp_send_chunk(req, (const char *)frame.data, frame.len);
		}

		camera_hal_return_frame(&frame);

		if (err != ESP_OK) {
			ESP_LOGI(TAG, "Cliente desconectado del stream");
			break;
		}
	}

	camera_lock_give();
	return err;
}

static esp_err_t capture_get_handler(httpd_req_t *req) {
	if (!camera_lock_take()) {
		ESP_LOGW(TAG, "Captura rechazada: la cámara ya está en uso");
		return send_camera_busy(req);
	}

	camera_hal_frame_t frame;
	esp_err_t err = camera_hal_get_frame(&frame);
	if (err != ESP_OK) {
		camera_lock_give();
		httpd_resp_send_500(req);
		return err;
	}

	ESP_LOGI(TAG, "Captura %ux%u, %u bytes",
			 (unsigned)frame.width, (unsigned)frame.height, (unsigned)frame.len);

	err = httpd_resp_set_type(req, "image/jpeg");
	if (err == ESP_OK) {
		httpd_resp_set_hdr(req, "Content-Disposition",
						   "inline; filename=\"capture.jpg\"");
		err = httpd_resp_send(req, (const char *)frame.data, frame.len);
	}

	camera_hal_return_frame(&frame);
	camera_lock_give();
	return err;
}

/*==================[external functions definition]==========================*/

esp_err_t camera_stream_server_start(uint16_t port) {
	if (s_server != NULL) {
		return ESP_ERR_INVALID_STATE;
	}

	if (s_camera_lock == NULL) {
		s_camera_lock = xSemaphoreCreateBinaryStatic(&s_camera_lock_storage);
		if (s_camera_lock == NULL) {
			ESP_LOGE(TAG, "No se pudo crear el mutex de cámara");
			return ESP_ERR_NO_MEM;
		}
		xSemaphoreGive(s_camera_lock);
	}

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.server_port = port;
	config.ctrl_port = port + 1;
	config.max_open_sockets = 4;
	config.lru_purge_enable = true;
	config.stack_size = 8192;
	config.recv_wait_timeout = 5;
	config.send_wait_timeout = 5;

	esp_err_t err = httpd_start(&s_server, &config);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo iniciar el servidor HTTP: %s", esp_err_to_name(err));
		s_server = NULL;
		return err;
	}

	const httpd_uri_t index_uri = {
		.uri = "/",
		.method = HTTP_GET,
		.handler = index_get_handler,
		.user_ctx = NULL,
	};
	const httpd_uri_t stream_uri = {
		.uri = "/stream",
		.method = HTTP_GET,
		.handler = stream_get_handler,
		.user_ctx = NULL,
	};
	const httpd_uri_t capture_uri = {
		.uri = "/capture",
		.method = HTTP_GET,
		.handler = capture_get_handler,
		.user_ctx = NULL,
	};

	httpd_register_uri_handler(s_server, &index_uri);
	httpd_register_uri_handler(s_server, &stream_uri);
	httpd_register_uri_handler(s_server, &capture_uri);

	ESP_LOGI(TAG, "Servidor HTTP escuchando en el puerto %u", (unsigned)port);
	return ESP_OK;
}

esp_err_t camera_stream_server_stop(void) {
	if (s_server == NULL) {
		return ESP_ERR_INVALID_STATE;
	}
	esp_err_t err = httpd_stop(s_server);
	s_server = NULL;
	return err;
}

/*==================[end of file]============================================*/
