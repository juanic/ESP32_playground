/*! @file sd_hal.c
 *  @brief SD card HAL implementation for ESP-IDF 6.x.
 *  @defgroup sd_hal impl.
 *  @{
 *
 * Monta la tarjeta SD sobre el host SDMMC del ESP32 y la expone en VFS.
 * Todo el archivo se compila sólo si `CONFIG_SD_HAL_ENABLED` está activo.
 */

/*==================[inclusions]=============================================*/
#include "sd_hal.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <string.h>
#include <sys/stat.h>
/*==================[macros and definitions]=================================*/

#define SD_HAL_MAX_FILES			8
#define SD_HAL_ALLOC_UNIT			(16 * 1024)

/*==================[internal data definition]===============================*/

static const char *TAG = "sd_hal";
static sdmmc_card_t *s_card = NULL;
static const char *s_mount_point = NULL;

/*==================[external functions definition]==========================*/

esp_err_t sd_hal_mount(const sd_hal_config_t *config) {
	if (config == NULL || config->mount_point == NULL) {
		return ESP_ERR_INVALID_ARG;
	}
	if (s_card != NULL) {
		return ESP_ERR_INVALID_STATE;
	}

	sdmmc_host_t host = SDMMC_HOST_DEFAULT();
	host.slot = SDMMC_HOST_SLOT_1;
	host.max_freq_khz = SDMMC_FREQ_DEFAULT;
	host.flags = config->one_bit ? SDMMC_HOST_FLAG_1BIT : SDMMC_HOST_FLAG_4BIT;

	sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
	slot_config.width = config->one_bit ? 1 : 4;
	slot_config.clk = (gpio_num_t)config->pin_clk;
	slot_config.cmd = (gpio_num_t)config->pin_cmd;
	slot_config.d0 = (gpio_num_t)config->pin_d0;
	if (!config->one_bit) {
		slot_config.d1 = (gpio_num_t)config->pin_d1;
		slot_config.d2 = (gpio_num_t)config->pin_d2;
		slot_config.d3 = (gpio_num_t)config->pin_d3;
	}
	slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

	const esp_vfs_fat_mount_config_t mount_config = {
		.format_if_mount_failed = false,
		.max_files = SD_HAL_MAX_FILES,
		.allocation_unit_size = SD_HAL_ALLOC_UNIT,
	};

	esp_err_t err = esp_vfs_fat_sdmmc_mount(config->mount_point, &host,
											&slot_config, &mount_config, &s_card);
	if (err != ESP_OK) {
		if (err == ESP_FAIL) {
			ESP_LOGE(TAG, "No se pudo montar el sistema de archivos; "
					 "formateá la tarjeta como FAT32");
		} else {
			ESP_LOGE(TAG, "No se pudo inicializar la tarjeta SD: %s",
					 esp_err_to_name(err));
		}
		s_card = NULL;
		return err;
	}

	s_mount_point = config->mount_point;

	const sdmmc_card_t *card = s_card;
	ESP_LOGI(TAG, "SD montada en %s: %s %u MB (%s)",
			 s_mount_point, card->cid.name,
			 (unsigned)((uint64_t)card->csd.capacity * card->csd.sector_size / (1024 * 1024)),
			 config->one_bit ? "1-bit" : "4-bit");
	return ESP_OK;
}

esp_err_t sd_hal_unmount(void) {
	if (s_card == NULL || s_mount_point == NULL) {
		return ESP_ERR_INVALID_STATE;
	}

	esp_err_t err = esp_vfs_fat_sdcard_unmount(s_mount_point, s_card);
	if (err == ESP_OK) {
		s_card = NULL;
		s_mount_point = NULL;
		ESP_LOGI(TAG, "SD desmontada");
	}
	return err;
}

bool sd_hal_is_mounted(void) {
	return s_card != NULL;
}

const char *sd_hal_mount_point(void) {
	return s_mount_point;
}

uint64_t sd_hal_total_bytes(void) {
	if (s_card == NULL) {
		return 0;
	}
	return (uint64_t)s_card->csd.capacity * s_card->csd.sector_size;
}

uint64_t sd_hal_free_bytes(void) {
	if (s_mount_point == NULL) {
		return 0;
	}

	uint64_t total = 0;
	uint64_t free_bytes = 0;
	if (esp_vfs_fat_info(s_mount_point, &total, &free_bytes) != ESP_OK) {
		return 0;
	}
	return free_bytes;
}

/*==================[end of file]============================================*/
