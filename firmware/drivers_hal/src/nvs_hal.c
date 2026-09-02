/**
 * @file nvs_hal.c
 * @brief NVS HAL driver for ESP-IDF 6.x.
 */

/*==================[inclusions]=============================================*/
#include "nvs_hal.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
static const char *TAG = "nvs_hal";
static bool s_nvs_initialized = false;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

bool NvsHalInit(void) {
    if (s_nvs_initialized) {
        return true;
    }

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(ret));
        return false;
    }

    s_nvs_initialized = true;
    ESP_LOGI(TAG, "NVS initialized");
    return true;
}

bool NvsHalSetBlob(const char *namespace, const char *key,
                   const void *data, size_t len) {
    if (!s_nvs_initialized || !namespace || !key || !data) {
        return false;
    }

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(ret));
        return false;
    }

    ret = nvs_set_blob(handle, key, data, len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_blob failed: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return false;
    }

    ret = nvs_commit(handle);
    nvs_close(handle);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(ret));
        return false;
    }
    return true;
}

bool NvsHalGetBlob(const char *namespace, const char *key,
                   void *data, size_t *len) {
    if (!s_nvs_initialized || !namespace || !key || !data || !len) {
        return false;
    }

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(namespace, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        return false;
    }

    ret = nvs_get_blob(handle, key, data, len);
    nvs_close(handle);

    return (ret == ESP_OK);
}

bool NvsHalSetStr(const char *namespace, const char *key, const char *value) {
    if (!s_nvs_initialized || !namespace || !key || !value) {
        return false;
    }

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(ret));
        return false;
    }

    ret = nvs_set_str(handle, key, value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_str failed: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return false;
    }

    ret = nvs_commit(handle);
    nvs_close(handle);

    return (ret == ESP_OK);
}

bool NvsHalGetStr(const char *namespace, const char *key,
                  char *value, size_t max_len) {
    if (!s_nvs_initialized || !namespace || !key || !value || max_len == 0) {
        return false;
    }

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(namespace, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        return false;
    }

    ret = nvs_get_str(handle, key, value, &max_len);
    nvs_close(handle);

    return (ret == ESP_OK);
}

bool NvsHalEraseKey(const char *namespace, const char *key) {
    if (!s_nvs_initialized || !namespace || !key) {
        return false;
    }

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(namespace, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        return false;
    }

    ret = nvs_erase_key(handle, key);
    if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(handle);
        return false;
    }

    ret = nvs_commit(handle);
    nvs_close(handle);

    return (ret == ESP_OK);
}

/*==================[end of file]============================================*/
