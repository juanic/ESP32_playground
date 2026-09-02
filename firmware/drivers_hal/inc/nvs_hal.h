#ifndef NVS_HAL_H
#define NVS_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup nvs_hal NVS HAL
 *  @brief Non-volatile storage driver for ESP32.
 *  @{
 *
 * @section genDesc General Description
 *
 * Simple wrapper around ESP-IDF NVS API for persistent key-value storage.
 * Supports blobs and strings with automatic NVS flash initialization.
 *
 * @author Audio Relay Project
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
/*==================[macros]=================================================*/

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialize NVS flash. Safe to call multiple times.
 *
 * @return true if initialization succeeded.
 */
bool NvsHalInit(void);

/**
 * @brief Store a blob in NVS.
 *
 * @param namespace NVS namespace (max 15 chars).
 * @param key       NVS key (max 15 chars).
 * @param data      Pointer to data to store.
 * @param len       Length of data in bytes.
 * @return true if successful.
 */
bool NvsHalSetBlob(const char *namespace, const char *key,
                   const void *data, size_t len);

/**
 * @brief Read a blob from NVS.
 *
 * @param namespace NVS namespace.
 * @param key       NVS key.
 * @param data      Output buffer.
 * @param len       On input: buffer size. On output: actual data size.
 * @return true if successful.
 */
bool NvsHalGetBlob(const char *namespace, const char *key,
                   void *data, size_t *len);

/**
 * @brief Store a string in NVS.
 *
 * @param namespace NVS namespace.
 * @param key       NVS key.
 * @param value     Null-terminated string to store.
 * @return true if successful.
 */
bool NvsHalSetStr(const char *namespace, const char *key, const char *value);

/**
 * @brief Read a string from NVS.
 *
 * @param namespace NVS namespace.
 * @param key       NVS key.
 * @param value     Output buffer.
 * @param max_len   Size of output buffer.
 * @return true if successful.
 */
bool NvsHalGetStr(const char *namespace, const char *key,
                  char *value, size_t max_len);

/**
 * @brief Erase a single key from NVS.
 *
 * @param namespace NVS namespace.
 * @param key       NVS key.
 * @return true if successful.
 */
bool NvsHalEraseKey(const char *namespace, const char *key);

/** @} */
/** @} */

#endif /* #ifndef NVS_HAL_H */

/*==================[end of file]============================================*/
