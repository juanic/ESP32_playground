/**
 * @file bt_access_control.c
 * @brief Bluetooth access control middleware — whitelist/blacklist.
 */

/*==================[inclusions]=============================================*/
#include "bt_access_control.h"
#include "nvs_hal.h"
#include "esp_log.h"
#include <string.h>
/*==================[macros and definitions]=================================*/

#define NVS_NAMESPACE    "acl"
#define NVS_KEY_WHITELIST "whitelist"
#define NVS_KEY_BLACKLIST "blacklist"

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
static const char *TAG = "bt_access_control";

static uint8_t s_whitelist[ACL_MAX_ENTRIES][ACL_BDA_LEN];
static uint8_t s_blacklist[ACL_MAX_ENTRIES][ACL_BDA_LEN];
static uint8_t s_whitelist_count = 0;
static uint8_t s_blacklist_count = 0;
static bool s_initialized = false;

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

static bool bda_equal(const uint8_t *a, const uint8_t *b) {
    return memcmp(a, b, ACL_BDA_LEN) == 0;
}

static bool bda_in_list(const uint8_t *bda, const uint8_t list[][ACL_BDA_LEN], uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        if (bda_equal(bda, list[i])) {
            return true;
        }
    }
    return false;
}

static void load_lists(void) {
    size_t len;

    /* Load whitelist */
    len = sizeof(s_whitelist);
    if (NvsHalGetBlob(NVS_NAMESPACE, NVS_KEY_WHITELIST, s_whitelist, &len)) {
        s_whitelist_count = len / ACL_BDA_LEN;
        ESP_LOGI(TAG, "Loaded %d whitelist entries", s_whitelist_count);
    } else {
        s_whitelist_count = 0;
    }

    /* Load blacklist */
    len = sizeof(s_blacklist);
    if (NvsHalGetBlob(NVS_NAMESPACE, NVS_KEY_BLACKLIST, s_blacklist, &len)) {
        s_blacklist_count = len / ACL_BDA_LEN;
        ESP_LOGI(TAG, "Loaded %d blacklist entries", s_blacklist_count);
    } else {
        s_blacklist_count = 0;
    }
}

static bool save_whitelist(void) {
    return NvsHalSetBlob(NVS_NAMESPACE, NVS_KEY_WHITELIST,
                         s_whitelist, s_whitelist_count * ACL_BDA_LEN);
}

static bool save_blacklist(void) {
    return NvsHalSetBlob(NVS_NAMESPACE, NVS_KEY_BLACKLIST,
                         s_blacklist, s_blacklist_count * ACL_BDA_LEN);
}

/*==================[external functions definition]==========================*/

bool AccessControlInit(void) {
    if (s_initialized) return true;

    NvsHalInit();
    load_lists();
    s_initialized = true;
    return true;
}

bool AccessControlIsAllowed(const uint8_t *bda) {
    if (!bda) return false;

    /* If blacklisted, always deny */
    if (bda_in_list(bda, s_blacklist, s_blacklist_count)) {
        ESP_LOGI(TAG, "BDA %02x:%02x:%02x:%02x:%02x:%02x BLACKLISTED",
                 bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        return false;
    }

    /* If whitelist is empty, allow everyone (not blacklisted) */
    if (s_whitelist_count == 0) {
        return true;
    }

    /* Whitelist has entries — only whitelisted BDAs are allowed */
    if (bda_in_list(bda, s_whitelist, s_whitelist_count)) {
        return true;
    }

    ESP_LOGI(TAG, "BDA %02x:%02x:%02x:%02x:%02x:%02x NOT in whitelist",
             bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
    return false;
}

bool AccessControlAdd(const uint8_t *bda, acl_list_type_t list_type) {
    if (!bda) return false;

    if (list_type == ACL_WHITELIST) {
        if (s_whitelist_count >= ACL_MAX_ENTRIES) {
            ESP_LOGW(TAG, "Whitelist full");
            return false;
        }
        if (bda_in_list(bda, s_whitelist, s_whitelist_count)) {
            return true; /* already there */
        }
        memcpy(s_whitelist[s_whitelist_count], bda, ACL_BDA_LEN);
        s_whitelist_count++;
        ESP_LOGI(TAG, "Added to whitelist: %02x:%02x:%02x:%02x:%02x:%02x",
                 bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        return save_whitelist();
    } else {
        if (s_blacklist_count >= ACL_MAX_ENTRIES) {
            ESP_LOGW(TAG, "Blacklist full");
            return false;
        }
        if (bda_in_list(bda, s_blacklist, s_blacklist_count)) {
            return true;
        }
        memcpy(s_blacklist[s_blacklist_count], bda, ACL_BDA_LEN);
        s_blacklist_count++;
        ESP_LOGI(TAG, "Added to blacklist: %02x:%02x:%02x:%02x:%02x:%02x",
                 bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        return save_blacklist();
    }
}

bool AccessControlRemove(const uint8_t *bda) {
    if (!bda) return false;

    bool removed = false;

    /* Remove from whitelist */
    for (uint8_t i = 0; i < s_whitelist_count; i++) {
        if (bda_equal(bda, s_whitelist[i])) {
            memmove(&s_whitelist[i], &s_whitelist[i + 1],
                    (s_whitelist_count - i - 1) * ACL_BDA_LEN);
            s_whitelist_count--;
            removed = true;
            break;
        }
    }

    /* Remove from blacklist */
    for (uint8_t i = 0; i < s_blacklist_count; i++) {
        if (bda_equal(bda, s_blacklist[i])) {
            memmove(&s_blacklist[i], &s_blacklist[i + 1],
                    (s_blacklist_count - i - 1) * ACL_BDA_LEN);
            s_blacklist_count--;
            removed = true;
            break;
        }
    }

    if (removed) {
        ESP_LOGI(TAG, "Removed: %02x:%02x:%02x:%02x:%02x:%02x",
                 bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        save_whitelist();
        save_blacklist();
    }

    return removed;
}

bool AccessControlIsBlacklisted(const uint8_t *bda) {
    if (!bda) return false;
    return bda_in_list(bda, s_blacklist, s_blacklist_count);
}

bool AccessControlIsWhitelisted(const uint8_t *bda) {
    if (!bda) return false;
    return bda_in_list(bda, s_whitelist, s_whitelist_count);
}

/*==================[end of file]============================================*/
