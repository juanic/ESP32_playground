/**
 * @file ble_hid_kb.c
 * @brief BLE HID Keyboard + Media Keys driver para ESP-IDF 6.x (Bluedroid).
 */

#include "ble_hid_kb.h"
#include "sdkconfig.h"

#if CONFIG_BT_ENABLED

#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include "esp_bt_defs.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "ble_hid_kb";

/* ======================== HID Report Descriptor ======================== */

#define REPORT_ID_KEYBOARD   0x01
#define REPORT_ID_MEDIA      0x02

static const uint8_t hid_report_map[] = {
    /* ---- Keyboard ---- */
    0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,        // USAGE (Keyboard)
    0xA1, 0x01,        // COLLECTION (Application)
    0x85, REPORT_ID_KEYBOARD,  //   REPORT_ID (1)
    0x05, 0x07,        //   USAGE_PAGE (Keyboard/Keypad)
    0x19, 0xE0,        //   USAGE_MINIMUM (Left Control)
    0x29, 0xE7,        //   USAGE_MAXIMUM (Right GUI)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,        //   REPORT_SIZE (1)
    0x95, 0x08,        //   REPORT_COUNT (8)
    0x81, 0x02,        //   INPUT (Data,Var,Abs)
    0x95, 0x01,        //   REPORT_COUNT (1)
    0x75, 0x08,        //   REPORT_SIZE (8)
    0x81, 0x01,        //   INPUT (Const,Array)
    /* LED output report */
    0x95, 0x05,        //   REPORT_COUNT (5)
    0x75, 0x01,        //   REPORT_SIZE (1)
    0x05, 0x08,        //   USAGE_PAGE (LEDs)
    0x19, 0x01,        //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,        //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,        //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,        //   REPORT_COUNT (1)
    0x75, 0x03,        //   REPORT_SIZE (3)
    0x91, 0x01,        //   OUTPUT (Const,Array)
    /* 6 key slots */
    0x95, 0x06,        //   REPORT_COUNT (6)
    0x75, 0x08,        //   REPORT_SIZE (8)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x25, 0x65,        //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,        //   USAGE_PAGE (Keyboard/Keypad)
    0x19, 0x00,        //   USAGE_MINIMUM (0)
    0x29, 0x65,        //   USAGE_MAXIMUM (101)
    0x81, 0x00,        //   INPUT (Data,Array)
    0xC0,              // END_COLLECTION

    /* ---- Media Keys ---- */
    0x05, 0x0C,        // USAGE_PAGE (Consumer)
    0x09, 0x01,        // USAGE (Consumer Control)
    0xA1, 0x01,        // COLLECTION (Application)
    0x85, REPORT_ID_MEDIA,     //   REPORT_ID (2)
     0x05, 0x0C,        //   USAGE_PAGE (Consumer)
     0x15, 0x00,        //   LOGICAL_MINIMUM (0)
     0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
     0x75, 0x01,        //   REPORT_SIZE (1)
     0x95, 0x10,        //   REPORT_COUNT (16)
     0x0A, 0xB5, 0x00,  //   USAGE (Scan Next Track)
     0x0A, 0xB6, 0x00,  //   USAGE (Scan Previous Track)
     0x0A, 0xB7, 0x00,  //   USAGE (Stop)
     0x0A, 0xCD, 0x00,  //   USAGE (Play/Pause)
     0x0A, 0xE2, 0x00,  //   USAGE (Mute)
     0x0A, 0xE9, 0x00,  //   USAGE (Volume Increment)
     0x0A, 0xEA, 0x00,  //   USAGE (Volume Decrement)
     0x0A, 0x23, 0x02,  //   USAGE (WWW Home)
     0x0A, 0x94, 0x01,  //   USAGE (My Computer)
     0x0A, 0x92, 0x01,  //   USAGE (Calculator)
     0x0A, 0x2A, 0x02,  //   USAGE (WWW Bookmarks)
     0x0A, 0x21, 0x02,  //   USAGE (WWW Search)
     0x0A, 0x26, 0x02,  //   USAGE (WWW Stop)
     0x0A, 0x24, 0x02,  //   USAGE (WWW Back)
     0x0A, 0x83, 0x01,  //   USAGE (Media Selection)
     0x0A, 0x8A, 0x01,  //   USAGE (Mail)
     0x81, 0x02,        //   INPUT (Data,Var,Abs)
     0xC0,              // END_COLLECTION
};

