/*! @file http_server.h
 *  @brief HTTP server interface for file uploads.
 *  @defgroup http_server HTTP server
 *  @{
 *
 * Proporciona una API mínima para iniciar un servidor HTTP simple con
 * un endpoint de upload que escribe archivos en la microSD montada.
 * El servidor usa la librería esp_http_server de ESP-IDF.
 *
 * @note Las funciones aquí están pensadas para ser llamadas desde
 * middleware, nunca directamente desde las apps.
 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include "esp_err.h"
/*==================[macros]=================================================*/
#define HTTP_TASK_STACK_SIZE     (8 * 1024)  /* 8KB stack for HTTP server task */
#define HTTP_QUEUE_SIZE        8             /* Queue size for incoming HTTP requests */

/*==================[typedef]================================================*/
/**
 * @brief HTTP server configuration.
 */
typedef struct {
    char *base_path;          /*!< Base path for file storage (SD card) */
    uint16_t port;          /*!< Server port (typically 80) */
    bool enable_ssl;        /*!< Enable HTTPS (not used in this app) */
} http_server_config_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Initialize and start the HTTP server.
 *
 * Configura y arranca el servidor HTTP en el puerto especificado,
 * usando base_path como raíz de almacenamiento (generalmente la SD).
 *
 * @param config Puntero a la estructura de configuración.
 * @return esp_err_t ESP_OK si el servidor se inició correctamente,
 *                 otro valor en caso de error.
 */
esp_err_t http_server_start(http_server_config_t *config);

/**
 * @brief Stop the HTTP server.
 *
 * Detiene el servidor HTTP y libera los recursos asociados.
 *
 * @return esp_err_t ESP_OK si el servidor se detuvo correctamente.
 */
esp_err_t http_server_stop(void);

/**
 * @brief Handle an incoming HTTP request.
 *
 * Función de callback que procesa una solicitud HTTP entrante.
 * Se usa específicamente para el endpoint de upload de archivos.
 *
 * @param method Método HTTP (GET, POST, etc.).
 * @param path   Ruta del endpoint.
 * @param data   Datos del cuerpo de la petición (para POST).
 * @param len    Longitud de los datos.
 * @return esp_err_t ESP_OK si se manejó la petición, error en otro caso.
 */
esp_err_t http_server_handle_request(const char *method, const char *path,
                                     const uint8_t *data, size_t len);

/** @} */
#endif /* #ifndef HTTP_SERVER_H */

/*==================[end of file]============================================*/