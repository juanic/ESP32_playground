/*! @file button_hal.c
 *  @brief GPIO button driver with software antirrebote.
 *  @defgroup button_hal Button HAL impl.
 *  @{
 *
 * Implementa la lectura de un botón físico conectado a un GPIO del
 * ESP32-S3 con detección de flanco de borde y filtrado antirrebote
 * en tiempo de software.
 *
 * El botón se configura con pull-up interno. La función read() retorna
 * el estado debounceado cada que se llama, por lo que la aplicación
 * debe hacer polling periódicamente (ej. cada 10-20 ms).
 */

#include "button_hal.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/gpio_struct.h"
#include "soc/gpio_periph.h"

static const gpio_config_t btn_conf = {
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULL_UP_ENABLE,
    .pull_down_en = GPIO_PULL_DOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};

static uint32_t last_debounce_time = 0;
static btn_state_t last_state = BTN_STATE_RELEASED;

void btn_hal_init(void) {
    /* GPIO0..34 are valid, but we configure the specific button GPIO
       in board_support. Here we just set the generic config. */
    gpio_config(&btn_conf);
}

btn_state_t btn_hal_read(void) {
    uint32_t level = gpio_get_level(BTN_GPIO);  /* BTN_GPIO defined in board_support */
    bool pressed = (level == 0);  /* Active low con pull-up */

    /* Antirrebote simple: solo actualizar si ha pasado BTN_DEBOUNCE_MS */
    if (pressed != last_state) {
        if (xTaskGetTickCount() - last_debounce_time > pdMS_TO_TICKS(BTN_DEBOUNCE_MS)) {
            last_state = pressed;
            last_debounce_time = xTaskGetTickCount();
        }
    }

    return last_state;
}

/*==================[end of file]============================================*/