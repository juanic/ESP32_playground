#ifndef WIFI_HAL_H
#define WIFI_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup wifi_hal Wi-Fi HAL
 *  @brief Wi-Fi station driver for ESP32.
 *  @{
 *
 * @section genDesc General Description
 *
 * This driver initializes the ESP-IDF Wi-Fi stack in station mode, connects
 * to the configured access point and exposes the assigned IPv4 address.
 * Reconnection is handled automatically after a disconnection.
 *
 * The component is only compiled when `CONFIG_WIFI_HAL_ENABLED` is set.
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
#include <stddef.h>
#include "esp_err.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Initialize the Wi-Fi stack and start a station connection.
 *
 * The function returns as soon as the connection process starts. Use
 * wifi_hal_wait_connected() to wait until an IP address is assigned.
 *
 * @param ssid Network SSID.
 * @param password Network password (empty string for open networks).
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t wifi_hal_init_sta(const char *ssid, const char *password);

/**
 * @brief Wait until the station obtains an IP address.
 *
 * @param timeout_ms Maximum time to wait, in milliseconds.
 * @return ESP_OK if connected, ESP_ERR_TIMEOUT otherwise.
 */
esp_err_t wifi_hal_wait_connected(uint32_t timeout_ms);

/**
 * @brief Get the current IPv4 address as a string.
 *
 * @param ip_str Destination buffer.
 * @param len Destination buffer length.
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if the station has no IP.
 */
esp_err_t wifi_hal_get_ip(char *ip_str, size_t len);

/**
 * @brief Check whether the station is currently connected.
 *
 * @return true if an IP address has been assigned.
 */
bool wifi_hal_is_connected(void);

/**
 * @brief Start SNTP time synchronization.
 *
 * Uses the system `time()` clock once a reply is received. The time zone is
 * configured by the application with `setenv("TZ", ...)`.
 *
 * @param server NTP server name, or NULL to use "pool.ntp.org".
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if already started.
 */
esp_err_t wifi_hal_start_sntp(const char *server);

/**
 * @brief Check whether the system clock has been synchronized by SNTP.
 */
bool wifi_hal_time_synced(void);

/** @} */
/** @} */

#endif /* #ifndef WIFI_HAL_H */

/*==================[end of file]============================================*/
