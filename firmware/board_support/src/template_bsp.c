/**
 * @file template_bsp.c
 * @author Nombre Apellido
 * @brief BSP driver template for an external component.
 * @version 0.1
 * @date AAAA-MM-DD
 *
 * @copyright Copyright (c) AAAA
 *
 */

/*==================[inclusions]=============================================*/
#include "template_bsp.h"
/* TODO: Include board_config.h and any HAL headers needed to access hardware resources.
 * BSP drivers must only use HAL/Board functions, never ESP-IDF APIs directly.
 * Examples:
 *   #include "board_config.h"
 *   #include "gpio_hal.h"
 */
/*==================[macros and definitions]=================================*/
/* TODO: Map the physical pins defined in board_config.h to local names if needed.
 * Example:
 *   #define SENSOR_GPIO_DATA    GPIO_SENSOR_DATA   // Defined in board_config.h
 *   #define SENSOR_GPIO_ENABLE  GPIO_SENSOR_ENABLE // Defined in board_config.h
 */

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

uint8_t TemplateBSPInit(void){
	/* TODO: Implement component initialization using HAL function calls.
	 * This function should:
	 *   1. Configure the required GPIO or peripheral via HAL
	 *   2. Set the component to a known initial state
	 *   3. Return true on success, false on failure
	 *
	 * Example:
	 *   GPIOInit(SENSOR_GPIO_DATA, GPIO_INPUT);
	 *   GPIOInit(SENSOR_GPIO_ENABLE, GPIO_OUTPUT);
	 *   GPIOOff(SENSOR_GPIO_ENABLE);
	 */
	return true;
}

/*==================[end of file]============================================*/
