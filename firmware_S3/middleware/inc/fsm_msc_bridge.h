/*! @file fsm_msc_bridge.h
 *  @brief Finite state machine for SERVER/MSC Bridge mode selection.
 *  @defgroup fsm_msc_bridge FSM MSC Bridge
 *  @{
 *
 * Máquina de estados que controla la transición entre el modo Servidor
 * (HTTP) y el modo MSC (Mass Storage Class) USB. Incluye las guardas necesarias
 * para evitar conflictos (ej. montar FAT y MSC al mismo tiempo).
 *
 * La FSM es llamada desde la capa middleware y usa los wrappers de
 * drivers_hal para las operaciones de bajo nivel.
 */

#ifndef FSM_MSC_BRIDGE_H
#define FSM_MSC_BRIDGE_H

/*==================[inclusions]=============================================*/
#include <stdbool.h>
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief FSM state enumeration.
 */
typedef enum {
    FSM_STATE_SERVER,       /**< Modo servidor HTTP activo */
    FSM_STATE_PENDRIVE,     /**< Modo MSC activado */
    FSM_STATE_TRANSITION    /**< Estado de transición en curso */
} fsm_state_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Initialize the FSM state machine.
 *
 * Deja la FSM en estado SERVER por defecto y prepara los recursos
 * necesarios (HTTP server, MSC driver).
 */
void fsm_msc_bridge_init(void);

/**
 * @brief Change from SERVER mode to MSC mode.
 *
 * Guarda las guardas:
 * - No permite la transición si ya hay una operación de escritura HTTP en curso.
 * - Desactiva el servidor HTTP.
 * - Desmonta el sistema de archivos FAT.
 * - Inicializa TinyUSB MSC y conecta el drive.
 *
 * @return true si la transición fue exitosa, false si se denegó.
 */
bool fsm_msc_bridge_server_to_msc(void);

/**
 * @brief Change from MSC mode back to SERVER mode.
 *
 * Guarda las guardas:
 * - Primero desconecta MSC (tud_disconnect).
 * - Espera un retardo de 200-300 ms para que el host termine de leer/escribir.
 * - Vuelve a montar el sistema de archivos FAT local.
 * - Reactiva el servidor HTTP.
 *
 * @return true si la transición fue exitosa, false si se denegó.
 */
bool fsm_msc_bridge_msc_to_server(void);

/**
 * @brief Get the current FSM state.
 *
 * @return fsm_state_t Estado actual de la máquina.
 */
fsm_state_t fsm_msc_bridge_get_state(void);

/** @} */
#endif /* #ifndef FSM_MSC_BRIDGE_H */

/*==================[end of file]============================================*/