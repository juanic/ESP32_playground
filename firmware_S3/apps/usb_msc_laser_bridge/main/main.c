/*! @mainpage UsbMscLaserBridge
 *
 * @section genDesc General Description
 *
 * Aplicación para el XIAO ESP32S3 Sense que alterna entre dos modos de operación:
 * - MODO SERVIDOR: El ESP32-S3 actúa como un servidor HTTP simple que permite
 *   subir archivos (ej. imágenes de corte de láser) a la microSD interna.
 * - MODO PENDRIVE: El ESP32-S3 actúa como un dispositivo USB MSC (Mass Storage
 *   Class), apareciendo como un pendrive para la controladora del láser, con la
 *   microSD como almacenamiento interno.
 *
 * La transición entre modos es controlada por un botón físico conectado al GPIO
 * correspondiente. Un press cambia de Servidor a Pendrive, y un segundo press
 * vuelve al modo Servidor.
 *
 * @section hardConn Hardware Connection
 * - Botón: GPIO_X (ver board_support/inc/button_pins.h, TODO confirmar número)
 * - LED addressable: GPIO_Y (ver board_support/inc/led_pins.h, TODO confirmar)
 * - MicroSD slot: pines SPI SD (ver board_support/inc/sd_pins.h, TODO confirmar)
 * - Puerto USB: USB OTG native del ESP32-S3 (XIAO)
 *
 * @section changelog Changelog
 *
 * |   Date       | Description                          |
 * |:-------------|:------------------------------------|
 * | 29/08/2026   | Initial creation                     |
 *
 * @author Juan Cerrudo
 *
 */

/*==================[inclusions]=============================================*/
/* Application includes follow the repo dependency rules:
 * - apps/ puede llamar a middleware/ y board_support/
 * - middleware/ puede llamar a board_support/ y drivers_hal/
 * - board_support/ puede llamar a drivers_hal/
 * - drivers_hal/ usa APIs de ESP-IDF directamente
 *
 * Por eso incluimos aquí solo board_support y middleware.
 * La inicialización real y el loop son llamados desde main.
 */

#include "board_support/inc/board_support.h"  /* Inicializa LEDs, GPIO, SD pins */
#include "middleware/inc/fsm_msc_bridge.h"    /* Máquina de estados SERVER <-> MSC Bridge */
#include "middleware/inc/http_server.h"        /* Servidor HTTP para uploads */

/*==================[macros and definitions]=================================*/
/* Etiquetas para el loop principal */
#define LOOP_PERIOD_MS 100

/*==================[internal data definition]=============================*/

/*==================[internal functions declaration]=======================*/

/*==================[external functions definition]======================*/
void app_main(void) {
    /* Inicializar la capa board_support (LEDs, GPIO, pins de SD) */
    board_support_init();

    /* Inicializar la máquina de estados FSM (SERVER por defecto) */
    fsm_msc_bridge_init();

    /* Bucle principal: solo refresca el estado y lee el botón */
    while (true) {
        /* Leer el botón y gestionar la transición FSM */
        /* El botón está conectado al GPIO definido en board_support */
        /* Si el botón es pulsado durante >500 ms, toggle de modo */
        /* (La lógica detallada está en fsm_msc_bridge.c con las funciones
           fsm_msc_bridge_server_to_msc() y fsm_msc_bridge_msc_to_server()) */

        /* Pequeño retraso para evitar lecturas demasiado rápidas */
        vTaskDelay(pdMS_TO_TICKS(LOOP_PERIOD_MS));
    }
}

/*==================[end of file]============================================*/