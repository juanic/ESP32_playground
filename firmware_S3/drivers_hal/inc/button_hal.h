/*! @file button_hal.h
 *  @brief GPIO button driver with software antirrebote.
 *  @defgroup button_hal Button HAL
 *  @{
 *
 * Driver para leer un botón físico conectado a un GPIO del ESP32-S3
 * con detección de flanco de borde y filtrado antirrebote en tiempo
 * de software. El botón se configura con pull-up interno.
 *
 * @note Las funciones aquí están pensadas para ser llamadas desde
 * board_support o middleware, nunca directamente desde las apps.
 */

#ifndef BUTTON_HAL_H
#define BUTTON_HAL_H

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include <stdbool.h>
/*==================[macros]=================================================*/
#define BTN_DEBOUNCE_MS     20  /* tiempo de antirrebote en ms */

/*==================[typedef]================================================*/
/**
 * @brief Estado del botón.
 */
typedef enum {
    BTN_STATE_RELEASED,  /**< Botón en reposo (no presionado) */
    BTN_STATE_PRESSED    /**< Botón presionado */
} btn_state_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Initialize the button GPIO.
 *
 * Configura el GPIO del botón como entrada con pull-up interno y
 * prepara la estructura de estado para el antirrebote.
 */
void btn_hal_init(void);

/**
 * @brief Leer el estado actual del botón (con antirrebote).
 *
 * @return btn_state_t BTN_STATE_PRESSED si el botón está presionado,
 *                     BTN_STATE_RELEASED en otro caso.
 */
btn_state_t btn_hal_read(void);

/** @} */
#endif /* #ifndef BUTTON_HAL_H */

/*==================[end of file]============================================*/