/**
 * @file avrcp_hal.c
 * @brief AVRCP Controller (CT) — media metadata and playback events.
 */

#include "avrcp_hal.h"
#include "sdkconfig.h"

#if CONFIG_BT_ENABLED && CONFIG_BT_CLASSIC_ENABLED && CONFIG_BT_A2DP_ENABLE

#include "esp_log.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include <string.h>

static const char *TAG = "avrcp_hal";

/* ======================== State ======================== */

static bool s_inited = false;
static bool s_connected = false;

static avrcp_meta_cb_t s_meta_cb = NULL;
static void *s_meta_ctx = NULL;

static avrcp_meta_t s_meta = {
    .title = {0},
    .artist = {0},
    .album = {0},
    .play_state = AVRCP_PLAY_STATE_STOPPED,
    .track_changed = false,
};

static uint8_t s_transaction = 1;

/* ======================== Forward declarations ======================== */

static void avrcp_ct_callback(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);

/* ======================== Internal ======================== */

static void notify_meta(void) {
    s_meta.track_changed = false;
    if (s_meta_cb) {
        s_meta_cb(&s_meta, s_meta_ctx);
    }
}

static void copy_attr(char *dst, size_t dst_size, uint8_t *src, int len) {
    if (!dst || !src || dst_size == 0) return;
    size_t n = (len > 0) ? (size_t)len : 0;
    if (n >= dst_size) n = dst_size - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

/* ======================== AVRCP CT callback ======================== */

static void avrcp_ct_callback(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param) {
    switch (event) {
    case ESP_AVRC_CT_CONNECTION_STATE_EVT: {
        s_connected = param->conn_stat.connected;
        ESP_LOGI(TAG, "AVRCP CT connected=%d", s_connected);
        if (s_connected) {
            /* Register notifications: track change + play status */
            s_transaction = 1;
            esp_avrc_ct_send_register_notification_cmd(s_transaction++,
                                                       ESP_AVRC_RN_TRACK_CHANGE,
                                                       0);
            esp_avrc_ct_send_register_notification_cmd(s_transaction++,
                                                       ESP_AVRC_RN_PLAY_STATUS_CHANGE,
                                                       0);
            /* Query now-playing metadata */
            esp_avrc_ct_send_metadata_cmd(s_transaction++,
                                          ESP_AVRC_MD_ATTR_TITLE |
                                          ESP_AVRC_MD_ATTR_ARTIST |
                                          ESP_AVRC_MD_ATTR_ALBUM);
        }
        break;
    }

    case ESP_AVRC_CT_METADATA_RSP_EVT: {
        switch (param->meta_rsp.attr_id) {
        case ESP_AVRC_MD_ATTR_TITLE:
            copy_attr(s_meta.title, sizeof(s_meta.title),
                      param->meta_rsp.attr_text, param->meta_rsp.attr_length);
            ESP_LOGI(TAG, "Title: %s", s_meta.title);
            break;
        case ESP_AVRC_MD_ATTR_ARTIST:
            copy_attr(s_meta.artist, sizeof(s_meta.artist),
                      param->meta_rsp.attr_text, param->meta_rsp.attr_length);
            ESP_LOGI(TAG, "Artist: %s", s_meta.artist);
            break;
        case ESP_AVRC_MD_ATTR_ALBUM:
            copy_attr(s_meta.album, sizeof(s_meta.album),
                      param->meta_rsp.attr_text, param->meta_rsp.attr_length);
            ESP_LOGI(TAG, "Album: %s", s_meta.album);
            break;
        default:
            break;
        }
        break;
    }

    case ESP_AVRC_CT_CHANGE_NOTIFY_EVT: {
        if (param->change_ntf.event_id == ESP_AVRC_RN_TRACK_CHANGE) {
            ESP_LOGI(TAG, "Track changed");
            s_meta.track_changed = true;
            /* Re-query metadata + re-subscribe */
            esp_avrc_ct_send_metadata_cmd(s_transaction++,
                                          ESP_AVRC_MD_ATTR_TITLE |
                                          ESP_AVRC_MD_ATTR_ARTIST |
                                          ESP_AVRC_MD_ATTR_ALBUM);
            esp_avrc_ct_send_register_notification_cmd(s_transaction++,
                                                       ESP_AVRC_RN_TRACK_CHANGE,
                                                       0);
            notify_meta();
        } else if (param->change_ntf.event_id == ESP_AVRC_RN_PLAY_STATUS_CHANGE) {
            s_meta.play_state = (avrcp_play_state_t)param->change_ntf.event_parameter.playback;
            ESP_LOGI(TAG, "Play status: %d", (int)s_meta.play_state);
            esp_avrc_ct_send_register_notification_cmd(s_transaction++,
                                                       ESP_AVRC_RN_PLAY_STATUS_CHANGE,
                                                       0);
            notify_meta();
        }
        break;
    }

    case ESP_AVRC_CT_PLAY_STATUS_RSP_EVT: {
        s_meta.play_state = (avrcp_play_state_t)param->play_status_rsp.play_status;
        ESP_LOGI(TAG, "Play status rsp: %d", (int)s_meta.play_state);
        break;
    }

    case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT:
    case ESP_AVRC_CT_REMOTE_FEATURES_EVT:
    case ESP_AVRC_CT_GET_RN_CAPABILITIES_RSP_EVT:
    default:
        break;
    }
}

/* ======================== Public API ======================== */

bool avrcp_hal_init(avrcp_meta_cb_t cb, void *ctx) {
    if (s_inited) return true;

    s_meta_cb = cb;
    s_meta_ctx = ctx;

    esp_err_t ret = esp_avrc_ct_register_callback(avrcp_ct_callback);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AVRCP CT register callback failed: %s", esp_err_to_name(ret));
        return false;
    }
    ret = esp_avrc_ct_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "AVRCP CT init failed: %s", esp_err_to_name(ret));
        return false;
    }

    s_inited = true;
    ESP_LOGI(TAG, "AVRCP CT initialized");
    return true;
}

bool avrcp_hal_get_metadata(avrcp_meta_t *meta) {
    if (!meta) return false;
    memcpy(meta, &s_meta, sizeof(s_meta));
    return true;
}

#else /* !(CONFIG_BT_ENABLED && CLASSIC && A2DP) — stubs */

#include "esp_log.h"
static const char *TAG = "avrcp_hal";

bool avrcp_hal_init(avrcp_meta_cb_t cb, void *ctx) {
    ESP_LOGW(TAG, "AVRCP not enabled in sdkconfig");
    return false;
}
bool avrcp_hal_get_metadata(avrcp_meta_t *meta) { return false; }

#endif /* CONFIG_BT_ENABLED && CLASSIC && A2DP */

/*==================[end of file]============================================*/
