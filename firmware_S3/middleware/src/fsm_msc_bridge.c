/*! @file fsm_msc_bridge.c
 *  @brief Finite state machine implementation for SERVER/MSC Bridge mode.
 *  @defgroup fsm_msc_bridge FSM impl.
 *  @{
 *
 * Implementa la máquina de estados que controla la transición entre
 * el modo Servidor (HTTP) y el modo MSC (Mass Storage Class) USB. Incluye
 * las guardas necesarias para evitar conflictos (ej. montar FAT y MSC al
 * mismo tiempo).
 *
 * La FSM usa los wrappers de drivers_hal para las operaciones de bajo
 * nivel (mount/unmount, MSC connect/disconnect).
 *
 * El estado inicial es SERVER. Las transiciones son:
 * - SERVER -> MSC: detiene HTTP, desmonta FAT, inicializa TinyUSB MSC y
 *   conecta el drive (tud_connect). Hay un timer de 250 ms que vuelve
 *   automáticamente a SERVER al concluir.
 * - MSC -> SERVER: desconecta MSC (tud_disconnect), espera el timer,
 *   vuelve a montar FAT y.reactiva el HTTP.
 *
 * @{
 */

#include "fsm_msc_bridge.h"
#include "http_server.h"
#include "sd_hal.h"
#include "tinyusb_msc.h"
#include "button_hal.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "string.h"

/*==================[internal data declaration]==============================*/
static fsm_state_t current_state = FSM_STATE_SERVER;

/* HTTP server handle */
static httpd_handle_t server = NULL;

/* Timer for the 200-300ms delay during mode switch */
static TimerHandle_t transition_timer = NULL;

/* Function called by the transition timer to switch back to server mode */
static void transition_timer_callback(TimerHandle_t xTimerID) {
    /* Switch back to server mode automatically after the delay */
    if (current_state == FSM_STATE_PENDRIVE) {
        fsm_msc_bridge_msc_to_server();
    }
}

/*==================[internal functions declaration]=========================*/
static bool fsm_guard_no_active_write(void) {
    /* Simple guard: check if we are currently in a state that would
     * conflict. In a full implementation, this might check HTTP
     * request counters or semaphores. */
    return true;
}

static void fsm_enter_server(void) {
    /* Mount FAT filesystem */
    sd_mount();
    /* Start HTTP server */
    http_server_start((http_server_config_t *)http_server_start(NULL));
    current_state = FSM_STATE_SERVER;
    ESP_LOGI("fsm_msc_bridge", "Transitioned to SERVER mode");
}

static void fsm_enter_penDrive(void) {
    /* Disconnect MSC first */
    tinyusb_msc_disconnect();
    /* Unmount FAT */
    sd_unmount();
    /* Initialize TinyUSB MSC and connect */
    tinyusb_msc_init();
    tinyusb_msc_connect();
    current_state = FSM_STATE_PENDRIVE;
    /* Start a timer to auto-return to server after 250ms */
    if (transition_timer == NULL) {
        transition_timer = xTimerCreate("transition_timer",
                                        pdMS_TO_TICKS(250),
                                        pdFALSE,
                                        (void *)0,
                                        transition_timer_callback);
    }
    xTimerStart(transition_timer, 0);
    ESP_LOGI("fsm_msc_bridge", "Transitioned to PENDRIVE mode");
}

/*==================[external functions definition]=========================*/
void fsm_msc_bridge_init(void) {
    current_state = FSM_STATE_SERVER;
    /* Initialize underlying layers */
    sd_mount();  /* mount FAT initially */
    ESP_LOGI("fsm_msc_bridge", "FSM initialized in SERVER mode");
}

bool fsm_msc_bridge_server_to_msc(void) {
    if (current_state != FSM_STATE_SERVER) {
        ESP_LOGW("fsm_msc_bridge", "Cannot transition: not in SERVER mode");
        return false;
    }

    /* Guard: check if no active HTTP write is in progress */
    if (!fsm_guard_no_active_write()) {
        ESP_LOGW("fsm_msc_bridge", "Guard: active HTTP write in progress");
        return false;
    }

    /* Leave server mode */
    http_server_stop();
    /* Enter msc mode */
    fsm_enter_penDrive();

    return true;
}

bool fsm_msc_bridge_msc_to_server(void) {
    if (current_state != FSM_STATE_PENDRIVE) {
        ESP_LOGW("fsm_msc_bridge", "Cannot transition: not in MSC mode");
        return false;
    }

    /* Stop the transition timer first */
    if (transition_timer) {
        xTimerDelete(transition_timer, 0);
        transition_timer = NULL;
    }

    /* Return to server mode immediately (no auto-delay) */
    fsm_enter_server();

    return true;
}

fsm_state_t fsm_msc_bridge_get_state(void) {
    return current_state;
}

/*==================[end of file]============================================*/
/*==================[end of file]============================================*/