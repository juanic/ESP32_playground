/*! @file sd_pins.h
 *  @brief SD card pin definitions for XIAO ESP32S3 Sense.
 *  @defgroup sd_pins SD pin definitions
 *  @{
 *
 * Definiciones de los pines de la tarjeta SD conectada a la placa
 * XIAO ESP32S3 Sense. Los números de GPIO deben confirmarse contra
 * la documentación oficial de Seeed Studio antes de usarse en
 * producción, por lo que están marcados como TODO.
 *
 * @note Esta capa puede ser llamada por drivers_hal.
 */

#ifndef SD_PINS_H
#define SD_PINS_H

/*==================[inclusions]=============================================*/
/* TODO: Include any necessary ESP-IDF headers if needed */
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief SD card bus pin configuration.
 */
typedef struct {
    gpio_num_t cs;      /*!< Chip Select (CS) GPIO */
    gpio_num_t clk;     /*!< Clock (CLK) GPIO */
    gpio_num_t cmd;     /*!< Command (CMD) GPIO */
    gpio_num_t d0;      /*!< Data 0 (D0) GPIO */
    gpio_num_t d1;      /*!< Data 1 (D1) GPIO (optional, SPI mode) */
    gpio_num_t d2;      /*!< Data 2 (D2) GPIO (optional, SPI mode) */
    gpio_num_t d3;      /*!< Data 3 (D3) GPIO (optional, SPI mode) */
} sd_pin_config_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Get the default SD pin configuration for XIAO ESP32S3 Sense.
 *
 * @return sd_pin_config_t Default pin configuration (TODO: confirm GPIO numbers).
 */
sd_pin_config_t sd_get_default_pins(void);

/** @} */
#endif /* #ifndef SD_PINS_H */

/*==================[end of file]============================================*/