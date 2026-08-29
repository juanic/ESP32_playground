/*==================[inclusions]=============================================*/
#include "ble_hid_kb.h"

/* The whole BLE HID implementation is only compiled when this transport is
 * actually selected (HID_TRANSPORT_BLE / BOTH). This mirrors the CMake condition
 * that adds this file to drivers_hal's SRCS, so the object can never end up empty
 * while the app expects these symbols. For the selected transport, CMake also adds
 * 'esp_hid' + 'bt' to PRIV_REQUIRES, so esp_bt.h and friends are on the include
 * path. (In ESP-IDF v6.x BLE is implied by the Bluedroid/NimBLE host choice; there
 * is no separate CONFIG_BT_BLE_ENABLED symbol.) */
#if defined(CONFIG_HID_TRANSPORT_BLE) || defined(CONFIG_HID_TRANSPORT_BOTH)

#include <inttypes.h>
#include <string.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_hid_common.h"
#include "esp_hidd.h"
#include "esp_hidd_gatts.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros]=================================================*/
#define BLE_HID_KB_TAG            "BLE_HID_KB"
#define BLE_HID_CC_REPORT_ID      1   /**< HID report id of the Consumer Control collection */
#define BLE_HID_CC_MAP_INDEX      0   /**< index of the Consumer Control map in s_report_maps */
#define BLE_HID_KB_REPORT_ID      2   /**< HID report id of the boot keyboard collection */
#define BLE_HID_KB_MAP_INDEX      1   /**< index of the keyboard map in s_report_maps */
#define BLE_HID_CC_REPORT_LEN     2   /**< Consumer Control input report length (16-bit usage) */
#define BLE_HID_KB_REPORT_LEN     8   /**< boot keyboard input report length */
#define BLE_HID_MAX_BOND_DEV      16  /**< max bonded peers we can enumerate/remove */
#define BLE_HID_KEY_RELEASE_MS    15  /**< press duration before auto-release */

/*==================[typedef]================================================*/

/*==================[internal function declaration]==========================*/
static esp_err_t ble_hid_kb_gap_init(void);
static esp_err_t ble_hid_kb_adv_init(uint16_t appearance, const char *device_name);
static esp_err_t ble_hid_kb_adv_start(void);
static void ble_hid_kb_gap_event_handler(esp_gap_ble_cb_event_t event,
                                         esp_ble_gap_cb_param_t *param);
static void ble_hid_kb_hidd_event_callback(void *handler_args,
                                           esp_event_base_t base,
                                           int32_t id,
                                           void *event_data);

/*==================[internal data definition]==============================*/

/* Consumer Control report map: a single 16-bit input field whose value is the
 * USB HID Consumer-page usage code of the pressed key. Keeping ONE report with
 * a single fixed-size (16-bit) field avoids the mixed-size report-map bug. */
static const uint8_t s_consumer_map[] = {
    0x05, 0x0C,        // Usage Page (Consumer Devices)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x03,  //   Logical Maximum (1023)
    0x19, 0x00,        //   Usage Minimum (0)
    0x2A, 0xFF, 0x03,  //   Usage Maximum (1023)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x00,        //   Input (Data,Array,Abs)
    0xC0               // End Collection
};

/* Optional boot keyboard report map: modifier (1) + reserved (1) + 6 keycodes. */
static const uint8_t s_keyboard_map[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        //   Report ID (2)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x07,        //     Usage Page (Keyboard/Keypad)
    0x19, 0xE0,        //     Usage Minimum (224)
    0x29, 0xE7,        //     Usage Maximum (231)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x08,        //     Report Count (8)
    0x81, 0x02,        //     Input (Data,Var,Abs)        ; modifier byte
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x08,        //     Report Size (8)
    0x81, 0x03,        //     Input (Const,Var,Abs)       ; reserved byte
    0x95, 0x06,        //     Report Count (6)
    0x75, 0x08,        //     Report Size (8)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x65,        //     Logical Maximum (101)
    0x05, 0x07,        //     Usage Page (Keyboard/Keypad)
    0x19, 0x00,        //     Usage Minimum (0)
    0x29, 0x65,        //     Usage Maximum (101)
    0x81, 0x00,        //     Input (Data,Array,Abs)      ; 6 keycodes
    0xC0,              //   End Collection (Physical)
    0xC0               // End Collection
};

