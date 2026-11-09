/*! @file http_server.c
 *  @brief HTTP server implementation for file uploads.
 *  @defgroup http_server impl.
 *  @{
 *
 * Implementa un servidor HTTP simple usando la librería esp_http_server
 * de ESP-IDF. Proporciona un endpoint de upload que escribe el cuerpo
 * de la petición en el sistema de archivos (microSD montada).
 *
 * El servidor está diseñado para recibir archivos de corte de láser
 * (generalmente de pocos MB) y guardarlos en la tarjeta SD.
 */

#include "http_server.h"
#include "sd_hal.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_vfs_fat.h"
#include "string.h"
#include "stdio.h"
#include "esp_spi_flash.h"

/* Tag para logging */
static const char *TAG = "http_server";

/* Static variables */
static httpd_handle_t server = NULL;
static char base_path[64] = "/sdcard";

/* Endpoint URI for file upload */
#define UPLOAD_URI "/upload"

/* Max size of an upload (10 MB) */
#define MAX_UPLOAD_SIZE (10 * 1024 * 1024)

static esp_err_t upload_post_handler(httpd_req_t *req) {
    char buf[1024];
    int ret = 0;
    uint32_t received = 0;
    FILE *fd = NULL;

    /* Get content-length from the request headers */
    size_t content_len = httpd_req_get_content_length(req);

    if (content_len > MAX_UPLOAD_SIZE) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    /* Open file for writing */
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "%s/%s", base_path, "upload.bin");
    fd = fopen(filepath, "w");
    if (fd == NULL) {
        httpd_resp_send_500(req, "Failed to open file");
        return ESP_FAIL;
    }

    /* Read the data from the request and write to file */
    while (ret >= 0 && received < content_len) {
        ret = httpd_req_recv(req, buf, MIN(content_len - received, sizeof(buf)));
        if (ret > 0) {
            fwrite(buf, 1, ret, fd);
            received += ret;
        }
    }

    fclose(fd);
    /* Send response */
    httpd_resp_set_type(req, "application/octet-stream");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/*------------------------------------------------------------------
 * Inicializar y arrancar el servidor HTTP
 *------------------------------------------------------------------*/
esp_err_t http_server_start(http_server_config_t *config) {
    if (config != NULL && config->base_path != NULL) {
        snprintf(base_path, sizeof(base_path), "%s", config->base_path);
    }

    httpd_config_t config_http = HTTPD_DEFAULT_CONFIG();
    config_http.server_port = 80;
    config_http.uri_match_fn = httpd_uri_match_wildcard;

    /* Iniciar el servidor HTTP */
    server = httpd_start(&config_http);
    if (server == NULL) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return ESP_FAIL;
    }

    /* Registrar el handler de upload */
    httpd_uri_t upload_uri = {
        .uri       = UPLOAD_URI,
        .method    = HTTP_POST,
        .handler   = upload_post_handler,
        .user_ctx  = NULL
    };
    httpd_uri_register(server, &upload_uri);

    ESP_LOGI(TAG, "HTTP server started on port %d", config_http.server_port);
    return ESP_OK;
}

/*------------------------------------------------------------------
 * Detener el servidor HTTP
 *------------------------------------------------------------------*/
esp_err_t http_server_stop(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "HTTP server stopped");
        return ESP_OK;
    }
    return ESP_FAIL;
}

/*------------------------------------------------------------------
 * Manejo de peticiones HTTP
 *------------------------------------------------------------------*/
esp_err_t http_server_handle_request(const char *method, const char *path,
                                     const uint8_t *data, size_t len) {
    /* Basic handling - delegate to the URI handlers */
    if (server == NULL) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

/*==================[end of file]============================================*/