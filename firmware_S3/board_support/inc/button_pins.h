/*! @file button_pins.h
 *  @brief Button pin definitions for XIAO ESP32S3 Sense.
 *  @defgroup button_pins Button pin definitions
 *  @{
 *
 * Definiciones del botón físico conectado al ESP32-S3 Sense.
 * Los números de GPIO deben confirmarse contra la documentación oficial
 * de Seeed Studio. Se usa pull-up interno en el software.
 *
 * @note Esta capa puede ser llamada por middleware.
 */

#ifndef BUTTON_PINS_H
#define BUTTON_PINS_H

/*==================[inclusions]=============================================*/
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief Button pin configuration.
 */
typedef struct {
    gpio_num_t gpio;    /*!< GPIO number for the button */
    bool active_low;    /*!< True if button is active low (pull-up) */
} btn_pin_config_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Get the default button pin configuration for XIAO ESP32S3 Sense.
 *
 * @return btn_pin_config_t Default pin configuration (TODO: confirm GPIO number).
 */
btn_pin_config_t btn_get_default_pins(void);

/** @} */
#endif /* #ifndef BUTTON_PINS_H */

/*==================[end of file]============================================*/