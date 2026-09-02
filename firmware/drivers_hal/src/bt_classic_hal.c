/**
 * @file bt_classic_hal.c
 * @brief Bluetooth Classic HAL for ESP-IDF 6.x (Bluedroid A2DP).
 */

#include "bt_classic_hal.h"
#include "sdkconfig.h"
#include "esp_log.h"

#if CONFIG_BT_ENABLED

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "bt_classic_hal";

/* ======================== State ======================== */

static bool s_stack_initialized = false;
static bool s_a2d_connected = false;
static esp_bd_addr_t s_peer_bda = {0};
static char s_peer_name[ESP_BT_GAP_MAX_BDNAME_LEN + 1] = {0};
static char s_target_name[ESP_BT_GAP_MAX_BDNAME_LEN + 1] = {0};

/* Role: sink or source (mutually exclusive) */
static bool s_is_sink = false;
static bool s_is_source = false;

/* Callbacks */
static bt_classic_sink_data_cb_t s_sink_data_cb = NULL;
static bt_classic_source_state_cb_t s_source_state_cb = NULL;
static bt_classic_source_data_cb_t s_source_data_cb = NULL;
static bt_classic_source_scan_cb_t s_source_scan_cb = NULL;
static bt_classic_conn_state_cb_t s_conn_cb = NULL;

/* Source state machine */
enum {
    SRC_STATE_IDLE = 0,
    SRC_STATE_DISCOVERING,
    SRC_STATE_DISCOVERED,
    SRC_STATE_CONNECTING,
    SRC_STATE_CONNECTED,
    SRC_STATE_DISCONNECTING,
    SRC_STATE_SCANNING,
};
static int s_src_state = SRC_STATE_IDLE;

/* ======================== Forward declarations ======================== */

static void a2d_event_handler(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);
static void gap_event_handler(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
static int32_t a2d_source_data_callback(uint8_t *data, int32_t len);
static void a2d_sink_data_callback(const uint8_t *data, uint32_t len);
static void source_connect_to_target(void);

/* ======================== Helpers ======================== */

static char *bda2str(const uint8_t *bda, char *str, size_t size) {
    if (!bda || !str || size < 18) return NULL;
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
    return str;
}

static bool get_name_from_eir(uint8_t *eir, uint8_t *bdname, uint8_t *bdname_len) {
    uint8_t *rmt_bdname = NULL;
    uint8_t rmt_bdname_len = 0;

    if (!eir) return false;

    rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME, &rmt_bdname_len);
    if (!rmt_bdname) {
        rmt_bdname = esp_bt_gap_resolve_eir_data(eir, ESP_BT_EIR_TYPE_SHORT_LOCAL_NAME, &rmt_bdname_len);
    }

    if (rmt_bdname) {
        if (rmt_bdname_len > ESP_BT_GAP_MAX_BDNAME_LEN) {
            rmt_bdname_len = ESP_BT_GAP_MAX_BDNAME_LEN;
        }
        if (bdname) {
            memcpy(bdname, rmt_bdname, rmt_bdname_len);
            bdname[rmt_bdname_len] = '\0';
        }
        if (bdname_len) {
            *bdname_len = rmt_bdname_len;
        }
        return true;
    }
    return false;
}

/* ======================== GAP callback ======================== */

