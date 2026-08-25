/**
 * @file animation.c
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 * @brief 
 * @version 0.1
 * @date 2023-10-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/*==================[inclusions]=============================================*/
#include "animation.h"
#include "led.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
static uint8_t current_step = 0;
static animation_t last_anim = ANIMATION_COUNT;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/

uint8_t AnimationInit(void){
    LedsInit();
    LedsOffAll();
    current_step = 0;
    last_anim = ANIMATION_COUNT;
    return true;
}

void AnimationRunStep(animation_t anim) {
    // Reset state step if we switch animations
    if (anim != last_anim) {
        current_step = 0;
        last_anim = anim;
    }

    switch (anim) {
        case ANIMATION_BLINK_ALL:
            if (current_step == 0) {
                LedsOffAll();
                current_step = 1;
            } else {
                LedsMask(LED_1 | LED_2 | LED_3);
                current_step = 0;
            }
            break;

        case ANIMATION_CHASE:
            LedsOffAll();
            if (current_step == 0) {
                LedOn(LED_1);
                current_step = 1;
            } else if (current_step == 1) {
                LedOn(LED_2);
                current_step = 2;
            } else {
                LedOn(LED_3);
                current_step = 0;
            }
            break;

        case ANIMATION_PING_PONG:
            LedsOffAll();
            if (current_step == 0) {
                LedOn(LED_1);
                current_step = 1;
            } else if (current_step == 1) {
                LedOn(LED_2);
                current_step = 2;
            } else if (current_step == 2) {
                LedOn(LED_3);
                current_step = 3;
            } else {
                LedOn(LED_2);
                current_step = 0;
            }
            break;

        default:
            LedsOffAll();
            break;
    }
}

/*==================[end of file]============================================*/
