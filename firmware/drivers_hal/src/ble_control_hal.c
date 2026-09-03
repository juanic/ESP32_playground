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

/* ======================== UUIDs (Nordic UART Service) ======================== */

/* 128-bit UUIDs stored little-endian (LSB first), as required by esp_bt_uuid_t. */
static const uint8_t nus_svc_uuid128[16] = {
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e,
}; /* 6E400001-B5A3-F393-E0A9-E50E24DCCA9E */
static const uint8_t nus_rx_uuid128[16] = {
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x02, 0x00, 0x40, 0x6e,
}; /* 6E400002-B5A3-F393-E0A9-E50E24DCCA9E */
static const uint8_t nus_tx_uuid128[16] = {
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x03, 0x00, 0x40, 0x6e,
}; /* 6E400003-B5A3-F393-E0A9-E50E24DCCA9E */

static esp_bt_uuid_t uuid_svc = {.len = ESP_UUID_LEN_128};
static esp_bt_uuid_t uuid_rx  = {.len = ESP_UUID_LEN_128};
static esp_bt_uuid_t uuid_tx  = {.len = ESP_UUID_LEN_128};
static esp_bt_uuid_t uuid_ccc = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x2902};

/* ======================== State ======================== */

static char s_device_name[32] = "Audio Relay CTL";

static esp_gatt_if_t s_gatt_if = ESP_GATT_IF_NONE;
static uint16_t      s_conn_id  = 0;
static bool          s_connected = false;
static esp_bd_addr_t s_remote_bda = {0};

static uint16_t s_svc_handle   = 0;
static uint16_t s_rx_handle    = 0;
static uint16_t s_tx_handle    = 0;
static uint16_t s_tx_ccc       = 0;
static uint16_t s_tx_ccc_val   = 0;

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

/* Flags only; device name appended at runtime (capped to 31 bytes). */
static const uint8_t adv_raw_prefix[] = {
    0x02, 0x01, 0x06,                        /* Flags */
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
    /* false = notification (no CCCD write / ACK required from client) */
    esp_ble_gatts_send_indicate(s_gatt_if, s_conn_id, char_handle,
                                (uint16_t)len, pkt, false);
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
    /* Flags + device name (capped so the packet stays within 31 bytes). */
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

    /* 128-bit service UUID goes in the scan response (adv packet has no room). */
    uint8_t scan_rsp[18];
    scan_rsp[0] = 0x11;           /* length: type + 16 bytes */
    scan_rsp[1] = 0x07;           /* Complete List of 128-bit Service UUIDs */
    memcpy(&scan_rsp[2], nus_svc_uuid128, sizeof(nus_svc_uuid128));

    esp_err_t rc = esp_ble_gap_config_adv_data_raw(adv, adv_len);
    if (rc != ESP_OK) {
        ESP_LOGE(TAG, "Config adv failed: %s", esp_err_to_name(rc));
    }
    rc = esp_ble_gap_config_scan_rsp_data_raw(scan_rsp, sizeof(scan_rsp));
    if (rc != ESP_OK) {
        ESP_LOGE(TAG, "Config scan rsp failed: %s", esp_err_to_name(rc));
    }
    /* Advertising starts once both raw data sets report completion. */
}

void ble_control_hal_send_rsp(const char *line) {
    send_notify(s_tx_handle, s_tx_ccc_val, line);
}

void ble_control_hal_send_evt(const char *line) {
    send_notify(s_tx_handle, s_tx_ccc_val, line);
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

static bool s_adv_data_ready = false;
static bool s_scan_rsp_ready = false;

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        s_adv_data_ready = true;
        if (s_scan_rsp_ready) {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        s_scan_rsp_ready = true;
        if (s_adv_data_ready) {
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
    memcpy(uuid_svc.uuid.uuid128, nus_svc_uuid128, sizeof(nus_svc_uuid128));
    memcpy(uuid_rx.uuid.uuid128, nus_rx_uuid128, sizeof(nus_rx_uuid128));
    memcpy(uuid_tx.uuid.uuid128, nus_tx_uuid128, sizeof(nus_tx_uuid128));

    esp_gatt_srvc_id_t srvc_id = {
        .id.uuid = uuid_svc,
        .id.inst_id = 0,
        .is_primary = true,
    };
    /* NUS has 1 service decl + 2 char decls + 2 value attrs + 1 CCCD = 6 handles. */
    esp_ble_gatts_create_service(s_gatt_if, &srvc_id, 8);
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

        /* RX (write) */
        esp_ble_gatts_add_char(s_svc_handle, &uuid_rx,
                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                               ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_WRITE_NR,
                               NULL, NULL);
        /* TX (notify) — carries both RSP and EVT lines */
        esp_ble_gatts_add_char(s_svc_handle, &uuid_tx,
                               ESP_GATT_PERM_READ,
                               ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                               NULL, NULL);
        break;
    }

    case ESP_GATTS_ADD_CHAR_EVT: {
        uint16_t handle = param->add_char.attr_handle;
        const uint8_t *uuid128 = param->add_char.char_uuid.uuid.uuid128;
        if (memcmp(uuid128, nus_rx_uuid128, sizeof(nus_rx_uuid128)) == 0) {
            s_rx_handle = handle;
        } else if (memcmp(uuid128, nus_tx_uuid128, sizeof(nus_tx_uuid128)) == 0) {
            s_tx_handle = handle;
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
        s_tx_ccc = handle;
        esp_ble_gatts_start_service(s_svc_handle);
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
        s_tx_ccc_val = 0;
        s_line_len = 0;
        ESP_LOGI(TAG, "Client disconnected, restarting advertising");
        esp_ble_gap_start_advertising(&adv_params);
        break;
    }

    case ESP_GATTS_WRITE_EVT: {
        uint16_t handle = param->write.handle;
        /* CCCD update */
        if (!param->write.is_prep && param->write.len == 2 && handle == s_tx_ccc) {
            uint16_t value = param->write.value[0] | (param->write.value[1] << 8);
            s_tx_ccc_val = (value & 0x0003) ? value : 0;
        } else if (handle == s_rx_handle) {
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
