/**
 * @file ble_control_hal.c
 * @brief BLE GATT server text-line transport for device control (Bluedroid).
 */

#include "ble_control_hal.h"
#include "sdkconfig.h"

#if CONFIG_BT_ENABLED && CONFIG_BT_BLE_ENABLED && CONFIG_BT_GATTS_ENABLE

#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include "esp_bt_defs.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "ble_control_hal";

/* ======================== UUIDs ======================== */

static esp_bt_uuid_t uuid_svc = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = BLE_CONTROL_SERVICE_UUID};
static esp_bt_uuid_t uuid_cmd = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = BLE_CONTROL_CMD_UUID};
static esp_bt_uuid_t uuid_rsp = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = BLE_CONTROL_RSP_UUID};
static esp_bt_uuid_t uuid_evt = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = BLE_CONTROL_EVT_UUID};
static esp_bt_uuid_t uuid_ccc = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x2902};

/* ======================== State ======================== */

static char s_device_name[32] = "Audio Relay CTL";

static esp_gatt_if_t s_gatt_if = ESP_GATT_IF_NONE;
static uint16_t      s_conn_id  = 0;
static bool          s_connected = false;
static esp_bd_addr_t s_remote_bda = {0};

static uint16_t s_svc_handle   = 0;
static uint16_t s_cmd_handle   = 0;
static uint16_t s_rsp_handle   = 0;
static uint16_t s_evt_handle   = 0;
static uint16_t s_rsp_ccc      = 0;
static uint16_t s_evt_ccc      = 0;
static uint16_t s_rsp_ccc_val  = 0;
static uint16_t s_evt_ccc_val  = 0;

static ble_control_cmd_cb_t s_cmd_cb = NULL;
static void *s_cmd_ctx = NULL;

static char     s_line_buf[BLE_CONTROL_MAX_LINE_LEN];
static uint16_t s_line_len = 0;

/* ======================== Forward declarations ======================== */

static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param);
static void gap_event_handler(esp_gap_ble_cb_event_t event,
                              esp_ble_gap_cb_param_t *param);

/* ======================== Advertising ======================== */

static esp_ble_adv_params_t adv_params = {
    .adv_int_min       = 0x20,
    .adv_int_max       = 0x40,
    .adv_type          = ADV_TYPE_IND,
    .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
    .channel_map       = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

/* Flags + 16-bit service UUID, name appended at runtime (capped to 31 bytes). */
static const uint8_t adv_raw_prefix[] = {
    0x02, 0x01, 0x06,                        /* Flags */
    0x03, 0x07, 0xC0, 0xFF,                  /* Complete 16-bit UUIDs: 0xFFC0 */
};

/* ======================== Send helpers ======================== */

static void send_notify(uint16_t char_handle, uint16_t ccc_val, const char *line) {
    if (!s_connected || (ccc_val & 0x0001) == 0) {
        return;
    }
    size_t len = strlen(line);
    if (len > BLE_CONTROL_MAX_LINE_LEN) {
        len = BLE_CONTROL_MAX_LINE_LEN;
    }
    uint8_t pkt[BLE_CONTROL_MAX_PKT_LEN];
    memcpy(pkt, line, len);
    bool more = false;
    if (len >= BLE_CONTROL_MAX_PKT_LEN) {
        more = true;
    }
    esp_ble_gatts_send_indicate(s_gatt_if, s_conn_id, char_handle,
                                (uint16_t)len, pkt, more);
}

/* ======================== Public API ======================== */

bool ble_control_hal_init(const char *device_name, ble_control_cmd_cb_t cmd_cb, void *ctx) {
    ESP_LOGI(TAG, "Initializing BLE Control HAL...");

    s_cmd_cb = cmd_cb;
    s_cmd_ctx = ctx;

    esp_err_t ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GATTS register callback failed: %s", esp_err_to_name(ret));
        return false;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GAP register callback failed: %s", esp_err_to_name(ret));
        return false;
    }

    if (device_name) {
        strncpy(s_device_name, device_name, sizeof(s_device_name) - 1);
    }
    esp_ble_gap_set_device_name(s_device_name);
    esp_ble_gatt_set_local_mtu(517);

    /* Register GATTS app — triggers ESP_GATTS_REG_EVT → s_gatt_if */
    esp_ble_gatts_app_register(0x02);

    ESP_LOGI(TAG, "BLE Control HAL initialized: \"%s\"", s_device_name);
    return true;
}

