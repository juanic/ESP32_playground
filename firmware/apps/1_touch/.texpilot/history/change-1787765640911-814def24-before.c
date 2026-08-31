/*! @mainpage Touch
 *
 * @section genDesc General Description
 *
 * This example reads six capacitive touch pads of the ESP32 (touch_hal) and
 * mirrors the state of pads T8 and T9 on two LEDs (board_support/led). The
 * filtered value of each pad is also printed periodically through the serial
 * console, which is useful to tune CONFIG_TOUCH_THRESHOLD for your own
 * wiring/finger.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral      |   ESP32   	|
 * |:-------------------|:--------------|
 * | 	Touch pad (T4)   | 	GPIO_13		|
 * | 	Touch pad (T5)   | 	GPIO_12		|
 * | 	Touch pad (T6)   | 	GPIO_14		|
 * | 	Touch pad (T7)   | 	GPIO_27		|
 * | 	Touch pad (T8)   | 	GPIO_33		|
 * | 	Touch pad (T9)   | 	GPIO_32		|
 * | 	LED_1            | 	GPIO_2		|
 * | 	LED_2            | 	GPIO_5		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 24/08/2026 | Document creation		                         |
 * | 26/08/2026 | Added touch pads T4, T5, T6 and T7             |
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
#include "pwm_hal.h"
/*==================[macros and definitions]=================================*/
#define PLOT
#define TOUCH_BTN_1        TOUCH_PAD_8   /* GPIO_33 */
#define TOUCH_BTN_2        TOUCH_PAD_9   /* GPIO_32 */
#define TOUCH_BTN_3        TOUCH_PAD_5   /* GPIO_12 */
#define TOUCH_BTN_4        TOUCH_PAD_4   /* GPIO_13 */
#define TOUCH_BTN_5        TOUCH_PAD_6   /* GPIO_14 */
#define TOUCH_BTN_6        TOUCH_PAD_7   /* GPIO_27 */

