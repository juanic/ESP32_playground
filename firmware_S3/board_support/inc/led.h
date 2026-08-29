#ifndef LED_H
#define LED_H

/** @defgroup bsp BSP
 *  @brief Board support package layer.
 *  @{
 *  @defgroup led_bsp LED BSP
 *  @brief LEDs driver for the development board.
 *  @{
 *
 * @section genDesc General Description
 * 
 * Custom board have 3 LEDs LED_1: green, LED_2: yellow, LED_3: red
 * 
 * @section Hardware_connection Hardware connection:
 * 
 * |    Component Pin   |    ESP32 GPIO     |
 * |:------------------:|:-----------------:|
 * |       LED_3        |      GPIO_18      |
 * |       LED_2        |      GPIO_5       |
 * |       LED_1        |      GPIO_2       |
 * 
 * @author Juan Cerrudo
 *
 * @section changelog
 *
 * |   Date	    | Description                                    						|
 * |:----------:|:----------------------------------------------------------------------|
 * | 23/06/2026 | Document creation		                         						|
 * | 24/08/2026 | Ported from ESP32-C6 to ESP32 (LED_3 moved to GPIO_18)					|
 * 
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
/*==================[macros]=================================================*/
/**
 * @brief List of available LEDs in custom board.
 */
typedef enum LEDs {
    LED_3 = (1 << 0), /**< Color red. Routed to GPIO_18 */
    LED_2 = (1 << 1), /**< Color yellow. Routed to GPIO_5 */
    LED_1 = (1 << 2), /**< Color green. Routed to GPIO_4 */
} led_t;
/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief ESP-EDU LEDs initialization
 * 
 * @return uint8_t 
 */
uint8_t LedsInit(void);

/**
 * @brief Turn on a specific LED
 * 
 * @param led LED number
 * @return uint8_t false: if invalid LED number
 */
uint8_t LedOn(led_t led);

/**
 * @brief Turn off a specific LED
 * 
 * @param led LED number
 * @return uint8_t false: if invalid LED number 
 */
uint8_t LedOff(led_t led);

/**
 * @brief Invert a specific LED state
 * 
 * @param led LED number
 * @return uint8_t false: if invalid LED number 
 */
uint8_t LedToggle(led_t led);

/**
 * @brief Turn off all LEDs
 * 
 * @return uint8_t 
 */
uint8_t LedsOffAll(void);

/**
 * @brief Turn on or off leds from a mask.
 * 
 * @param mask (b0: LED_3, b1: LED_2, b2: LED_1)
 * @return uint8_t 
 */
uint8_t LedsMask(uint8_t mask);

/** @} */
/** @} */

#endif /* #ifndef LED_H */

/*==================[end of file]============================================*/