static void gap_event_handler(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch (event) {
    case ESP_BT_GAP_DISC_RES_EVT: {
        if (s_src_state != SRC_STATE_DISCOVERING && s_src_state != SRC_STATE_SCANNING) break;

        char bda_str[18];
        ESP_LOGI(TAG, "Scanned: %s", bda2str(param->disc_res.bda, bda_str, sizeof(bda_str)));

        uint8_t *eir = NULL;
        for (int i = 0; i < param->disc_res.num_prop; i++) {
            esp_bt_gap_dev_prop_t *p = param->disc_res.prop + i;
            if (p->type == ESP_BT_GAP_DEV_PROP_EIR) {
                eir = (uint8_t *)(p->val);
            }
        }

        if (eir) {
            uint8_t name_len = 0;
            if (get_name_from_eir(eir, (uint8_t *)s_peer_name, &name_len)) {
                if (s_src_state == SRC_STATE_SCANNING) {
                    if (s_source_scan_cb) s_source_scan_cb(s_peer_name, param->disc_res.bda);
                    break;
                }
                if (strcmp(s_peer_name, s_target_name) == 0) {
                    ESP_LOGI(TAG, "Found target: %s", s_peer_name);
                    memcpy(s_peer_bda, param->disc_res.bda, BT_CLASSIC_BD_ADDR_LEN);
                    s_src_state = SRC_STATE_DISCOVERED;
                    esp_bt_gap_cancel_discovery();
                }
            }
        }
        break;
    }
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT: {
        if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
            if (s_src_state == SRC_STATE_DISCOVERED) {
                s_src_state = SRC_STATE_CONNECTING;
                ESP_LOGI(TAG, "Connecting to %s ...", s_peer_name);
                esp_a2d_source_connect(s_peer_bda);
            } else if (s_src_state == SRC_STATE_DISCOVERING) {
                ESP_LOGI(TAG, "Target not found, restarting discovery...");
                esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
            } else if (s_src_state == SRC_STATE_SCANNING) {
                s_src_state = SRC_STATE_IDLE;
                s_source_scan_cb = NULL;
            }
        }
        break;
    }
    case ESP_BT_GAP_AUTH_CMPL_EVT: {
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(TAG, "Auth success: %s", param->auth_cmpl.device_name);
        } else {
            ESP_LOGE(TAG, "Auth failed: %d", param->auth_cmpl.stat);
        }
        break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT: {
        esp_bt_pin_code_t pin_code = {0};
        esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
        break;
    }
    case ESP_BT_GAP_CFM_REQ_EVT: {
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    }
    default:
        break;
    }
}

/* ======================== A2DP callbacks ======================== */

static void a2d_event_handler(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param) {
    switch (event) {
    case ESP_A2D_PROF_STATE_EVT: {
        if (param->a2d_prof_stat.init_state == ESP_A2D_INIT_SUCCESS) {
            ESP_LOGI(TAG, "A2DP profile enabled");
        }
        break;
    }
    case ESP_A2D_CONNECTION_STATE_EVT: {
        if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
            s_a2d_connected = true;
            memcpy(s_peer_bda, param->conn_stat.remote_bda, BT_CLASSIC_BD_ADDR_LEN);
            ESP_LOGI(TAG, "A2DP connected: %02x:%02x:%02x:%02x:%02x:%02x",
                     s_peer_bda[0], s_peer_bda[1], s_peer_bda[2],
                     s_peer_bda[3], s_peer_bda[4], s_peer_bda[5]);
            if (s_is_source && s_source_state_cb) {
                s_source_state_cb(true);
            }
            if (s_conn_cb) {
                s_conn_cb(true, s_peer_bda);
            }
        } else if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
            s_a2d_connected = false;
            ESP_LOGI(TAG, "A2DP disconnected");
            if (s_conn_cb) {
                s_conn_cb(false, NULL);
            }
            if (s_is_source) {
                s_src_state = SRC_STATE_IDLE;
                if (s_source_state_cb) {
                    s_source_state_cb(false);
                }
            }
        }
        break;
    }
    case ESP_A2D_AUDIO_STATE_EVT: {
        if (param->audio_stat.state == ESP_A2D_AUDIO_STATE_STARTED) {
            ESP_LOGI(TAG, "A2DP audio started");
        } else {
            ESP_LOGI(TAG, "A2DP audio stopped");
        }
        break;
    }
    case ESP_A2D_AUDIO_CFG_EVT: {
        ESP_LOGI(TAG, "A2DP audio config: rate=%d ch=%d",
                 param->audio_cfg.mcc.cie.sbc_info.samp_freq,
                 param->audio_cfg.mcc.cie.sbc_info.ch_mode);
        break;
    }
    case ESP_A2D_REPORT_SNK_CODEC_CAPS_EVT: {
        ESP_LOGI(TAG, "Sink codec caps received");
        break;
    }
    default:
        break;
    }
}

static void a2d_sink_data_callback(const uint8_t *data, uint32_t len) {
    if (s_sink_data_cb) {
        s_sink_data_cb(data, len);
    }
}

