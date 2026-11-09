#ifndef SD_HAL_H
#define SD_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup sd_hal SD HAL
 *  @brief SD card mounting for the ESP32 SDMMC host.
 *  @{
 *
 * @section genDesc General Description
 *
 * Mounts a FAT filesystem on an SD card connected to the ESP32 SDMMC host
 * and exposes it through the VFS layer, so regular POSIX calls (`fopen`,
 * `mkdir`, `opendir`, ...) work on the mount point.
 *
 * The pin assignment belongs to the board support layer (BSP); this HAL only
 * knows about the SDMMC host. One-line (1-bit) mode is supported because the
 * AI-Thinker ESP32-CAM shares the other data lines with strapping pins and
 * the flash LED.
 *
 * The component is only compiled when `CONFIG_SD_HAL_ENABLED` is set.
 *
 * @section changelog Changelog
 *
 * |   Date     | Description                                            |
 * |:----------:|:-------------------------------------------------------|
 * | 09/11/2026 | Document creation                                      |
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/**
 * @brief SDMMC pin configuration.
 *
 * Pins set to -1 are not used. When `one_bit` is true only CLK, CMD and D0
 * are configured; D1..D3 are ignored.
 */
typedef struct {
	int pin_clk;				/**< Clock pin */
	int pin_cmd;				/**< Command pin */
	int pin_d0;					/**< Data line 0 */
	int pin_d1;					/**< Data line 1 (-1 in 1-bit mode) */
	int pin_d2;					/**< Data line 2 (-1 in 1-bit mode) */
	int pin_d3;					/**< Data line 3 (-1 in 1-bit mode) */
	bool one_bit;				/**< Use 1-line SDMMC mode */
	const char *mount_point;	/**< VFS mount point, e.g. "/sdcard" */
} sd_hal_config_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Mount the SD card FAT filesystem.
 *
 * @param config Pin assignment and mount point.
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if config is NULL,
 *         ESP_ERR_INVALID_STATE if a card is already mounted, or the error
 *         reported by esp_vfs_fat_sdmmc_mount().
 */
esp_err_t sd_hal_mount(const sd_hal_config_t *config);

/**
 * @brief Unmount the SD card filesystem.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if no card is mounted.
 */
esp_err_t sd_hal_unmount(void);

/**
 * @brief Check whether a card is currently mounted.
 */
bool sd_hal_is_mounted(void);

/**
 * @brief Get the mount point of the mounted card.
 *
 * @return Mount point string, or NULL if no card is mounted.
 */
const char *sd_hal_mount_point(void);

/**
 * @brief Get total capacity in bytes.
 *
 * @return Capacity in bytes, or 0 if no card is mounted.
 */
uint64_t sd_hal_total_bytes(void);

/**
 * @brief Get free space in bytes.
 *
 * @return Free space in bytes, or 0 if no card is mounted.
 */
uint64_t sd_hal_free_bytes(void);

/** @} */
/** @} */

#endif /* #ifndef SD_HAL_H */

/*==================[end of file]============================================*/
