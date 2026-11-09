/*! @file sd_hal.c
 *  @brief SD card mount/unmount implementation using esp_vfs_fat_sdmmc.
 *  @defgroup sd_hal SD HAL impl.
 *  @{
 *
 * Implementa el montaje y desmontaje del sistema de archivos FAT sobre
 * la tarjeta SD usando la API VFS de ESP-IDF (esp_vfs_fat_sdmmc).
 * El partition label de la SD debe estar definido en la configuración
 * del proyecto (CONFIG_PARTITION_SD card).

#include "sd_hal.h"
#include "driver/sdmmc_types.h"
#include "driver/sdmmc_controller.h"
#include "esp_vfs_fat.h"
#include "esp_err.h"
#include "esp_log.h"
#include "stdio.h"

static const char *TAG = "sd_hal";

static const sdmmc_host_t host = SDSPI_HOST_DEFAULT();
static mmc_card_t *card = NULL;
static bool mounted = false;

/* Mount options for fat formatting */
static const mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
    .allocation_unit_size = 16 * 1024
};

esp_err_t sd_mount_internal(void) {
    esp_vfs_fat_sddisk_config_t vfs_config = {
        .format_storage_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    esp_err_t err = esp_vfs_fat_sddisk_mount("/sdcard", "storage", &vfs_config, &card);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FAT filesystem: %s", esp_err_to_name(err));
        return err;
    }
    mounted = true;
    ESP_LOGI(TAG, "SD card mounted successfully");
    return ESP_OK;
}

esp_err_t sd_unmount_internal(void) {
    if (!mounted) {
        ESP_LOGW(TAG, "SD card not mounted");
        return SD_RESULT_NOT_MOUNTED;
    }

    esp_err_t err = esp_vfs_fat_sddisk_unmount("/sdcard", card);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to unmount FAT filesystem: %s", esp_err_to_name(err));
        return err;
    }
    mounted = false;
    ESP_LOGI(TAG, "SD card unmounted successfully");
    return ESP_OK;
}

sd_result_t sd_mount(void) {
    esp_err_t ret = sd_mount_internal();
    return (ret == ESP_OK) ? SD_RESULT_OK : SD_RESULT_ERROR;
}

sd_result_t sd_unmount(void) {
    esp_err_t ret = sd_unmount_internal();
    return (ret == ESP_OK) ? SD_RESULT_OK : SD_RESULT_ERROR;
}

void sd_record_partition(void) {
    /* TODO: leer la partition table para confirmar el label de la SD */
    ESP_LOGI(TAG, "SD partition label: storage (configurear en sdkconfig)");
}

/*==================[end of file]============================================*/