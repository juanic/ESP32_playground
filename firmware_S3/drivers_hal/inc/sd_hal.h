/*! @file sd_hal.h
 *  @brief SD card mount/unmount interface for the VFS FAT layer.
 *  @defgroup sd_hal SD HAL
 *  @{
 *
 * Proporciona funciones para montar y desmontar el sistema de archivos
 * FAT en la tarjeta SD usando la configuración VFS de ESP-IDF.
 *
 * El sistema de archivos usa la partition table entry para la SD y el
 * driver esp_vfs_fat_sdmmc. La tarjeta debe estar soldado/insertada
 * en el slot de la placa XIAO ESP32S3 Sense.
 *
 * @note Las funciones aquí están pensadas para ser llamadas desde
 * board_support o middleware, nunca directamente desde las apps.
 */

#ifndef SD_HAL_H
#define SD_HAL_H

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include <stdbool.h>
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief Result type for mount/unmount operations.
 */
typedef enum {
    SD_RESULT_OK,           /**< Operation successful */
    SD_RESULT_ERROR,        /**< Generic error */
    SD_RESULT_NOT_MOUNTED,/**< Filesystem not mounted */
    SD_RESULT_INVALID_ARG /**< Invalid argument */
} sd_result_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Initialize the SD card VFS layer.
 *
 * Configures the SPI bus and mounts the FAT filesystem on the SD card.
 * Must be called before any read/write operations.
 *
 * @return sd_result_t OK if successful, error otherwise.
 */
sd_result_t sd_mount(void);

/**
 * @brief Unmount the SD card filesystem.
 *
 * Safely unmounts the FAT filesystem and releases the SD card resources.
 *
 * @return sd_result_t OK if successful, error otherwise.
 */
sd_result_t sd_unmount(void);

/** @} */
#endif /* #ifndef SD_HAL_H */

/*==================[end of file]============================================*/