/* ======================== GATT UUIDs (as esp_bt_uuid_t) ======================== */

static esp_bt_uuid_t uuid_hid_service         = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x1812};
static esp_bt_uuid_t uuid_hid_information     = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x2A4A};
static esp_bt_uuid_t uuid_hid_report_map      = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x2A4B};
static esp_bt_uuid_t uuid_hid_control_point   = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x2A4C};
static esp_bt_uuid_t uuid_hid_report          = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x2A4D};
static esp_bt_uuid_t uuid_hid_protocol_mode   = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x2A4E};
static esp_bt_uuid_t uuid_battery_service     = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x180F};
static esp_bt_uuid_t uuid_battery_level       = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x2A19};
static esp_bt_uuid_t uuid_devinfo_service     = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x180A};
static esp_bt_uuid_t uuid_manufacturer_name   = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x2A29};
static esp_bt_uuid_t uuid_ccc                 = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x2902};
static esp_bt_uuid_t uuid_report_ref          = {.len = ESP_UUID_LEN_16, .uuid.uuid16 = 0x2908};

/* ======================== State ======================== */

static char s_device_name[31] = "ESP32 HID KB";

static esp_gatt_if_t s_gatt_if = ESP_GATT_IF_NONE;
static uint16_t      s_conn_id = 0;
static bool          s_connected = false;
static esp_bd_addr_t s_remote_bda = {0};

static uint16_t s_hid_svc_handle = 0;
static uint16_t s_hh_char_handles[7];
static uint16_t s_hid_descr_handles[4]; /* CCC_kbd, Ref_kbd, CCC_media, Ref_media */
static uint16_t s_batt_svc_handle = 0;
static uint16_t s_batt_char_handle = 0;
static uint16_t s_devinfo_svc_handle = 0;
static uint16_t s_devinfo_char_handle = 0;

static uint16_t s_keyboard_ccc = 0;
static uint16_t s_media_ccc = 0;

static hid_keyboard_report_t s_kbd_report;
static hid_media_report_t    s_media_report;

/* Advertising */
static esp_ble_adv_params_t adv_params = {
    .adv_int_min       = 0x20,
    .adv_int_max       = 0x40,
    .adv_type          = ADV_TYPE_IND,
    .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
    .channel_map       = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

/* Raw advertising data: flags + local name + HID service UUID + appearance.
 * The legacy esp_ble_gap_config_adv_data API rejects 16-bit service UUIDs
 * (its service_uuid_len must be a multiple of 16 bytes / 128-bit UUIDs), so we
 * assemble the ADV payload manually with esp_ble_gap_config_adv_data_raw. */
static uint8_t adv_raw_data[] = {
    0x02, 0x01, 0x06,                                  /* Flags: LE General Disc + BR/EDR not supported */
    0x03, 0x07, 0x12, 0x18,                            /* Complete List of 16-bit Service UUIDs: HID (0x1812) */
    0x03, 0x19, 0xC1, 0x03,                            /* Appearance: 0x03C1 (HID Keyboard) */
    /* Complete local name "ESP32 HID KB *" appended below at runtime */
};

/* ======================== Forward declarations ======================== */

static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param);
static void gap_event_handler(esp_gap_ble_cb_event_t event,
                              esp_ble_gap_cb_param_t *param);
static void send_keyboard_report(void);
static void send_media_report(void);

/* ======================== Helpers ======================== */

static void create_hid_service(void);
static void create_battery_service(void);
static void create_devinfo_service(void);

/* ======================== Public API ======================== */

bool ble_hid_kb_init(const ble_hid_kb_config_t *config) {
    if (!config) return false;

    ESP_LOGI(TAG, "Initializing BLE HID Keyboard...");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BT controller init failed: %s", esp_err_to_name(ret));
        return false;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BT controller enable failed: %s", esp_err_to_name(ret));
        return false;
    }

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

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GATTS register callback failed: %s", esp_err_to_name(ret));
        return false;
    }

    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GAP register callback failed: %s", esp_err_to_name(ret));
        return false;
    }

    /* SMP security configuration — bonding + MITM, secure connections.
     * Windows empareja teclados BLE sólo con Secure Connections + MITM
     * (para prevenir "keyboard injection"); sin el bit MITM puede rechazar
     * el pairing. Alineado con el driver de referencia (ESP_LE_AUTH_REQ_SC_MITM_BOND). */
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
    uint8_t key_size = 16;
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t resp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(auth_req));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(iocap));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(key_size));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(init_key));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &resp_key, sizeof(resp_key));

    strncpy(s_device_name, config->device_name, sizeof(s_device_name) - 1);
    esp_ble_gap_set_device_name(config->device_name);
    esp_ble_gatt_set_local_mtu(517);

    /* Register GATTS app — triggers ESP_GATTS_REG_EVT → s_gatt_if */
    esp_ble_gatts_app_register(0x01);

    ESP_LOGI(TAG, "BLE HID initialized: \"%s\"", config->device_name);
    return true;
}