static int32_t a2d_source_data_callback(uint8_t *data, int32_t len) {
    if (!data || len <= 0) {
        return 0;
    }

    int32_t supplied = s_source_data_cb ? s_source_data_cb(data, len) : 0;
    if (supplied < 0) supplied = 0;
    if (supplied > len) supplied = len;
    if (supplied < len) {
        memset(data + supplied, 0, (size_t)(len - supplied));
    }
    return len;
}

/* ======================== Source helpers ======================== */

static void source_connect_to_target(void) {
    if (strlen(s_target_name) == 0) {
        ESP_LOGW(TAG, "No target name set");
        return;
    }
    ESP_LOGI(TAG, "Starting discovery for: %s", s_target_name);
    s_src_state = SRC_STATE_DISCOVERING;
    esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
}

/* ======================== Public API ======================== */

bool bt_classic_hal_init(const char *device_name) {
    if (s_stack_initialized) {
        ESP_LOGW(TAG, "BT stack already initialized");
        return true;
    }

    ESP_LOGI(TAG, "Initializing BT Classic stack...");

    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Dual-mode controller: keep BLE memory so a BLE GATT control
     * channel can coexist with Classic A2DP. Do NOT release BLE RAM. */

    /* Init BT controller (dual-mode BTDM) */
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BT controller init failed: %s", esp_err_to_name(ret));
        return false;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BT controller enable failed: %s", esp_err_to_name(ret));
        return false;
    }

    /* Init Bluedroid */
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid init failed: %s", esp_err_to_name(ret));
        return false;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid enable failed: %s", esp_err_to_name(ret));
        return false;
    }

    /* Set device name */
    esp_bt_gap_set_device_name(device_name);

    /* SSP configuration */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_NONE;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));

    /* Legacy pairing */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);

    /* Register GAP callback */
    esp_bt_gap_register_callback(gap_event_handler);

    s_stack_initialized = true;
    ESP_LOGI(TAG, "BT Classic initialized: \"%s\"", device_name);
    uint8_t own_bda[BT_CLASSIC_BD_ADDR_LEN];
    bt_classic_hal_get_own_bda(own_bda);
    ESP_LOGI(TAG, "Own BDA: %02x:%02x:%02x:%02x:%02x:%02x",
             own_bda[0], own_bda[1], own_bda[2],
             own_bda[3], own_bda[4], own_bda[5]);
    return true;
}

void bt_classic_hal_register_conn_cb(bt_classic_conn_state_cb_t cb) {
    s_conn_cb = cb;
}

bool bt_classic_hal_init_a2dp_sink(bt_classic_sink_data_cb_t data_cb) {
    if (!s_stack_initialized) {
        ESP_LOGE(TAG, "BT stack not initialized");
        return false;
    }
    if (s_is_source) {
        ESP_LOGE(TAG, "Cannot init sink: source already active");
        return false;
    }

    s_sink_data_cb = data_cb;
    s_is_sink = true;

    esp_a2d_register_callback(a2d_event_handler);
    esp_err_t ret = esp_a2d_sink_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "A2DP sink init failed: %s", esp_err_to_name(ret));
        return false;
    }

    esp_a2d_sink_register_data_callback(a2d_sink_data_callback);

    /* Make discoverable */
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    ESP_LOGI(TAG, "A2DP Sink initialized");
    return true;
}

bool bt_classic_hal_init_a2dp_source(bt_classic_source_state_cb_t state_cb,
                                     bt_classic_source_data_cb_t data_cb) {
    if (!s_stack_initialized) {
        ESP_LOGE(TAG, "BT stack not initialized");
        return false;
    }
    if (s_is_sink) {
        ESP_LOGE(TAG, "Cannot init source: sink already active");
        return false;
    }

    s_source_state_cb = state_cb;
    s_source_data_cb = data_cb;
    s_is_source = true;
    s_src_state = SRC_STATE_IDLE;

    esp_a2d_register_callback(a2d_event_handler);
    esp_a2d_source_init();
    esp_a2d_source_register_data_callback(a2d_source_data_callback);

    /* Non-discoverable for source role */
    esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);

    ESP_LOGI(TAG, "A2DP Source initialized");
    return true;
}

