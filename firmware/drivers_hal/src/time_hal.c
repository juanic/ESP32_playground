/**
 * @file time_hal.c
 * @brief Virtual clock driver for ESP32 (Apps supplies epoch over BLE).
 */

/*==================[inclusions]=============================================*/
#include "time_hal.h"
#include "nvs_hal.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <string.h>

/*==================[macros and definitions]=================================*/

#define NVS_NAMESPACE     "relay"
#define NVS_KEY_REPOCH    "ref_epoch"
#define NVS_KEY_RUPTIME   "ref_uptime"

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
static const char *TAG = "time_hal";

static bool   s_inited = false;
static bool   s_synced = false;
static int64_t s_ref_epoch;               /* Unix seconds at sync time   */
static int64_t s_ref_uptime_us;           /* esp_timer uptime at sync time */

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static int64_t uptime_us(void) {
    return esp_timer_get_time();          /* microseconds since boot */
}

/*==================[external functions definition]==========================*/

bool TimeHalInit(void) {
    if (s_inited) return true;

    NvsHalInit();

    int64_t epoch = 0, upt = 0;
    size_t len = sizeof(epoch);
    bool ok_epoch = NvsHalGetBlob(NVS_NAMESPACE, NVS_KEY_REPOCH, &epoch, &len);
    len = sizeof(upt);
    bool ok_upt = NvsHalGetBlob(NVS_NAMESPACE, NVS_KEY_RUPTIME, &upt, &len);

    if (ok_epoch && ok_upt && epoch > 0) {
        s_ref_epoch = epoch;
        s_ref_uptime_us = upt;
        s_synced = true;
        ESP_LOGI(TAG, "Loaded persisted time reference (epoch %lld)", (long long)epoch);
    } else {
        s_synced = false;
        ESP_LOGW(TAG, "No time reference; awaiting App SYNC_TIME");
    }

    s_inited = true;
    return true;
}

bool TimeHalSync(int64_t epoch_unix) {
    if (epoch_unix <= 0) return false;

    s_ref_epoch = epoch_unix;
    s_ref_uptime_us = uptime_us();
    s_synced = true;

    bool ok = NvsHalSetBlob(NVS_NAMESPACE, NVS_KEY_REPOCH, &s_ref_epoch, sizeof(s_ref_epoch))
           && NvsHalSetBlob(NVS_NAMESPACE, NVS_KEY_RUPTIME, &s_ref_uptime_us, sizeof(s_ref_uptime_us));
    ESP_LOGI(TAG, "Time synced to epoch %lld (%s)",
             (long long)epoch_unix, ok ? "persisted" : "NVS write failed");
    return ok;
}

bool TimeHalGetNow(int64_t *out) {
    if (!out || !s_synced) return false;

    int64_t now = s_ref_epoch + (uptime_us() - s_ref_uptime_us) / 1000000LL;
    *out = now;
    return true;
}

bool TimeHalIsSynced(void) {
    return s_synced && s_inited;
}

/*==================[end of file]============================================*/