void ble_control_hal_start(void) {
    /* Flag + service UUID prefix, then device name (capped). */
    uint8_t name[32];
    size_t n = strlen(s_device_name);
    if (n > 20) n = 20;
    name[0] = (uint8_t)(n + 1);
    name[1] = 0x09;              /* Complete Local Name */
    memcpy(&name[2], s_device_name, n);
    uint16_t name_len = (uint16_t)(n + 2);

    uint8_t adv[32];
    memcpy(adv, adv_raw_prefix, sizeof(adv_raw_prefix));
    memcpy(&adv[sizeof(adv_raw_prefix)], name, name_len);
    uint16_t adv_len = (uint16_t)(sizeof(adv_raw_prefix) + name_len);

    esp_err_t rc = esp_ble_gap_config_adv_data_raw(adv, adv_len);
    if (rc != ESP_OK) {
        ESP_LOGE(TAG, "Config adv failed: %s", esp_err_to_name(rc));
    }
    /* Advertising starts on ADV_DATA_RAW_SET_COMPLETE_EVT */
}

void ble_control_hal_send_rsp(const char *line) {
    send_notify(s_rsp_handle, s_rsp_ccc_val, line);
}

void ble_control_hal_send_evt(const char *line) {
    send_notify(s_evt_handle, s_evt_ccc_val, line);
}

bool ble_control_hal_is_connected(void) {
    return s_connected;
}

void ble_control_hal_disconnect(void) {
    if (s_connected) {
        esp_ble_gap_disconnect(s_remote_bda);
    }
}

/* ======================== Line framing ======================== */

static void feed_line(const uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        uint8_t c = data[i];
        if (c == '\n' || c == '\r') {
            if (s_line_len > 0) {
                s_line_buf[s_line_len] = '\0';
                uint16_t line_len = s_line_len;
                s_line_len = 0;
                ESP_LOGI(TAG, "CMD: %s", s_line_buf);
                if (s_cmd_cb) {
                    s_cmd_cb(s_line_buf, line_len, s_cmd_ctx);
                }
            }
            continue;
        }
        if (s_line_len < (BLE_CONTROL_MAX_LINE_LEN - 1)) {
            s_line_buf[s_line_len++] = (char)c;
        }
    }
}

/* ======================== GAP callback ======================== */

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        if (param->adv_data_raw_cmpl.status == ESP_BT_STATUS_SUCCESS) {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        break;
    case ESP_GAP_BLE_SEC_REQ_EVT:
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT:
        ESP_LOGI(TAG, "BLE auth complete, success=%d", param->ble_security.auth_cmpl.success);
        break;
    default:
        break;
    }
}

/* ======================== GATTS callback ======================== */

static void create_control_service(void) {
    esp_gatt_srvc_id_t srvc_id = {
        .id.uuid = uuid_svc,
        .id.inst_id = 0,
        .is_primary = true,
    };
    esp_ble_gatts_create_service(s_gatt_if, &srvc_id, 12);
}

