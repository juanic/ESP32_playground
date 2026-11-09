/*! @file tinyusb_msc.h
 *  @brief TinyUSB MSC (Mass Storage Class) driver wrapper.
 *  @defgroup tinyusb_msc TinyUSB MSC HAL
 *  @{
 *
 * Wrapper alrededor de la librería TinyUSB para operar el periférico
 * USB en modo Device (MSC), permitiendo que el host (láser) lea/escriba
 * archivos en la tarjeta SD conectada al ESP32-S3.
 *
 * El modo de operación (Server vs MSC) es controlado por la capa
 * middleware a través de las funciones de transición definidas allí.
 *
 * @note Las funciones aquí están pensadas para ser llamadas desde
 * middleware, nunca directamente desde las apps.
 */

#ifndef TINYUSB_MSC_H
#define TINYUSB_MSC_H

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include <stdbool.h>
/*==================[macros]=================================================*/
#define MSC_TUD_CONNECT()   tud_connect()
#define MSC_TUD_DISCONNECT() tud_disconnect()
/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief Initialize the TinyUSB MSC driver.
 *
 * Configura el periférico USB en modo Device MSC con la tarjeta SD
 * como backend (backing store). Debe llamarse una vez al arranque.
 *
 * @return true si la inicialización fue exitosa, false en caso contrario.
 */
bool tinyusb_msc_init(void);

/**
 * @brief Put the MSC device online (host can now enumerate the drive).
 *
 * Llama a tud_connect() para que el host detecte el drive nuevo.
 * Debe conocerse después de tinyusb_msc_init().
 */
void tinyusb_msc_connect(void);

/**
 * @brief Put the MSC device offline (host can no longer access the drive).
 *
 * Llama a tud_disconnect() para simular un "eject" seguro antes de
 * desmontar el sistema de archivos.
 */
void tinyusb_msc_disconnect(void);

/** @} */
#endif /* #ifndef TINYUSB_MSC_H */

/*==================[end of file]============================================*/