bool ble_hid_kb_start(void) {
    /* Build complete ADV payload: flags + HID UUID + appearance, then name. */
    uint8_t name[31];
    int name_len = 0;
    const char *dn = s_device_name[0] ? s_device_name : "ESP32 HID KB";
    size_t n = strlen(dn);
    /* ADV payload is capped at 31 bytes total: 11 fixed + (2 name header + n) */
    if (n > 18) n = 18;
    name[0] = (uint8_t)(n + 1);  /* AD structure length = 1(type) + n */
    name[1] = 0x09;              /* Complete Local Name */
    memcpy(&name[2], dn, n);
    name_len = n + 2;

    uint8_t adv[32];
    memcpy(adv, adv_raw_data, sizeof(adv_raw_data));
    uint8_t adv_len = (uint8_t)(sizeof(adv_raw_data) + name_len);

    esp_err_t rc = esp_ble_gap_config_adv_data_raw(adv, adv_len);
    if (rc != ESP_OK) {
        ESP_LOGE(TAG, "Config adv data raw failed: %s", esp_err_to_name(rc));
        return false;
    }
    ESP_LOGI(TAG, "Advertising data set (%d bytes, adv_len=%d)", (int)sizeof(adv_raw_data), adv_len);
    return true;
}

void ble_hid_kb_stop(void) {
    if (s_connected) {
        esp_ble_gap_disconnect(s_remote_bda);
    }
    esp_ble_gap_stop_advertising();
    s_connected = false;
}

bool ble_hid_kb_is_connected(void) {
    return s_connected;
}

bool ble_hid_kb_wait_connection(uint32_t timeout_ms) {
    int64_t start = esp_timer_get_time();
    while (!s_connected) {
        if (timeout_ms > 0) {
            int64_t elapsed = (esp_timer_get_time() - start) / 1000;
            if (elapsed >= (int64_t)timeout_ms) return false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return true;
}

bool ble_hid_kb_send_media(const media_key_t key) {
    if (!s_connected) return false;
    memcpy(s_media_report, key, sizeof(hid_media_report_t));
    send_media_report();
    vTaskDelay(pdMS_TO_TICKS(10));
    memset(s_media_report, 0, sizeof(hid_media_report_t));
    send_media_report();
    return true;
}

bool ble_hid_kb_press_media(const media_key_t key) {
    if (!s_connected) return false;
    memcpy(s_media_report, key, sizeof(hid_media_report_t));
    send_media_report();
    return true;
}

bool ble_hid_kb_release_media(void) {
    if (!s_connected) return false;
    memset(s_media_report, 0, sizeof(hid_media_report_t));
    send_media_report();
    return true;
}

bool ble_hid_kb_send_key(uint8_t hid_keycode) {
    return ble_hid_kb_press_key(0, hid_keycode);
}

bool ble_hid_kb_press_key(uint8_t modifiers, uint8_t keycode) {
    if (!s_connected || s_keyboard_ccc == 0) return false;
    s_kbd_report.modifiers = modifiers;
    s_kbd_report.keys[0] = keycode;
    send_keyboard_report();
    return true;
}

void ble_hid_kb_release_all(void) {
    if (!s_connected) return;
    memset(&s_kbd_report, 0, sizeof(s_kbd_report));
    memset(s_media_report, 0, sizeof(s_media_report));
    send_keyboard_report();
    send_media_report();
}

bool ble_hid_kb_send_text(const char *text) {
    if (!s_connected || !text) return false;

    static const uint8_t ascii_to_hid[128] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0x2A, 0x2B, 0x28, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x29, 0, 0, 0, 0,
        0x2C, 0x1E|0x80, 0x34|0x80, 0x20|0x80, 0x21|0x80, 0x22|0x80,
        0x24|0x80, 0x34, 0x26|0x80, 0x27|0x80, 0x25|0x80, 0x2E|0x80,
        0x36, 0x2D, 0x37, 0x38,
        0x27, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
        0x33|0x80, 0x33, 0x36|0x80, 0x2E, 0x37|0x80, 0x38|0x80, 0x1F|0x80,
        0x04|0x80, 0x05|0x80, 0x06|0x80, 0x07|0x80, 0x08|0x80, 0x09|0x80,
        0x0A|0x80, 0x0B|0x80, 0x0C|0x80, 0x0D|0x80, 0x0E|0x80, 0x0F|0x80,
        0x10|0x80, 0x11|0x80, 0x12|0x80, 0x13|0x80, 0x14|0x80, 0x15|0x80,
        0x16|0x80, 0x17|0x80, 0x18|0x80, 0x19|0x80, 0x1A|0x80, 0x1B|0x80,
        0x1C|0x80, 0x1D|0x80, 0x1E|0x80, 0x2F, 0x31, 0x30,
        0x35|0x80, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
        0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
        0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x2F|0x80, 0x31|0x80, 0x30|0x80,
        0x35|0x80, 0,
    };

    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        if (c == '\r') continue;
        uint8_t code = (c < 128) ? ascii_to_hid[(uint8_t)c] : 0;
        if (code == 0) continue;

        bool shift = (code & 0x80) != 0;
        code &= 0x7F;

        s_kbd_report.modifiers = shift ? KEY_MOD_LSHIFT : 0;
        s_kbd_report.keys[0] = code;
        send_keyboard_report();
        vTaskDelay(pdMS_TO_TICKS(8));

        s_kbd_report.modifiers = 0;
        s_kbd_report.keys[0] = 0;
        send_keyboard_report();
        vTaskDelay(pdMS_TO_TICKS(8));
    }
    return true;
}