static esp_hid_raw_report_map_t s_report_maps[] = {
    { .data = s_consumer_map, .len = sizeof(s_consumer_map) },
    { .data = s_keyboard_map, .len = sizeof(s_keyboard_map) },
};

static esp_hidd_dev_t *s_hid_dev   = NULL;
static bool            s_connected = false;
static bool            s_initialised = false;

/*==================[internal functions definition]=========================*/

static esp_err_t ble_hid_kb_gap_init(void)
{
    esp_err_t ret;
    esp_bt_controller_config_t cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) {
        ESP_LOGW(BLE_HID_KB_TAG, "bt controller mem release (classic): %d", ret);
    }

    ret = esp_bt_controller_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "bt controller init failed: %d", ret);
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "bt controller enable failed: %d", ret);
        return ret;
    }

    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "bluedroid init failed: %d", ret);
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "bluedroid enable failed: %d", ret);
        return ret;
    }

    return ESP_OK;
}

static esp_err_t ble_hid_kb_adv_init(uint16_t appearance, const char *device_name)
{
    static const uint8_t hid_svc_uuid128[16] = {
        0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
        0x00, 0x10, 0x00, 0x00, 0x12, 0x18, 0x00, 0x00
    };

    esp_ble_adv_data_t adv_data = {
        .set_scan_rsp      = false,
        .include_name      = true,
        .include_txpower   = true,
        .min_interval      = 0x0006,
        .max_interval      = 0x0010,
        .appearance        = appearance,
        .manufacturer_len  = 0,
        .p_manufacturer_data = NULL,
        .service_data_len  = 0,
        .p_service_data    = NULL,
        .service_uuid_len  = sizeof(hid_svc_uuid128),
        .p_service_uuid    = (uint8_t *)hid_svc_uuid128,
        .flag              = 0x6,
    };

    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
    esp_ble_io_cap_t   iocap    = ESP_IO_CAP_IO;
    uint8_t init_key  = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key   = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t key_size  = 16;
    uint32_t passkey  = 1234;
    esp_err_t ret;

    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, 1)) != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "set AUTHEN_REQ_MODE failed: %d", ret);
        return ret;
    }
    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, 1)) != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "set IOCAP_MODE failed: %d", ret);
        return ret;
    }
    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, 1)) != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "set SET_INIT_KEY failed: %d", ret);
        return ret;
    }
    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, 1)) != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "set SET_RSP_KEY failed: %d", ret);
        return ret;
    }
    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, 1)) != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "set MAX_KEY_SIZE failed: %d", ret);
        return ret;
    }
    if ((ret = esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t))) != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "set STATIC_PASSKEY failed: %d", ret);
        return ret;
    }
    if ((ret = esp_ble_gap_set_device_name(device_name)) != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "set device name failed: %d", ret);
        return ret;
    }
    if ((ret = esp_ble_gap_config_adv_data(&adv_data)) != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "config adv data failed: %d", ret);
        return ret;
    }
    return ESP_OK;
}

