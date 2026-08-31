#ifndef PWM_HAL_H
#define PWM_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup pwm_hal PWM HAL
 *  @brief PWM driver for ESP32 (LEDC peripheral).
 *  @{
 *
 * @section genDesc General Description
 *
 * This driver wraps the ESP-IDF LEDC peripheral and exposes a simple,
 * hardware-independent PWM interface.  Up to 4 independent channels can be
 * configured simultaneously, each bound to a different GPIO.
 *
 * @note The driver uses LEDC Low-Speed mode and Timer 0 with 13-bit resolution
 *       (0–8191 steps).  The default frequency is 5 000 Hz but can be changed
 *       at initialisation time.
 *
 * @author Juan Ignacio
 *
 * @section changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 26/08/2026 | Document creation		                         |
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
#include "gpio_hal.h"
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialise a PWM channel on the given GPIO.
 *
 * Configures the LEDC timer (once) and binds a free LEDC channel to the
 * requested GPIO.  The output starts with duty = 0 (off).
 *
 * @param gpio   GPIO pin to output the PWM signal on.
 * @param freq_hz PWM frequency in Hertz (e.g. 5000).
 * @return true on success, false if no free channel is available or the
 *         GPIO is invalid.
 */
uint8_t PwmHalInit(gpio_t gpio, uint32_t freq_hz);

/**
 * @brief Set the duty cycle of a previously initialised PWM channel.
 *
 * @param gpio         GPIO pin whose duty cycle is to be changed.
 * @param duty_percent Duty cycle in percent (0–100).  Values above 100 are
 *                     clamped to 100.
 * @return true on success, false if the GPIO has not been initialised.
 */
uint8_t PwmHalSetDuty(gpio_t gpio, uint8_t duty_percent);

/**
 * @brief Read the current duty cycle of a PWM channel.
 *
 * @param gpio GPIO pin to query.
 * @return Current duty cycle in percent (0–100), or 0 if the GPIO has not
 *         been initialised.
 */
uint8_t PwmHalGetDuty(gpio_t gpio);

/**
 * @brief Turn off the PWM output (set duty to 0 %).
 *
 * @param gpio GPIO pin to turn off.
 * @return true on success, false if the GPIO has not been initialised.
 */
uint8_t PwmHalOff(gpio_t gpio);

/**
 * @brief Turn on the PWM output at full brightness (set duty to 100 %).
 *
 * @param gpio GPIO pin to turn on.
 * @return true on success, false if the GPIO has not been initialised.
 */
uint8_t PwmHalOn(gpio_t gpio);

/** @} */
/** @} */

#endif /* #ifndef PWM_HAL_H */

/*==================[end of file]============================================*/