void ble_hid_kb_set_battery(uint8_t level) {
    if (s_batt_char_handle == 0) return;
    esp_ble_gatts_set_attr_value(s_batt_char_handle, 1, &level);
}

/* ======================== Internal ======================== */

static void send_keyboard_report(void) {
    if (!s_connected) {
        ESP_LOGW(TAG, "KBD send skipped: not connected");
        return;
    }
    uint8_t payload[sizeof(hid_keyboard_report_t) + 1];
    payload[0] = REPORT_ID_KEYBOARD;
    memcpy(payload + 1, &s_kbd_report, sizeof(hid_keyboard_report_t));
    esp_err_t rc = esp_ble_gatts_send_indicate(s_gatt_if, s_conn_id,
                                               s_hh_char_handles[4],
                                               sizeof(payload), payload, false);
    ESP_LOGI(TAG, "KBD send_indicate: conn=%d gatts_if=%d handle=%d len=%d rc=%s",
             s_conn_id, (int)s_gatt_if, s_hh_char_handles[4], (int)sizeof(payload),
             esp_err_to_name(rc));
}

static void send_media_report(void) {
    if (!s_connected) {
        ESP_LOGW(TAG, "MEDIA send skipped: not connected");
        return;
    }
    uint8_t payload[sizeof(hid_media_report_t) + 1];
    payload[0] = REPORT_ID_MEDIA;
    memcpy(payload + 1, s_media_report, sizeof(hid_media_report_t));
    esp_err_t rc = esp_ble_gatts_send_indicate(s_gatt_if, s_conn_id,
                                               s_hh_char_handles[6],
                                               sizeof(payload), payload, false);
    ESP_LOGI(TAG, "MEDIA send_indicate: conn=%d gatts_if=%d handle=%d len=%d rc=%s",
             s_conn_id, (int)s_gatt_if, s_hh_char_handles[6], (int)sizeof(payload),
             esp_err_to_name(rc));
}

/* ======================== Service creation helpers ======================== */

static void create_hid_service(void) {
    esp_gatt_srvc_id_t srvc_id = {
        .id.uuid = uuid_hid_service,
        .id.inst_id = 0,
        .is_primary = true,
    };
    esp_ble_gatts_create_service(s_gatt_if, &srvc_id, 20);
}

static void create_battery_service(void) {
    esp_gatt_srvc_id_t srvc_id = {
        .id.uuid = uuid_battery_service,
        .id.inst_id = 0,
        .is_primary = true,
    };
    esp_ble_gatts_create_service(s_gatt_if, &srvc_id, 4);
}