bool bt_classic_hal_source_start_discovery(const char *target_name) {
    if (!s_is_source || !target_name) return false;
    if (s_src_state == SRC_STATE_DISCOVERING || s_src_state == SRC_STATE_SCANNING) return false;

    strncpy(s_target_name, target_name, ESP_BT_GAP_MAX_BDNAME_LEN);
    source_connect_to_target();
    return true;
}

bool bt_classic_hal_source_start_scan(bt_classic_source_scan_cb_t scan_cb) {
    if (!s_is_source || !scan_cb) return false;
    if (s_src_state == SRC_STATE_DISCOVERING || s_src_state == SRC_STATE_SCANNING) return false;
    s_source_scan_cb = scan_cb;
    s_src_state = SRC_STATE_SCANNING;
    esp_err_t ret = esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
    if (ret != ESP_OK) {
        s_src_state = SRC_STATE_IDLE;
        s_source_scan_cb = NULL;
        return false;
    }
    return true;
}

bool bt_classic_hal_source_connect(const uint8_t *bda) {
    if (!s_is_source || !bda) return false;

    memcpy(s_peer_bda, bda, BT_CLASSIC_BD_ADDR_LEN);
    s_src_state = SRC_STATE_CONNECTING;
    esp_a2d_source_connect(s_peer_bda);
    return true;
}

int bt_classic_hal_source_write(const uint8_t *data, int32_t len) {
    if (!s_is_source || !s_a2d_connected || !data || len <= 0) {
        return -1;
    }
    /* A2DP source expects the callback to fill the buffer.
     * For relay mode, we store data in a ring buffer and the
     * data callback reads from it. For now, direct write is
     * not supported in IDF 6.x A2DP source (data is pull-based).
     * This is handled at the app level. */
    return len;
}

void bt_classic_hal_disconnect(void) {
    if (!s_a2d_connected) return;
    if (s_is_source) {
        esp_a2d_source_disconnect(s_peer_bda);
    } else if (s_is_sink) {
        esp_a2d_sink_disconnect(s_peer_bda);
    }
}

void bt_classic_hal_set_discoverable(void) {
    if (s_is_sink) {
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    }
}

bool bt_classic_hal_is_connected(void) {
    return s_a2d_connected;
}

void bt_classic_hal_get_own_bda(uint8_t *bda) {
    if (bda) {
        const esp_bd_addr_t *addr = (const esp_bd_addr_t *)esp_bt_dev_get_address();
        memcpy(bda, *addr, BT_CLASSIC_BD_ADDR_LEN);
    }
}

bool bt_classic_hal_get_peer_bda(uint8_t *bda) {
    if (!bda || !s_a2d_connected) return false;
    memcpy(bda, s_peer_bda, BT_CLASSIC_BD_ADDR_LEN);
    return true;
}

#else /* !CONFIG_BT_ENABLED — stubs when BT is disabled */

static const char *TAG = "bt_classic_hal";

bool bt_classic_hal_init(const char *device_name) {
    ESP_LOGW(TAG, "BT not enabled in sdkconfig");
    return false;
}
bool bt_classic_hal_init_a2dp_sink(bt_classic_sink_data_cb_t data_cb) { return false; }
bool bt_classic_hal_init_a2dp_source(bt_classic_source_state_cb_t state_cb,
                                     bt_classic_source_data_cb_t data_cb) { return false; }
bool bt_classic_hal_source_start_discovery(const char *target_name) { return false; }
bool bt_classic_hal_source_start_scan(bt_classic_source_scan_cb_t scan_cb) { return false; }
bool bt_classic_hal_source_connect(const uint8_t *bda) { return false; }
int bt_classic_hal_source_write(const uint8_t *data, int32_t len) { return -1; }
void bt_classic_hal_disconnect(void) {}
void bt_classic_hal_set_discoverable(void) {}
bool bt_classic_hal_is_connected(void) { return false; }
void bt_classic_hal_get_own_bda(uint8_t *bda) {}
bool bt_classic_hal_get_peer_bda(uint8_t *bda) { return false; }
void bt_classic_hal_register_conn_cb(bt_classic_conn_state_cb_t cb) {}

#endif /* CONFIG_BT_ENABLED */

/*==================[end of file]============================================*/
