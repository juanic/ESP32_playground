/*! @mainpage Touch
 *
 * @section genDesc General Description
 *
 * Debug app that reads the six capacitive touch pads T4..T9 of the ESP32-S3
 * (touch_hal) and prints their filtered (smoothed) value through the serial
 * console, together with the deviation from a baseline captured at startup.
 *
 * On the ESP32-S3 the channel value INCREASES when the pad is touched, and the
 * untouched baseline is already a large number (~3000-4000 with the v2 sensor),
 * so a fixed absolute threshold is useless: detection must be RELATIVE to the
 * baseline. Use this app to observe the real deviation while you touch a pad and
 * tune TOUCH_DELTA_TRIGGER (and the 4_hid_touch app's threshold) accordingly.
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
 * | 29/08/2026 | Relative (baseline) touch detection + print deviation |
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
#define TOUCH_CAL_SAMPLES     32      /* samples averaged for the baseline */
#define TOUCH_DELTA_TRIGGER   200     /* deviation above baseline => touched */
#define LOOP_PERIOD_MS        100

/* Channels observed in this debug app (same ones used by 4_hid_touch). */
typedef struct {
    touch_t     pad;
    const char *label;
} chan_t;

static const chan_t s_ch[] = {
    { TOUCH_PAD_4, "T4" },
    { TOUCH_PAD_5, "T5" },
    { TOUCH_PAD_6, "T6" },
    { TOUCH_PAD_7, "T7" },
    { TOUCH_PAD_8, "T8" },
    { TOUCH_PAD_9, "T9" },
};
#define N_CH (sizeof(s_ch) / sizeof(s_ch[0]))

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();

    TouchHalInit();
    for (int i = 0; i < N_CH; i++) {
        /* The internal relative threshold is unused here; we do our own
         * baseline-delta detection, so a placeholder (0) is fine. */
        TouchHalChannelConfig(s_ch[i].pad, 0);
    }
    TouchHalStart();

    /* Let the touch-sensor filter settle before calibrating: the first readings
     * right after continuous scanning starts can spike, which corrupts the
     * baseline of the first channel sampled (T4). */
    vTaskDelay(pdMS_TO_TICKS(1000));

    /* Calibration: capture the untouched baseline of each channel. */
    uint32_t baseline[N_CH] = {0};
    for (int i = 0; i < N_CH; i++) {
        uint32_t sum = 0, v = 0;
        (void)TouchHalRead(s_ch[i].pad, &v); /* discard one transient sample */
        for (int s = 0; s < TOUCH_CAL_SAMPLES; s++) {
            if (TouchHalRead(s_ch[i].pad, &v)) {
                sum += v;
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        baseline[i] = sum / TOUCH_CAL_SAMPLES;
        printf("%s baseline = %" PRIu32 "\n", s_ch[i].label, baseline[i]);
    }

    while (true) {
        printf("touch: ");
        bool any = false;
        for (int i = 0; i < N_CH; i++) {
            uint32_t v = 0;
            TouchHalRead(s_ch[i].pad, &v);
            int32_t dev = (int32_t)v - (int32_t)baseline[i];
            bool touched = (dev >= TOUCH_DELTA_TRIGGER);
            if (touched) {
                any = true;
            }
            printf("%s=%4" PRIu32 " (d=%+" PRId32 ")%s ", s_ch[i].label, v, dev,
                   touched ? "*" : " ");
        }
        printf("%s\n", any ? "[TOUCHED]" : "");
        any ? LedOn(LED_1) : LedOff(LED_1);
        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}
/*==================[end of file]============================================*/
