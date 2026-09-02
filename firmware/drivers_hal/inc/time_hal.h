#ifndef TIME_HAL_H
#define TIME_HAL_H

/** @defgroup hal HAL
 *  @brief Hardware Abstraction Layer.
 *  @{
 *  @defgroup time_hal Time HAL
 *  @brief Virtual clock driver for ESP32.
 *  @{
 *
 * @section genDesc General Description
 *
 * The ESP32 has no persistent wall-clock on its own: it has no internet,
 * and without a battery-backed RTC the internal RTC clears when power is
 * removed. This HAL provides a simple "virtual clock":
 *
 * - A real Unix epoch is supplied by the host App over BLE (see
 *   the "SYNC_TIME" command in relay_control). This reference is stored
 *   in NVS together with the uptime at which it was received.
 * - The current time is derived as:
 *     now = ref_epoch + (uptime_now - ref_uptime)
 *   where uptime is read from esp_timer_get_time() (us since boot).
 *
 * This keeps the clock accurate while the device is powered, survives
 * a software reset (reference is persisted), and only drifts if the
 * power is lost for an extended period without a re-sync. The App is
 * expected to re-sync on each connection.
 *
 * @author Audio Relay Project
 *
 **/

/*==================[inclusions]=============================================*/
#include <stdbool.h>
#include <stdint.h>
/*==================[macros]=================================================*/

/** Sentinel for an unsynchronized / invalid timestamp. */
#define TIME_HAL_INVALID  ((int64_t)-1)

/*==================[typedef]================================================*/

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/

/**
 * @brief Initialize the time HAL and load any persisted reference.
 *
 * @return true if successful (always true unless NVS fails).
 */
bool TimeHalInit(void);

/**
 * @brief Synchronize the clock with a known Unix epoch.
 *
 * Stores the epoch together with the current uptime as the reference
 * and persists it to NVS.
 *
 * @param epoch_unix  Unix time (seconds since 1970-01-01 UTC).
 * @return true if the reference was stored.
 */
bool TimeHalSync(int64_t epoch_unix);

/**
 * @brief Get the current Unix time.
 *
 * @param out  On success receives the current Unix time in seconds.
 * @return true if the clock is synchronized and @p out is valid.
 */
bool TimeHalGetNow(int64_t *out);

/**
 * @brief True if a valid time reference is present.
 *
 * @return true if TimeHalGetNow() can be used reliably.
 */
bool TimeHalIsSynced(void);

/** @} */
/** @} */

#endif /* #ifndef TIME_HAL_H */

/*==================[end of file]============================================*/
