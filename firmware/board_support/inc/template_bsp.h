#ifndef TEMPLATE_BSP_H
#define TEMPLATE_BSP_H

/** @defgroup bsp BSP
 *  @brief Board support package layer.
 *  @{
 *  @defgroup template_bsp Template BSP
 *  @brief Driver for external component.
 *  @{
 *
 * @section genDesc General Description
 * 
 * This file is the template for developing a new Board Support Package (BSP)
 * driver. It abstracts an external component (sensor, display, actuator, etc.)
 * connected to the board, using the HAL layer for hardware access.
 *
 * @section Hardware_connection Hardware connection:
 *
 * |    Component Pin   |    ESP32 GPIO     |
 * |:------------------:|:-----------------:|
 * |       PIN_X        |      GPIO_X       |
 *
 * @author Nombre Apellido
 *
 * @section changelog
 *
 * |   Date     | Description                                            |
 * |:----------:|:-------------------------------------------------------|
 * | DD/MM/AAAA | Document creation                                      |
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief TODO: Define the data types needed for this BSP driver.
 *
 * Examples:
 *   - An enum of device channels or operating modes.
 *   - A struct representing a measurement or a configuration.
 */

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Initialize the external component.
 *
 * TODO: Describe what this function configures and what initial state
 * the component is left in after initialization.
 *
 * @return uint8_t true if initialization was successful, false otherwise.
 */
uint8_t TemplateBSPInit(void);

/**
 * @brief TODO: Add the remaining public functions of this driver.
 *
 * Follow the naming convention: ModuleNameAction()
 * Examples: SensorRead(), DisplayWrite(), ButtonIsPressed()
 */

/** @} */
/** @} */

#endif /* #ifndef TEMPLATE_BSP_H */

/*==================[end of file]============================================*/
