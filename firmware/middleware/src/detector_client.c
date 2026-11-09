/*! @file detector_client.c
 *  @brief HTTP client for the external people detection service.
 *  @defgroup detector_client impl.
 *  @{
 *
 * Sube la captura al servidor de detecciÃƒÂ³n y traduce la respuesta de texto
 * (`box=x,y,w,h,score`) al JSON compacto que consume el visor web.
 */

/*==================[inclusions]=============================================*/
#include "detector_client.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "photo_repository.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*==================[macros and definitions]=================================*/

#define DETECTOR_HTTP_TIMEOUT_MS		60000
#define DETECTOR_RESPONSE_MAX		2048

/*==================[internal data definition]===============================*/

static const char *TAG = "detector_client";

/*==================[internal functions declaration]=========================*/

static esp_err_t build_json(const float boxes[][5], size_t count,
							char *out, size_t out_len,
							detector_client_summary_t *summary);

/*==================[internal functions definition]==========================*/

typedef struct {
	char *buffer;
	size_t size;
	size_t written;
} detector_response_ctx_t;

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
	if (evt->event_id == HTTP_EVENT_ON_DATA) {
		detector_response_ctx_t *ctx = (detector_response_ctx_t *)evt->user_data;
		if (ctx != NULL && ctx->buffer != NULL) {
			size_t copy_len = (size_t)evt->data_len;
			if (ctx->written + copy_len >= ctx->size) {
				copy_len = ctx->size > ctx->written + 1 ? ctx->size - 1 - ctx->written : 0;
			}
			if (copy_len > 0) {
				memcpy(ctx->buffer + ctx->written, evt->data, copy_len);
				ctx->written += copy_len;
				ctx->buffer[ctx->written] = '\0';
			}
		}
	}
	return ESP_OK;
}

static esp_err_t build_json(const float boxes[][5], size_t count,
							char *out, size_t out_len,
							detector_client_summary_t *summary) {
	float sum = 0.0f;
	float min = 1.0f;
	float max = 0.0f;

	for (size_t i = 0; i < count; i++) {
		float score = boxes[i][4];
		sum += score;
		if (score < min) {
			min = score;
		}
		if (score > max) {
			max = score;
		}
	}
	if (count == 0) {
		min = 0.0f;
	}

	float avg = count > 0 ? sum / (float)count : 0.0f;

	size_t used = 0;
	int written = snprintf(out, out_len,
						   "{\"count\":%u,\"avg\":%.3f,\"min\":%.3f,\"max\":%.3f,\"boxes\":[",
						   (unsigned)count, avg, min, max);
	if (written < 0 || (size_t)written >= out_len) {
		return ESP_ERR_INVALID_SIZE;
	}
	used = (size_t)written;

	for (size_t i = 0; i < count; i++) {
		written = snprintf(out + used, out_len - used,
						   "%s[%.4f,%.4f,%.4f,%.4f,%.4f]",
						   i > 0 ? "," : "",
						   boxes[i][0], boxes[i][1], boxes[i][2],
						   boxes[i][3], boxes[i][4]);
		if (written < 0 || (size_t)written >= out_len - used) {
			return ESP_ERR_INVALID_SIZE;
		}
		used += (size_t)written;
	}

	written = snprintf(out + used, out_len - used, "]}");
	if (written < 0 || (size_t)written >= out_len - used) {
		return ESP_ERR_INVALID_SIZE;
	}

	if (summary != NULL) {
		summary->count = (uint32_t)count;
		summary->confidence_avg = avg;
		summary->confidence_min = min;
		summary->confidence_max = max;
	}
	return ESP_OK;
}

/*==================[external functions definition]==========================*/

esp_err_t detector_client_detect(const char *url, const uint8_t *jpeg,
								 size_t jpeg_len, float threshold,
								 char *json_out, size_t json_len,
								 detector_client_summary_t *summary) {
	if (url == NULL || jpeg == NULL || jpeg_len == 0 ||
		json_out == NULL || json_len == 0) {
		return ESP_ERR_INVALID_ARG;
	}

	char response[DETECTOR_RESPONSE_MAX] = {0};
	detector_response_ctx_t resp_ctx = {
		.buffer = response,
		.size = sizeof(response),
		.written = 0,
	};

	const esp_http_client_config_t config = {
		.url = url,
		.method = HTTP_METHOD_POST,
		.timeout_ms = DETECTOR_HTTP_TIMEOUT_MS,
		.buffer_size = 1024,
		.buffer_size_tx = 2048,
		.disable_auto_redirect = true,
		.event_handler = http_event_handler,
		.user_data = &resp_ctx,
	};

	esp_http_client_handle_t client = esp_http_client_init(&config);
	if (client == NULL) {
		ESP_LOGE(TAG, "No se pudo crear el cliente HTTP");
		return ESP_FAIL;
	}

	char threshold_str[8];
	snprintf(threshold_str, sizeof(threshold_str), "%.3f", threshold);
	esp_http_client_set_header(client, "Content-Type", "image/jpeg");
	esp_http_client_set_header(client, "X-Threshold", threshold_str);
	esp_http_client_set_post_field(client, (const char *)jpeg, (int)jpeg_len);

	esp_err_t err = esp_http_client_perform(client);
	int status = esp_http_client_get_status_code(client);
	esp_http_client_cleanup(client);

	if (err != ESP_OK) {
		ESP_LOGW(TAG, "Fallo al contactar el detector: %s", esp_err_to_name(err));
		return err;
	}
	if (status != 200) {
		ESP_LOGW(TAG, "El detector respondió HTTP %d", status);
		return ESP_ERR_INVALID_RESPONSE;
	}

	float boxes[PHOTO_REPO_MAX_BOXES][5];
	size_t box_count = 0;
	char *saveptr = NULL;
	char *line = strtok_r(response, "\r\n", &saveptr);
	while (line != NULL && box_count < PHOTO_REPO_MAX_BOXES) {
		float x, y, w, h, score;
		if (sscanf(line, "box=%f,%f,%f,%f,%f", &x, &y, &w, &h, &score) == 5) {
			boxes[box_count][0] = x;
			boxes[box_count][1] = y;
			boxes[box_count][2] = w;
			boxes[box_count][3] = h;
			boxes[box_count][4] = score;
			box_count++;
		}
		line = strtok_r(NULL, "\r\n", &saveptr);
	}

	err = build_json((const float (*)[5])boxes, box_count,
					 json_out, json_len, summary);
	if (err != ESP_OK) {
		ESP_LOGW(TAG, "Respuesta demasiado grande para el buffer JSON");
		return err;
	}

	ESP_LOGI(TAG, "DetecciÃƒÂ³n: %u persona(s), confianza media %.2f",
			 (unsigned)box_count,
			 summary != NULL ? summary->confidence_avg : 0.0f);
	return ESP_OK;
}

/*==================[end of file]============================================*/