static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param) {
    switch (event) {

    case ESP_GATTS_REG_EVT: {
        if (param->reg.status == ESP_GATT_OK) {
            s_gatt_if = gatts_if;
            create_control_service();
        }
        break;
    }

    case ESP_GATTS_CREATE_EVT: {
        s_svc_handle = param->create.service_handle;
        ESP_LOGI(TAG, "Control service created, handle=%d", s_svc_handle);

        /* CMD (write) */
        esp_ble_gatts_add_char(s_svc_handle, &uuid_cmd,
                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                               ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_WRITE_NR,
                               NULL, NULL);
        /* RSP (notify) */
        esp_ble_gatts_add_char(s_svc_handle, &uuid_rsp,
                               ESP_GATT_PERM_READ,
                               ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                               NULL, NULL);
        /* EVT (notify) */
        esp_ble_gatts_add_char(s_svc_handle, &uuid_evt,
                               ESP_GATT_PERM_READ,
                               ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                               NULL, NULL);
        break;
    }

    case ESP_GATTS_ADD_CHAR_EVT: {
        uint16_t handle = param->add_char.attr_handle;
        uint16_t uuid = param->add_char.char_uuid.uuid.uuid16;
        if (uuid == BLE_CONTROL_CMD_UUID) {
            s_cmd_handle = handle;
        } else if (uuid == BLE_CONTROL_RSP_UUID) {
            s_rsp_handle = handle;
            static uint8_t ccc_zero[] = {0x00, 0x00};
            esp_attr_value_t ccc_val = {.attr_max_len = 2, .attr_len = 2, .attr_value = ccc_zero};
            esp_ble_gatts_add_char_descr(s_svc_handle, &uuid_ccc,
                                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                         &ccc_val, NULL);
        } else if (uuid == BLE_CONTROL_EVT_UUID) {
            s_evt_handle = handle;
            static uint8_t ccc_zero[] = {0x00, 0x00};
            esp_attr_value_t ccc_val = {.attr_max_len = 2, .attr_len = 2, .attr_value = ccc_zero};
            esp_ble_gatts_add_char_descr(s_svc_handle, &uuid_ccc,
                                         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                         &ccc_val, NULL);
        }
        break;
    }

    case ESP_GATTS_ADD_CHAR_DESCR_EVT: {
        uint16_t handle = param->add_char_descr.attr_handle;
        /* First CCC = RSP, second CCC = EVT */
        if (s_rsp_ccc == 0) {
            s_rsp_ccc = handle;
        } else if (s_evt_ccc == 0) {
            s_evt_ccc = handle;
            esp_ble_gatts_start_service(s_svc_handle);
        }
        break;
    }

    case ESP_GATTS_START_EVT:
        break;

    case ESP_GATTS_CONNECT_EVT: {
        s_conn_id = param->connect.conn_id;
        memcpy(s_remote_bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        s_connected = true;
        ESP_LOGI(TAG, "Client connected, conn_id=%d", s_conn_id);

        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        conn_params.latency = 0;
        conn_params.max_int = 0x20;
        conn_params.min_int = 0x10;
        conn_params.timeout = 400;
        esp_ble_gap_update_conn_params(&conn_params);
        break;
    }

    case ESP_GATTS_DISCONNECT_EVT:
    case ESP_GATTS_CLOSE_EVT: {
        s_connected = false;
        s_conn_id = 0;
        s_rsp_ccc_val = 0;
        s_evt_ccc_val = 0;
        s_line_len = 0;
        ESP_LOGI(TAG, "Client disconnected, restarting advertising");
        esp_ble_gap_start_advertising(&adv_params);
        break;
    }

    case ESP_GATTS_WRITE_EVT: {
        uint16_t handle = param->write.handle;
        /* CCCD update */
        if (!param->write.is_prep && param->write.len == 2 &&
            (handle == s_rsp_ccc || handle == s_evt_ccc)) {
            uint16_t value = param->write.value[0] | (param->write.value[1] << 8);
            if (handle == s_rsp_ccc) {
                s_rsp_ccc_val = (value & 0x0003) ? value : 0;
            } else {
                s_evt_ccc_val = (value & 0x0003) ? value : 0;
            }
        } else if (handle == s_cmd_handle) {
            feed_line(param->write.value, param->write.len);
        }
        break;
    }

    default:
        break;
    }
}

#else /* !(CONFIG_BT_ENABLED && BLE && GATTS) — stubs */

#include "esp_log.h"
static const char *TAG = "ble_control_hal";

bool ble_control_hal_init(const char *device_name, ble_control_cmd_cb_t cmd_cb, void *ctx) {
    ESP_LOGW(TAG, "BLE control not enabled in sdkconfig");
    return false;
}
void ble_control_hal_start(void) {}
void ble_control_hal_send_rsp(const char *line) {}
void ble_control_hal_send_evt(const char *line) {}
bool ble_control_hal_is_connected(void) { return false; }
void ble_control_hal_disconnect(void) {}

#endif /* CONFIG_BT_ENABLED && BLE && GATTS */

/*==================[end of file]============================================*/
