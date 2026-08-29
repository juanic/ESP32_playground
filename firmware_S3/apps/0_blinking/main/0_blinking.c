/*! @mainpage Blinking
 *
 * \section genDesc General Description
 *
 * This example makes LED_1, LED_2 and LED_3 blink in a sequence.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 18/06/2026 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "animation.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_STEP_PERIOD 500
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
    // Initialize the animation middleware (which in turn initializes BSP and HAL)
    AnimationInit();
    
    animation_t current_anim = ANIMATION_CHASE;
    uint32_t step_counter = 0;

    while(true){
        // Execute one step of the current animation sequence
        AnimationRunStep(current_anim);
        
        vTaskDelay(CONFIG_STEP_PERIOD / portTICK_PERIOD_MS);
        
        step_counter++;
        
        // Every 15 steps, switch to the next animation pattern
        if (step_counter >= 15) {
            step_counter = 0;
            current_anim = (current_anim + 1) % ANIMATION_COUNT;
            printf("Cambiando a la siguiente secuencia de animación...\n");
        }
    }
}
/*==================[end of file]============================================*/