#define TOUCH_PAD_COUNT    6
#define CALIB_SAMPLES      10            /* Number of baseline readings */
#define CALIB_MARGIN       500           /* Threshold = baseline - margin */
#define LOOP_PERIOD_MS     100
/*==================[internal data definition]===============================*/
static const touch_t touch_pads[TOUCH_PAD_COUNT] = {
    TOUCH_BTN_1, TOUCH_BTN_2, TOUCH_BTN_3,
    TOUCH_BTN_4, TOUCH_BTN_5, TOUCH_BTN_6
};

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();
    PwmHalInit(GPIO_2, 5000);
    PwmHalSetDuty(GPIO_2, 0);

    /* --- Touch init: use threshold = 0 so nothing triggers during calibration --- */
    TouchHalInit();
    for(int i = 0; i < TOUCH_PAD_COUNT; i++){
        TouchHalChannelConfig(touch_pads[i], 0);
    }
    TouchHalStart();

    /* --- Calibration: take CALIB_SAMPLES readings per pad and average --- */
    uint32_t accum[TOUCH_PAD_COUNT] = {0};

    printf("Calibrating touch pads (%d samples)...\n", CALIB_SAMPLES);
    for(int s = 0; s < CALIB_SAMPLES; s++){
        vTaskDelay(pdMS_TO_TICKS(50));
        for(int i = 0; i < TOUCH_PAD_COUNT; i++){
            uint32_t val = 0;
            if(TouchHalRead(touch_pads[i], &val)){
                accum[i] += val;
            }
        }
    }

    for(int i = 0; i < TOUCH_PAD_COUNT; i++){
        uint32_t avg = accum[i] / CALIB_SAMPLES;
        uint32_t thresh = (avg > CALIB_MARGIN) ? (avg - CALIB_MARGIN) : 1;
        TouchHalSetThreshold(touch_pads[i], thresh);
        printf("  Pad %d: baseline=%4" PRIu32 "  threshold=%4" PRIu32 "\n", i + 1, avg, thresh);
    }
    printf("Calibration done!\n\n");

    uint8_t duty = 0;
    bool blinking = false;
    uint32_t blink_half_period_ms = 500;
    uint32_t blink_timer_ms = 0;
    bool led_state = true;

    bool last_touched_1 = false;
    bool last_touched_2 = false;
    bool last_touched_3 = false;
    bool last_touched_4 = false;
    bool last_touched_5 = false;
    bool last_touched_6 = false;

    uint32_t print_counter = 0;

    /* --- Main loop --- */
    while(true){
        bool touched_1 = TouchHalIsTouched(TOUCH_BTN_1);
        bool touched_2 = TouchHalIsTouched(TOUCH_BTN_2);
        bool touched_3 = TouchHalIsTouched(TOUCH_BTN_3);
        bool touched_4 = TouchHalIsTouched(TOUCH_BTN_4);
        bool touched_5 = TouchHalIsTouched(TOUCH_BTN_5);
        bool touched_6 = TouchHalIsTouched(TOUCH_BTN_6);

        bool press_1 = touched_1 && !last_touched_1;
        bool press_2 = touched_2 && !last_touched_2;
        bool press_3 = touched_3 && !last_touched_3;
        bool press_4 = touched_4 && !last_touched_4;
        bool press_5 = touched_5 && !last_touched_5;
        bool press_6 = touched_6 && !last_touched_6;

        last_touched_1 = touched_1;
        last_touched_2 = touched_2;
        last_touched_3 = touched_3;
        last_touched_4 = touched_4;
        last_touched_5 = touched_5;
        last_touched_6 = touched_6;

        if (press_1) {
            duty = 0;
            blinking = false;
            PwmHalOff(GPIO_2);
        } else if (press_2) {
            duty = 100;
            blinking = false;
            PwmHalOn(GPIO_2);
        } else if (press_3) {
            if (duty < 100) {
                duty += 10;
                if (duty > 100) {
                    duty = 100;
                }
            }
            if (!blinking) {
                PwmHalSetDuty(GPIO_2, duty);
            }
        } else if (press_4) {
            if (duty > 0) {
                if (duty >= 10) {
                    duty -= 10;
                } else {
                    duty = 0;
                }
            }
            if (!blinking) {
                PwmHalSetDuty(GPIO_2, duty);
            }
        } else if (press_5) {
            /* Titilar más rápido */
            if (!blinking) {
                blinking = true;
                blink_half_period_ms = 500;
                blink_timer_ms = 0;
                led_state = true;
            } else {
                if (blink_half_period_ms > 100) {
                    blink_half_period_ms -= 100;
                }
            }
        } else if (press_6) {
            /* Titilar más lento */
            if (!blinking) {
                blinking = true;
                blink_half_period_ms = 500;
                blink_timer_ms = 0;
                led_state = true;
            } else {
                if (blink_half_period_ms < 2000) {
                    blink_half_period_ms += 100;
                }
            }
        }

        /* Blink control */
        if (blinking) {
            blink_timer_ms += LOOP_PERIOD_MS;
            if (blink_timer_ms >= blink_half_period_ms) {
                blink_timer_ms = 0;
                led_state = !led_state;
            }
            if (led_state) {
                PwmHalSetDuty(GPIO_2, duty);
            } else {
                PwmHalSetDuty(GPIO_2, 0);
            }
        }

        /* Print status */
#ifdef PLOT
        uint32_t value_1 = 0, value_2 = 0, value_3 = 0;
        uint32_t value_4 = 0, value_5 = 0, value_6 = 0;
        if(TouchHalRead(TOUCH_BTN_1, &value_1) && TouchHalRead(TOUCH_BTN_2, &value_2) &&
           TouchHalRead(TOUCH_BTN_3, &value_3) && TouchHalRead(TOUCH_BTN_4, &value_4) &&
           TouchHalRead(TOUCH_BTN_5, &value_5) && TouchHalRead(TOUCH_BTN_6, &value_6)){
            printf("%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 ",%" PRIu32 "\n",
                   value_1, value_2, value_3, value_4, value_5, value_6);
        }
#else
        /* Print status every 500 ms (5 cycles of 100 ms) */
        print_counter++;
        if (print_counter >= 5) {
            print_counter = 0;
            uint32_t value_1 = 0, value_2 = 0, value_3 = 0;
            uint32_t value_4 = 0, value_5 = 0, value_6 = 0;
            if(TouchHalRead(TOUCH_BTN_1, &value_1) && TouchHalRead(TOUCH_BTN_2, &value_2) &&
               TouchHalRead(TOUCH_BTN_3, &value_3) && TouchHalRead(TOUCH_BTN_4, &value_4) &&
               TouchHalRead(TOUCH_BTN_5, &value_5) && TouchHalRead(TOUCH_BTN_6, &value_6)){
                printf("T8 (GPIO33) [APAGAR]:    %4" PRIu32 " %s | T9 (GPIO32) [ENCENDER]:  %4" PRIu32 " %s\n",
                       value_1, touched_1 ? "[TOUCHED]" : "         ",
                       value_2, touched_2 ? "[TOUCHED]" : "         ");
                printf("T5 (GPIO12) [AUMENTAR]:  %4" PRIu32 " %s | T4 (GPIO13) [DISMINUIR]: %4" PRIu32 " %s\n",
                       value_3, touched_3 ? "[TOUCHED]" : "         ",
                       value_4, touched_4 ? "[TOUCHED]" : "         ");
                printf("T6 (GPIO14) [RAPIDO]:    %4" PRIu32 " %s | T7 (GPIO27) [LENTO]:     %4" PRIu32 " %s\n",
                       value_5, touched_5 ? "[TOUCHED]" : "         ",
                       value_6, touched_6 ? "[TOUCHED]" : "         ");
                if (blinking) {
                    printf("PWM Duty (GPIO_2): %3u%% | Modo: TITILANDO (semiperiodo: %lu ms)\n\n", duty, (unsigned long)blink_half_period_ms);
                } else {
                    printf("PWM Duty (GPIO_2): %3u%% | Modo: FIJO\n\n", duty);
                }
            }
        }
#endif

        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}
/*==================[end of file]============================================*/