static esp_err_t ble_hid_kb_adv_start(void)
{
    static esp_ble_adv_params_t adv_params = {
        .adv_int_min       = 0x20,
        .adv_int_max       = 0x30,
        .adv_type          = ADV_TYPE_IND,
        .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
        .channel_map       = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    return esp_ble_gap_start_advertising(&adv_params);
}

static void ble_hid_kb_gap_event_handler(esp_gap_ble_cb_event_t event,
                                         esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        ESP_LOGI(BLE_HID_KB_TAG, "[GAP] advertising data set");
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(BLE_HID_KB_TAG, "[GAP] advertising started");
        } else {
            ESP_LOGE(BLE_HID_KB_TAG, "[GAP] advertising start failed: 0x%x",
                     param->adv_start_cmpl.status);
        }
        break;

    case ESP_GAP_BLE_AUTH_CMPL_EVT:
        if (param->ble_security.auth_cmpl.success) {
            ESP_LOGI(BLE_HID_KB_TAG, "[GAP] AUTH success -> device BONDED with %02X:%02X:%02X:%02X:%02X:%02X",
                     param->ble_security.auth_cmpl.bd_addr[0],
                     param->ble_security.auth_cmpl.bd_addr[1],
                     param->ble_security.auth_cmpl.bd_addr[2],
                     param->ble_security.auth_cmpl.bd_addr[3],
                     param->ble_security.auth_cmpl.bd_addr[4],
                     param->ble_security.auth_cmpl.bd_addr[5]);
        } else {
            ESP_LOGE(BLE_HID_KB_TAG, "[GAP] AUTH FAILED (reason 0x%x) -> bonding not established",
                     param->ble_security.auth_cmpl.fail_reason);
        }
        break;

    case ESP_GAP_BLE_KEY_EVT:
        ESP_LOGI(BLE_HID_KB_TAG, "[GAP] bonding key type: %d",
                 (int)param->ble_security.ble_key.key_type);
        break;

    case ESP_GAP_BLE_SEC_REQ_EVT:
        ESP_LOGI(BLE_HID_KB_TAG, "[GAP] security request received -> accepting");
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        break;

    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
        ESP_LOGI(BLE_HID_KB_TAG, "[GAP] passkey to show on peer: %" PRIu32,
                 param->ble_security.key_notif.passkey);
        break;

    case ESP_GAP_BLE_NC_REQ_EVT:
        ESP_LOGI(BLE_HID_KB_TAG, "[GAP] numeric comparison passkey: %" PRIu32 " -> auto-confirm",
                 param->ble_security.key_notif.passkey);
        esp_ble_confirm_reply(param->ble_security.key_notif.bd_addr, true);
        break;

    case ESP_GAP_BLE_PASSKEY_REQ_EVT:
        ESP_LOGI(BLE_HID_KB_TAG, "[GAP] passkey request -> replying %" PRIu32, (uint32_t)1234);
        esp_ble_passkey_reply(param->ble_security.ble_req.bd_addr, true, 1234);
        break;

    case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
        ESP_LOGI(BLE_HID_KB_TAG, "[GAP] remove bond complete (status 0x%x)",
                 param->remove_bond_dev_cmpl.status);
        break;

    case ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT:
        ESP_LOGI(BLE_HID_KB_TAG, "[GAP] clear all bonds complete (status 0x%x)",
                 param->clear_bond_dev_cmpl.status);
        break;

    default:
        break;
    }
}

static void ble_hid_kb_hidd_event_callback(void *handler_args,
                                           esp_event_base_t base,
                                           int32_t id,
                                           void *event_data)
{
    esp_hidd_event_t event = (esp_hidd_event_t)id;
    esp_hidd_event_data_t *param = (esp_hidd_event_data_t *)event_data;

    switch (event) {
    case ESP_HIDD_START_EVENT:
        ESP_LOGI(BLE_HID_KB_TAG, "[HID] service started, starting advertising");
        ble_hid_kb_adv_start();
        break;

    case ESP_HIDD_CONNECT_EVENT:
        s_connected = true;
        ESP_LOGI(BLE_HID_KB_TAG, "[HID] >>> CONNECTED to host");
        break;

    case ESP_HIDD_DISCONNECT_EVENT:
        s_connected = false;
        ESP_LOGI(BLE_HID_KB_TAG, "[HID] <<< DISCONNECTED (reason: %s), restarting advertising",
                 esp_hid_disconnect_reason_str(esp_hidd_dev_transport_get(param->disconnect.dev),
                                               param->disconnect.reason));
        ble_hid_kb_adv_start();
        break;

    case ESP_HIDD_STOP_EVENT:
        ESP_LOGI(BLE_HID_KB_TAG, "[HID] service stopped");
        break;

    default:
        break;
    }
}

/*==================[external functions definition]=========================*/

