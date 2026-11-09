/*! @file wifi_hal.c
 *  @brief Wi-Fi station HAL implementation for ESP-IDF 6.x.
 *  @defgroup wifi_hal impl.
 *  @{
 *
 * Inicializa la pila Wi-Fi como estación, se conecta al AP configurado y
 * reintenta automáticamente tras una desconexión. La configuración no se
 * persiste en NVS (WIFI_STORAGE_RAM) para mantener el ejemplo autocontenido.
 */

/*==================[inclusions]=============================================*/
#include "wifi_hal.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
/*==================[macros and definitions]=================================*/

#define WIFI_HAL_CONNECTED_BIT	BIT0
#define WIFI_HAL_RECONNECT_MS	2000
#define WIFI_HAL_SNTP_SERVER	"pool.ntp.org"

/*==================[internal data definition]===============================*/

static const char *TAG = "wifi_hal";
static EventGroupHandle_t s_event_group = NULL;
static esp_netif_t *s_sta_netif = NULL;
static esp_timer_handle_t s_reconnect_timer = NULL;
static bool s_connected = false;
static bool s_sntp_started = false;
static volatile bool s_time_synced = false;

/*==================[internal functions declaration]=========================*/

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
							   int32_t event_id, void *event_data);
static void ip_event_handler(void *arg, esp_event_base_t event_base,
							 int32_t event_id, void *event_data);
static void reconnect_timer_cb(void *arg);
static esp_err_t reconnect_timer_init(void);

/*==================[internal functions definition]==========================*/

static void reconnect_timer_cb(void *arg) {
	(void)arg;
	if (s_event_group != NULL) {
		esp_wifi_connect();
	}
}

static esp_err_t reconnect_timer_init(void) {
	if (s_reconnect_timer != NULL) {
		return ESP_OK;
	}

	const esp_timer_create_args_t timer_args = {
		.callback = reconnect_timer_cb,
		.name = "wifi_reconnect",
	};
	return esp_timer_create(&timer_args, &s_reconnect_timer);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
							   int32_t event_id, void *event_data) {
	(void)arg;
	(void)event_base;

	if (event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
		wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
		s_connected = false;
		if (s_event_group != NULL) {
			xEventGroupClearBits(s_event_group, WIFI_HAL_CONNECTED_BIT);
		}
		ESP_LOGW(TAG, "Wi-Fi desconectado (reason=%d), reintentando en %d ms",
				 event ? event->reason : -1, WIFI_HAL_RECONNECT_MS);
		if (s_reconnect_timer != NULL) {
			esp_timer_stop(s_reconnect_timer);
			esp_timer_start_once(s_reconnect_timer, WIFI_HAL_RECONNECT_MS * 1000ULL);
		}
	}
}

static void ip_event_handler(void *arg, esp_event_base_t event_base,
							 int32_t event_id, void *event_data) {
	(void)arg;
	(void)event_base;

	if (event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
		s_connected = true;
		if (s_event_group != NULL) {
			xEventGroupSetBits(s_event_group, WIFI_HAL_CONNECTED_BIT);
		}
		ESP_LOGI(TAG, "IP obtenida: " IPSTR, IP2STR(&event->ip_info.ip));
	}
}

/*==================[external functions definition]==========================*/

static void sntp_sync_cb(struct timeval *tv) {
	(void)tv;
	s_time_synced = true;
	ESP_LOGI(TAG, "Hora sincronizada por SNTP");
}

esp_err_t wifi_hal_start_sntp(const char *server) {
	if (s_sntp_started) {
		return ESP_ERR_INVALID_STATE;
	}

	const char *ntp_server = server != NULL ? server : WIFI_HAL_SNTP_SERVER;
	esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
	esp_sntp_setservername(0, ntp_server);
	esp_sntp_set_time_sync_notification_cb(sntp_sync_cb);
	esp_sntp_init();
	s_sntp_started = true;

	ESP_LOGI(TAG, "SNTP iniciado con '%s'", ntp_server);
	return ESP_OK;
}

