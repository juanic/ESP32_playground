/*! @mainpage Touch
 *
 * @section genDesc General Description
 *
 * This example reads two capacitive touch pads of the ESP32-S3 (touch_hal) and
 * mirrors their state on two LEDs (board_support/led). The filtered value of
 * each pad is also printed periodically through the serial console, which is
 * useful to tune TOUCH_THRESHOLD for your own wiring/finger.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral      |   ESP32-S3   	|
 * |:-------------------|:--------------|
 * | 	Touch pad (T8)   | 	GPIO_9		|
 * | 	Touch pad (T9)   | 	GPIO_10		|
 * | 	LED_1            | 	GPIO_4		|
 * | 	LED_2            | 	GPIO_5		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 24/08/2026 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "touch_hal.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define TOUCH_BTN_1        TOUCH_PAD_8   /* GPIO_9 */
#define TOUCH_BTN_2        TOUCH_PAD_9   /* GPIO_10 */
/* Adjust these thresholds to your own wiring: on the ESP32-S3 the channel
 * value INCREASES when touched, so log the untouched value with a finger away
 * from the pad and set the threshold a little above it */
#define TOUCH_THRESHOLD    500
#define LOOP_PERIOD_MS     100
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();

    TouchHalInit();
    TouchHalChannelConfig(TOUCH_BTN_1, TOUCH_THRESHOLD);
    TouchHalChannelConfig(TOUCH_BTN_2, TOUCH_THRESHOLD);
    TouchHalStart();

    while(true){
        bool touched_1 = TouchHalIsTouched(TOUCH_BTN_1);
        bool touched_2 = TouchHalIsTouched(TOUCH_BTN_2);

        touched_1 ? LedOn(LED_1) : LedOff(LED_1);
        touched_2 ? LedOn(LED_2) : LedOff(LED_2);

        uint32_t value_1 = 0;
        uint32_t value_2 = 0;
        if(TouchHalRead(TOUCH_BTN_1, &value_1) && TouchHalRead(TOUCH_BTN_2, &value_2)){
            printf("T8 (GPIO9): %4" PRIu32 " %s | T9 (GPIO10): %4" PRIu32 " %s\n",
                   value_1, touched_1 ? "[TOUCHED]" : "         ",
                   value_2, touched_2 ? "[TOUCHED]" : "         ");
        }

        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}
/*==================[end of file]============================================*/