static void create_devinfo_service(void) {
    esp_gatt_srvc_id_t srvc_id = {
        .id.uuid = uuid_devinfo_service,
        .id.inst_id = 0,
        .is_primary = true,
    };
    esp_ble_gatts_create_service(s_gatt_if, &srvc_id, 4);
}

/* ======================== GAP Callback ======================== */

static void gap_event_handler(esp_gap_ble_cb_event_t event,
                              esp_ble_gap_cb_param_t *param) {
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        /* Now create the HID service (needs gatt_if from REG_EVT) */
        create_hid_service();
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(TAG, "Connection params updated, status=%d", param->update_conn_params.status);
        break;
    case ESP_GAP_BLE_SEC_REQ_EVT:
        /* Remote device requests encryption — accept */
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT:
        ESP_LOGI(TAG, "Auth complete, success=%d", param->ble_security.auth_cmpl.success);
        break;
    default:
        break;
    }
}

/* ======================== GATTS Callback ======================== */

static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param) {

    switch (event) {

    case ESP_GATTS_REG_EVT: {
        if (param->reg.status == ESP_GATT_OK) {
            s_gatt_if = gatts_if;
        }
        break;
    }

    case ESP_GATTS_CREATE_EVT: {
        uint16_t svc_handle = param->create.service_handle;
        esp_bt_uuid_t *svc_uuid = &param->create.service_id.id.uuid;

        if (svc_uuid->uuid.uuid16 == 0x1812) {
            /* HID Service created — add all characteristics */
            s_hid_svc_handle = svc_handle;
            ESP_LOGI(TAG, "HID service created, handle=%d", svc_handle);

            /* HID Information (bcdHID=1.00, country=0, flags=01=NormallyConnectable)
             * Matches reference (hidInfo(0x00, 0x01)): {0x00,0x01,0x00,0x01} */
            static uint8_t hid_info[] = {0x00, 0x01, 0x00, 0x01};
            esp_attr_value_t hid_info_val = {
                .attr_max_len = 4,
                .attr_len     = 4,
                .attr_value   = hid_info,
            };
            esp_ble_gatts_add_char(svc_handle, &uuid_hid_information,
                                   ESP_GATT_PERM_READ,
                                   ESP_GATT_CHAR_PROP_BIT_READ,
                                   &hid_info_val, NULL);

            /* HID Report Map */
            esp_attr_value_t map_val = {
                .attr_max_len = sizeof(hid_report_map),
                .attr_len     = sizeof(hid_report_map),
                .attr_value   = (uint8_t *)hid_report_map,
            };
            esp_ble_gatts_add_char(svc_handle, &uuid_hid_report_map,
                                   ESP_GATT_PERM_READ,
                                   ESP_GATT_CHAR_PROP_BIT_READ,
                                   &map_val, NULL);

            /* HID Control Point */
            esp_ble_gatts_add_char(svc_handle, &uuid_hid_control_point,
                                   ESP_GATT_PERM_WRITE,
                                   ESP_GATT_CHAR_PROP_BIT_WRITE_NR,
                                   NULL, NULL);

            /* Protocol Mode */
            esp_attr_value_t proto_val = {
                .attr_max_len = 1,
                .attr_len     = 1,
                .attr_value   = (uint8_t[]){0x01},
            };
            esp_ble_gatts_add_char(svc_handle, &uuid_hid_protocol_mode,
                                   ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                   ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE_NR,
                                   &proto_val, NULL);

            /* Keyboard Input Report (notify) — handle stored at index 4 */
            esp_ble_gatts_add_char(svc_handle, &uuid_hid_report,
                                   ESP_GATT_PERM_READ,
                                   ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                                   NULL, NULL);

            /* Keyboard LED Output Report (write) */
            esp_ble_gatts_add_char(svc_handle, &uuid_hid_report,
                                   ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                   ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE_NR | ESP_GATT_CHAR_PROP_BIT_WRITE,
                                   NULL, NULL);

            /* Media Input Report (notify) — handle stored at index 6 */
            esp_ble_gatts_add_char(svc_handle, &uuid_hid_report,
                                   ESP_GATT_PERM_READ,
                                   ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                                   NULL, NULL);

            /* Create battery and device info services */
            create_battery_service();
            create_devinfo_service();
        } else if (svc_uuid->uuid.uuid16 == 0x180F) {
            /* Battery Service */
            s_batt_svc_handle = svc_handle;
            ESP_LOGI(TAG, "Battery service created");
            static uint8_t batt_val = 100;
            esp_attr_value_t batt_attr = {.attr_max_len = 1, .attr_len = 1, .attr_value = &batt_val};
            esp_ble_gatts_add_char(svc_handle, &uuid_battery_level,
                                   ESP_GATT_PERM_READ,
                                   ESP_GATT_CHAR_PROP_BIT_READ,
                                   &batt_attr, NULL);
        } else if (svc_uuid->uuid.uuid16 == 0x180A) {
            /* Device Info Service */
            s_devinfo_svc_handle = svc_handle;
            ESP_LOGI(TAG, "Device info service created");
            static uint8_t mfr_name[] = "Espressif";
            esp_attr_value_t mfr_attr = {
                .attr_max_len = sizeof(mfr_name),
                .attr_len     = sizeof(mfr_name) - 1,
                .attr_value   = mfr_name,
            };
            esp_ble_gatts_add_char(svc_handle, &uuid_manufacturer_name,
                                   ESP_GATT_PERM_READ,
                                   ESP_GATT_CHAR_PROP_BIT_READ,
                                   &mfr_attr, NULL);
        }
        break;
    }

    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;

    case ESP_GATTS_ADD_CHAR_EVT: {
        uint16_t handle = param->add_char.attr_handle;
        uint16_t uuid = param->add_char.char_uuid.uuid.uuid16;
        uint16_t svc = param->add_char.service_handle;

        if (svc == s_hid_svc_handle) {
            /* Store handles in order of creation */
            static int s_hid_char_idx = 0;
            s_hh_char_handles[s_hid_char_idx] = handle;
            ESP_LOGI(TAG, "HID char[%d] UUID=0x%04X handle=%d", s_hid_char_idx, uuid, handle);

            /* Add CCC + Report Reference to input Report characteristics */
            if (uuid == 0x2A4D && s_hid_char_idx == 4) {
                /* Keyboard input: CCC + Report Ref (ID=1, Type=Input) */
                static uint8_t ccc_zero[] = {0x00, 0x00};
                esp_attr_value_t ccc_val = {.attr_max_len = 2, .attr_len = 2, .attr_value = ccc_zero};
                esp_ble_gatts_add_char_descr(svc, &uuid_ccc,
                                             ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                             &ccc_val, NULL);
                static uint8_t kbd_ref[] = {REPORT_ID_KEYBOARD, 0x01};
                esp_attr_value_t ref_val = {.attr_max_len = 2, .attr_len = 2, .attr_value = kbd_ref};
                esp_ble_gatts_add_char_descr(svc, &uuid_report_ref,
                                             ESP_GATT_PERM_READ,
                                             &ref_val, NULL);
            } else if (uuid == 0x2A4D && s_hid_char_idx == 6) {
                /* Media input: CCC + Report Ref (ID=2, Type=Input) */
                static uint8_t ccc_zero[] = {0x00, 0x00};
                esp_attr_value_t ccc_val = {.attr_max_len = 2, .attr_len = 2, .attr_value = ccc_zero};
                esp_ble_gatts_add_char_descr(svc, &uuid_ccc,
                                             ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                             &ccc_val, NULL);
                static uint8_t media_ref[] = {REPORT_ID_MEDIA, 0x01};
                esp_attr_value_t ref_val = {.attr_max_len = 2, .attr_len = 2, .attr_value = media_ref};
                esp_ble_gatts_add_char_descr(svc, &uuid_report_ref,
                                             ESP_GATT_PERM_READ,
                                             &ref_val, NULL);
            }

            s_hid_char_idx++;
        } else if (svc == s_batt_svc_handle) {
            s_batt_char_handle = handle;
            esp_ble_gatts_start_service(s_batt_svc_handle);
        } else if (svc == s_devinfo_svc_handle) {
            s_devinfo_char_handle = handle;
            esp_ble_gatts_start_service(s_devinfo_svc_handle);
        }
        break;
    }

    case ESP_GATTS_ADD_CHAR_DESCR_EVT: {
        uint16_t handle = param->add_char_descr.attr_handle;
        uint16_t uuid = param->add_char_descr.descr_uuid.uuid.uuid16;
        ESP_LOGI(TAG, "Descriptor UUID=0x%04X handle=%d", uuid, handle);

        static int descr_idx = 0;
        if (descr_idx < 4) {
            s_hid_descr_handles[descr_idx] = handle;
            descr_idx++;
        }
        /* HID service complete after its 4 descriptors are added → make it discoverable */
        if (descr_idx == 4) {
            esp_err_t rc = esp_ble_gatts_start_service(s_hid_svc_handle);
            ESP_LOGI(TAG, "HID service start = %s", esp_err_to_name(rc));
        }
        break;
    }

    case ESP_GATTS_START_EVT:
        ESP_LOGI(TAG, "Service started, status=%d", param->start.status);
        break;

    case ESP_GATTS_CONNECT_EVT: {
        s_conn_id = param->connect.conn_id;
        memcpy(s_remote_bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        s_connected = true;
        ESP_LOGI(TAG, "Client connected, conn_id=%d gatts_if=%d media_handle=%d kbd_handle=%d",
                 s_conn_id, (int)s_gatt_if, s_hh_char_handles[6], s_hh_char_handles[4]);

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
        s_keyboard_ccc = 0;
        s_media_ccc = 0;
        memset(&s_kbd_report, 0, sizeof(s_kbd_report));
        memset(s_media_report, 0, sizeof(s_media_report));
        ESP_LOGI(TAG, "Client disconnected, restarting advertising");
        esp_ble_gap_start_advertising(&adv_params);
        break;
    }

    case ESP_GATTS_WRITE_EVT: {
        uint16_t handle = param->write.handle;
        ESP_LOGI(TAG, "WRITE handle=%d len=%d is_prep=%d offset=%d",
                 handle, param->write.len, param->write.is_prep, param->write.offset);
        /* CCCD writes are 2 bytes; value 0x0001 = notify, 0x0002 = indicate. */
        if (!param->write.is_prep && param->write.len == 2) {
            uint16_t value = param->write.value[0] | (param->write.value[1] << 8);
            /* Match CCC by stored handle OR char_handle+1 (robust fallback) */
            if (handle == s_hid_descr_handles[0] || handle == s_hh_char_handles[4] + 1) {
                s_keyboard_ccc = (value & 0x0003) ? value : 0;
                ESP_LOGI(TAG, "Keyboard CCC = 0x%04X (%s)", value,
                         (value & 0x0001) ? "notify" : (value & 0x0002) ? "indicate" : "disabled");
            } else if (handle == s_hid_descr_handles[2] || handle == s_hh_char_handles[6] + 1) {
                s_media_ccc = (value & 0x0003) ? value : 0;
                ESP_LOGI(TAG, "Media CCC = 0x%04X (%s)", value,
                         (value & 0x0001) ? "notify" : (value & 0x0002) ? "indicate" : "disabled");
            }
        }
        break;
    }

    case ESP_GATTS_READ_EVT: {
        uint16_t h = param->read.handle;
        ESP_LOGI(TAG, "READ handle=%d (offset=%d, need_rsp=%d)",
                 h, param->read.offset, param->read.need_rsp);
        if (h == s_batt_char_handle) {
            uint8_t batt = 100;
            esp_ble_gatts_set_attr_value(h, 1, &batt);
        }
        break;
    }

    default:
        break;
    }
}

#else /* !CONFIG_BT_ENABLED — stubs when BT is disabled */

#include "esp_log.h"
static const char *TAG = "ble_hid_kb";

bool ble_hid_kb_init(const ble_hid_kb_config_t *config) {
    ESP_LOGW(TAG, "BT not enabled in sdkconfig");
    return false;
}
bool ble_hid_kb_start(void) { return false; }
void ble_hid_kb_stop(void) {}
bool ble_hid_kb_is_connected(void) { return false; }
bool ble_hid_kb_wait_connection(uint32_t timeout_ms) { return false; }
bool ble_hid_kb_send_media(const media_key_t key) { return false; }
bool ble_hid_kb_press_media(const media_key_t key) { return false; }
bool ble_hid_kb_release_media(void) { return false; }
bool ble_hid_kb_send_key(uint8_t hid_keycode) { return false; }
bool ble_hid_kb_press_key(uint8_t modifiers, uint8_t keycode) { return false; }
void ble_hid_kb_release_all(void) {}
bool ble_hid_kb_send_text(const char *text) { return false; }
void ble_hid_kb_set_battery(uint8_t level) {}

#endif /* CONFIG_BT_ENABLED */