bool wifi_hal_time_synced(void) {
	return s_time_synced;
}

esp_err_t wifi_hal_init_sta(const char *ssid, const char *password) {
	if (ssid == NULL || password == NULL || ssid[0] == '\0') {
		return ESP_ERR_INVALID_ARG;
	}
	if (s_event_group != NULL) {
		return ESP_ERR_INVALID_STATE;
	}

	s_event_group = xEventGroupCreate();
	if (s_event_group == NULL) {
		return ESP_ERR_NO_MEM;
	}

	esp_err_t err = reconnect_timer_init();
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "No se pudo crear el timer de reconexión: %s", esp_err_to_name(err));
		goto fail;
	}

	err = esp_netif_init();
	if (err != ESP_OK) {
		goto fail;
	}

	err = esp_event_loop_create_default();
	if (err != ESP_OK) {
		goto fail;
	}

	s_sta_netif = esp_netif_create_default_wifi_sta();
	if (s_sta_netif == NULL) {
		err = ESP_FAIL;
		goto fail;
	}

	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	err = esp_wifi_init(&wifi_init_config);
	if (err != ESP_OK) {
		goto fail;
	}

	err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
											  &wifi_event_handler, NULL, NULL);
	if (err != ESP_OK) {
		goto fail;
	}

	err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
											  &ip_event_handler, NULL, NULL);
	if (err != ESP_OK) {
		goto fail;
	}

	wifi_config_t wifi_config = {0};
	strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
	strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
	wifi_config.sta.threshold.authmode = (password[0] != '\0') ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;

	err = esp_wifi_set_storage(WIFI_STORAGE_RAM);
	if (err != ESP_OK) {
		goto fail;
	}

	err = esp_wifi_set_mode(WIFI_MODE_STA);
	if (err != ESP_OK) {
		goto fail;
	}

	err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
	if (err != ESP_OK) {
		goto fail;
	}

	err = esp_wifi_start();
	if (err != ESP_OK) {
		goto fail;
	}

	/* El streaming no tolera los ahorros de energía del Wi-Fi. */
	esp_wifi_set_ps(WIFI_PS_NONE);

	ESP_LOGI(TAG, "Conectando a '%s'...", ssid);
	return ESP_OK;

fail:
	ESP_LOGE(TAG, "wifi_hal_init_sta falló: %s", esp_err_to_name(err));
	if (s_reconnect_timer != NULL) {
		esp_timer_delete(s_reconnect_timer);
		s_reconnect_timer = NULL;
	}
	if (s_event_group != NULL) {
		vEventGroupDelete(s_event_group);
		s_event_group = NULL;
	}
	return err;
}

esp_err_t wifi_hal_wait_connected(uint32_t timeout_ms) {
	if (s_event_group == NULL) {
		return ESP_ERR_INVALID_STATE;
	}
	if (s_connected) {
		return ESP_OK;
	}

	EventBits_t bits = xEventGroupWaitBits(s_event_group, WIFI_HAL_CONNECTED_BIT,
										   pdFALSE, pdTRUE,
										   pdMS_TO_TICKS(timeout_ms));
	return (bits & WIFI_HAL_CONNECTED_BIT) ? ESP_OK : ESP_ERR_TIMEOUT;
}

esp_err_t wifi_hal_get_ip(char *ip_str, size_t len) {
	if (ip_str == NULL || len == 0) {
		return ESP_ERR_INVALID_ARG;
	}
	if (s_sta_netif == NULL || !s_connected) {
		return ESP_ERR_INVALID_STATE;
	}

	esp_netif_ip_info_t ip_info;
	esp_err_t err = esp_netif_get_ip_info(s_sta_netif, &ip_info);
	if (err != ESP_OK) {
		return err;
	}

	snprintf(ip_str, len, IPSTR, IP2STR(&ip_info.ip));
	return ESP_OK;
}

bool wifi_hal_is_connected(void) {
	return s_connected;
}

/*==================[end of file]============================================*/