esp_err_t ble_hid_driver_init(const char *device_name)
{
    esp_err_t ret;

    if (s_initialised) {
        return ESP_ERR_INVALID_STATE;
    }
    if (device_name == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ret = ble_hid_kb_gap_init();
    if (ret != ESP_OK) {
        return ret;
    }

    if ((ret = esp_ble_gap_register_callback(ble_hid_kb_gap_event_handler)) != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "gap register callback failed: %d", ret);
        return ret;
    }

    if ((ret = esp_ble_gatts_register_callback(esp_hidd_gatts_event_handler)) != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "gatts register callback failed: %d", ret);
        return ret;
    }

    if ((ret = ble_hid_kb_adv_init(ESP_HID_APPEARANCE_GENERIC, device_name)) != ESP_OK) {
        return ret;
    }

    esp_hid_device_config_t config = {
        .vendor_id         = 0xE501,
        .product_id        = 0x1234,
        .version           = 0x0100,
        .device_name       = device_name,
        .manufacturer_name = "Espressif",
        .serial_number     = "1",
        .report_maps       = s_report_maps,
        .report_maps_len   = sizeof(s_report_maps) / sizeof(s_report_maps[0]),
    };

    if ((ret = esp_hidd_dev_init(&config, ESP_HID_TRANSPORT_BLE,
                                 ble_hid_kb_hidd_event_callback, &s_hid_dev)) != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "hidd dev init failed: %d", ret);
        return ret;
    }

    s_initialised = true;
    ESP_LOGI(BLE_HID_KB_TAG, "driver initialised, advertising as '%s'", device_name);
    return ESP_OK;
}

esp_err_t ble_hid_driver_send_key(media_key_t key)
{
    if (!s_hid_dev || !s_connected) {
        return ESP_ERR_INVALID_STATE;
    }

    uint16_t usage = hid_media_key_to_usage(key);
    if (usage == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t report[BLE_HID_CC_REPORT_LEN] = {
        (uint8_t)(usage & 0xFF), (uint8_t)((usage >> 8) & 0xFF)
    };
    esp_err_t ret = esp_hidd_dev_input_set(s_hid_dev, BLE_HID_CC_MAP_INDEX,
                                           BLE_HID_CC_REPORT_ID, report,
                                           BLE_HID_CC_REPORT_LEN);
    if (ret != ESP_OK) {
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(BLE_HID_KEY_RELEASE_MS));

    uint8_t release[BLE_HID_CC_REPORT_LEN] = { 0, 0 };
    return esp_hidd_dev_input_set(s_hid_dev, BLE_HID_CC_MAP_INDEX,
                                  BLE_HID_CC_REPORT_ID, release,
                                  BLE_HID_CC_REPORT_LEN);
}

esp_err_t ble_hid_driver_send_keyboard(uint8_t modifier, const uint8_t keycodes[6])
{
    if (!s_hid_dev || !s_connected) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t report[BLE_HID_KB_REPORT_LEN] = { 0 };
    report[0] = modifier;
    if (keycodes != NULL) {
        memcpy(&report[2], keycodes, 6);
    }

    esp_err_t ret = esp_hidd_dev_input_set(s_hid_dev, BLE_HID_KB_MAP_INDEX,
                                           BLE_HID_KB_REPORT_ID, report,
                                           BLE_HID_KB_REPORT_LEN);
    if (ret != ESP_OK) {
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(BLE_HID_KEY_RELEASE_MS));

    uint8_t release[BLE_HID_KB_REPORT_LEN] = { 0 };
    return esp_hidd_dev_input_set(s_hid_dev, BLE_HID_KB_MAP_INDEX,
                                  BLE_HID_KB_REPORT_ID, release,
                                  BLE_HID_KB_REPORT_LEN);
}

bool ble_hid_driver_is_connected(void)
{
    return (s_hid_dev != NULL) && s_connected;
}

esp_err_t ble_hid_driver_remove_bonds(void)
{
    int num = BLE_HID_MAX_BOND_DEV;
    esp_ble_bond_dev_t list[BLE_HID_MAX_BOND_DEV];

    esp_err_t ret = esp_ble_get_bond_device_list(&num, list);
    if (ret != ESP_OK) {
        ESP_LOGE(BLE_HID_KB_TAG, "get bond device list failed: %d", ret);
        return ret;
    }

    ESP_LOGI(BLE_HID_KB_TAG, "removing %d bonded device(s)", num);
    for (int i = 0; i < num; i++) {
        esp_ble_remove_bond_device(list[i].bd_addr);
    }
    return ESP_OK;
}

esp_err_t ble_hid_driver_deinit(void)
{
    esp_err_t ret = ESP_OK;

    if (s_hid_dev != NULL) {
        ret = esp_hidd_dev_deinit(s_hid_dev);
        s_hid_dev = NULL;
    }
    s_connected = false;
    s_initialised = false;
    return ret;
}

#endif /* CONFIG_HID_TRANSPORT_BLE || CONFIG_HID_TRANSPORT_BOTH */

/*==================[end of file]============================================*/
