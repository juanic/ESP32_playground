/*==================[inclusions]=============================================*/
#include "usb_hid_kb.h"

/* The whole USB HID implementation is only compiled when this transport is
 * actually selected (HID_TRANSPORT_USB / BOTH). This mirrors the CMake condition
 * that adds this file to drivers_hal's SRCS, so the object can never end up empty
 * while the app expects these symbols. For the selected transport, CMake also adds
 * 'esp_tinyusb' (+ its public 'tinyusb' stack dependency) to PRIV_REQUIRES, so
 * tinyusb.h and class/hid/hid_device.h are on the include path.
 * (esp_tinyusb v2.x: there is no tinyusb_init(); the stack is installed with
 * tinyusb_driver_install() and the HID class is enabled via CONFIG_TINYUSB_HID_COUNT.) */
#if defined(CONFIG_HID_TRANSPORT_USB) || defined(CONFIG_HID_TRANSPORT_BOTH)

#include <stdint.h>
#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "tinyusb_default_config.h"
#include "class/hid/hid_device.h"

#include "hid_common.h"

/*==================[macros]=================================================*/
#define USB_HID_KB_TAG        "USB_HID_KB"
#define USB_HID_CC_REPORT_ID  1   /**< HID report id of the Consumer Control collection */

/*==================[descriptors]============================================*/
/* Total length of the configuration descriptor (config header + 1 HID interface). */
#define TUSB_DESC_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + CFG_TUD_HID * TUD_HID_DESC_LEN)

/* Consumer Control (media keys) report descriptor. */
static const uint8_t s_hid_report_descriptor[] = {
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(USB_HID_CC_REPORT_ID))
};

/* String descriptors (language, manufacturer, product, serial, interface). */
static const char *s_hid_string_descriptor[5] = {
    (char[]){0x09, 0x04},        // 0: supported language (English, 0x0409)
    "ESP32-S3",                  // 1: Manufacturer
    "ESP32-S3 HID Media Keys",   // 2: Product
    "123456",                    // 3: Serial
    "HID Consumer Control",      // 4: HID interface
};

/* Configuration descriptor: 1 configuration, 1 HID interface (Consumer Control). */
static const uint8_t s_hid_configuration_descriptor[] = {
    /* Configuration number, interface count, string index, total length, attribute, power in mA */
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, TUSB_DESC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    /* Interface number, string index, boot protocol, report descriptor len, EP In address, size & polling interval */
    TUD_HID_DESCRIPTOR(0, 4, false, sizeof(s_hid_report_descriptor), 0x81, 16, 10),
};

/*==================[tinyusb hid callbacks]==================================*/
/* Invoked when the host requests the HID report descriptor. */
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return s_hid_report_descriptor;
}

/* Invoked when the host issues a GET_REPORT control request. */
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

/* Invoked when the host issues a SET_REPORT control request or sends data on the OUT endpoint. */
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}

/*==================[internal data]==========================================*/
static bool s_installed = false;

/*==================[public api]=============================================*/
esp_err_t usb_hid_driver_init(void)
{
    if (s_installed) {
        return ESP_OK;
    }

    tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG();

    tusb_cfg.descriptor.device = NULL;   /* use the default device descriptor */
    tusb_cfg.descriptor.full_speed_config = s_hid_configuration_descriptor;
    tusb_cfg.descriptor.string = s_hid_string_descriptor;
    tusb_cfg.descriptor.string_count =
        sizeof(s_hid_string_descriptor) / sizeof(s_hid_string_descriptor[0]);
#if (TUD_OPT_HIGH_SPEED)
    tusb_cfg.descriptor.high_speed_config = s_hid_configuration_descriptor;
#endif

    esp_err_t ret = tinyusb_driver_install(&tusb_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(USB_HID_KB_TAG, "tinyusb_driver_install failed: %d", ret);
        return ret;
    }

    s_installed = true;
    ESP_LOGI(USB_HID_KB_TAG, "USB HID (Consumer Control) driver installed");
    return ESP_OK;
}

esp_err_t usb_hid_driver_send_key(media_key_t key)
{
    uint16_t usage = hid_media_key_to_usage(key);
    if (usage == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_installed) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!tud_mounted()) {
        return ESP_ERR_INVALID_STATE;
    }

    /* Press: send the 16-bit Consumer Control usage. */
    uint8_t report[2] = {
        (uint8_t)(usage & 0xFF),
        (uint8_t)((usage >> 8) & 0xFF),
    };
    tud_hid_report(USB_HID_CC_REPORT_ID, report, sizeof(report));

    /* Hold the key briefly, then release (send zero usage). */
    vTaskDelay(pdMS_TO_TICKS(20));
    uint8_t release[2] = {0, 0};
    tud_hid_report(USB_HID_CC_REPORT_ID, release, sizeof(release));

    return ESP_OK;
}

bool usb_hid_driver_is_connected(void)
{
    return tud_mounted();
}

esp_err_t usb_hid_driver_deinit(void)
{
    if (!s_installed) {
        return ESP_OK;
    }
    esp_err_t ret = tinyusb_driver_uninstall();
    if (ret == ESP_OK) {
        s_installed = false;
    } else {
        ESP_LOGE(USB_HID_KB_TAG, "tinyusb_driver_uninstall failed: %d", ret);
    }
    return ret;
}

#endif /* CONFIG_HID_TRANSPORT_USB || CONFIG_HID_TRANSPORT_BOTH */
