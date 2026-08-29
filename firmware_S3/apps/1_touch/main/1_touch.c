/*! @mainpage Touch
 *
 * @section genDesc General Description
 *
 * This example reads the six capacitive touch pads T4..T9 of the ESP32-S3
 * (touch_hal) and prints their filtered (smoothed) value through the serial
 * console. It is a debugging aid to observe the raw values and tune the
 * baseline / delta thresholds (used by the 4_hid_touch app) for your own
 * wiring and finger.
 *
 * On the ESP32-S3 the channel value INCREASES when the pad is touched.
 *
 * @section hardConn Hardware Connection
 *
 * |    Touch channel   |   ESP32-S3   	|
 * |:-------------------|:--------------|
 * | 	T4              | 	GPIO_5		|
 * | 	T5              | 	GPIO_6		|
 * | 	T6              | 	GPIO_7		|
 * | 	T7              | 	GPIO_8		|
 * | 	T8              | 	GPIO_9		|
 * | 	T9              | 	GPIO_10		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 24/08/2026 | Document creation		                         |
 * | 29/08/2026 | Read T4..T9 and print raw values               |
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
/* TOUCH_THRESHOLD is only used here to mark a channel as touched on the
 * printed line; the raw value is what you should observe to tune it. */
#define TOUCH_THRESHOLD    500
#define LOOP_PERIOD_MS     100

/* Channels observed in this debug app (same ones used by 4_hid_touch). */
typedef struct {
    touch_t     pad;
    const char *label;   /* printed channel label */
} chan_t;

static const chan_t s_ch[] = {
    { TOUCH_PAD_4, "T4(GPIO5) " },
    { TOUCH_PAD_5, "T5(GPIO6) " },
    { TOUCH_PAD_6, "T6(GPIO7) " },
    { TOUCH_PAD_7, "T7(GPIO8) " },
    { TOUCH_PAD_8, "T8(GPIO9) " },
    { TOUCH_PAD_9, "T9(GPIO10)" },
};
#define N_CH (sizeof(s_ch) / sizeof(s_ch[0]))

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();

    TouchHalInit();
    for (int i = 0; i < N_CH; i++) {
        TouchHalChannelConfig(s_ch[i].pad, TOUCH_THRESHOLD);
    }
    TouchHalStart();

    while (true) {
        printf("touch: ");
        bool any = false;
        for (int i = 0; i < N_CH; i++) {
            uint32_t v = 0;
            bool ok = TouchHalRead(s_ch[i].pad, &v);
            bool touched = ok && (v >= TOUCH_THRESHOLD + 200);
            if (touched) {
                any = true;
            }
            printf("%s=%4" PRIu32 "%s ", s_ch[i].label, ok ? v : 0,
                   touched ? "*" : " ");
        }
        printf("%s\n", any ? "[TOUCHED]" : "");
        any ? LedOn(LED_1) : LedOff(LED_1);
        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}
/*==================[end of file]============================================*/
