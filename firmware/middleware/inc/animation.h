#ifndef ANIMATION_H
#define ANIMATION_H

/** @defgroup middleware Middleware
 *  @brief Layer of intermediate logical services.
 *  @{
 *  @defgroup animation_middleware Animation Middleware
 *  @brief LEDs animation driver.
 *  @{
 * 
 * @section genDesc General Description
 * 
 * This middleware provides a set of animation functions for the LEDs on the board.
 * It allows you to create different animation patterns for the LEDs.
 * 
 * @author Albano Peñalva
 *
 * @section changelog
 *
 * |   Date	    | Description                                    						|
 * |:----------:|:----------------------------------------------------------------------|
 * | 23/10/2023 | Document creation		                         						|
 * 
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief LED animation modes.
 */
typedef enum {
    ANIMATION_BLINK_ALL = 0,  /**< Toggle all LEDs together */
    ANIMATION_CHASE,          /**< Sequentially turn on one LED at a time */
    ANIMATION_PING_PONG,      /**< Forward and backward LED chase */
    ANIMATION_COUNT
} animation_t;
/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Initialize the animation middleware.
 * 
 * This function initializes the underlying board support layer (LEDs).
 * 
 * @return uint8_t true if initialization was successful.
 */
uint8_t AnimationInit(void);

/**
 * @brief Executes one step of the selected animation.
 * 
 * This function should be called periodically in the application loop.
 * 
 * @param anim The animation type to execute.
 */
void AnimationRunStep(animation_t anim);

/** @} */
/** @} */

#endif /* #ifndef ANIMATION_H */

/*==================[end of file]============================================*/

