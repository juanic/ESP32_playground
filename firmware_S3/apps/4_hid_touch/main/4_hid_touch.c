/*! @mainpage 4_hid_touch - Touch-controlled HID Consumer Control
 *
 * @section genDesc General Description
 *
 * Turns the ESP32-S3 capacitive touch pads into a USB/BLE HID media controller,
 * using the drivers_hal @c touch_hal driver to read the pads and the
 * @c usb_hid_kb / @c ble_hid_kb transport driver to send the key.
 *
 * Unlike the cycling 2_/3_ examples, here each touch channel is mapped to a
 * media key (play/pause, next, previous, volume up/down, stop). On boot the
 * baseline of every channel is averaged; afterwards a deviation of
 * @c TOUCH_DELTA_TRIGGER (200) counts from that baseline triggers a single key
 * press. On the ESP32-S3 touch sensor the raw value RISES when the pad is
 * touched (the opposite of the classic ESP32), which is exactly what the
 * calibration + delta test accounts for. Detection is edge-triggered, so
 * holding a pad sends one key only.
 *
 * Touch channel -> GPIO mapping on ESP32-S3 (touch_t enum in touch_hal.h):
 *   TOUCH_PAD_0=GPIO1, TOUCH_PAD_1=GPIO2, TOUCH_PAD_2=GPIO3, TOUCH_PAD_3=GPIO4,
 *   TOUCH_PAD_4=GPIO5, TOUCH_PAD_5=GPIO6, TOUCH_PAD_6=GPIO7, TOUCH_PAD_7=GPIO8,
 *   TOUCH_PAD_8=GPIO9, TOUCH_PAD_9=GPIO10 ... TOUCH_PAD_13=GPIO14.
 * The pads TOUCH_PAD_0/2/3 (GPIO1/3/4) are shared with boot-strapping pins, so
 * the default table uses non-strapping channels (TOUCH_PAD_1,4,5,6,7,8 ->
 * GPIO2,5,6,7,8,9). Adjust @ref s_map to match the pads you actually wire.
 *
 * @section changelog Changelog
 *
 * |   Date       | Description                          |
 * |:-------------|:-------------------------------------|
 * | 29/08/2026   | Document creation                   |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "touch_hal.h"
#include "hid_common.h"

#if defined(CONFIG_HID_TRANSPORT_USB) || defined(CONFIG_HID_TRANSPORT_BOTH)
#include "usb_hid_kb.h"
#define HID_USE_USB
#endif
#if defined(CONFIG_HID_TRANSPORT_BLE) || defined(CONFIG_HID_TRANSPORT_BOTH)
#include "ble_hid_kb.h"
#define HID_USE_BLE
#endif

/*==================[macros and definitions]=================================*/
#define TOUCH_CAL_SAMPLES     32      /* samples averaged for the baseline */
#define TOUCH_DELTA_TRIGGER   200     /* counts above baseline => touched   */
#define TOUCH_READ_PERIOD_MS  20      /* polling period while connected      */
#define TOUCH_HOST_POLL_MS    200     /* polling period while no host        */

/* Touch channel -> media key mapping (ESP32-S3: TOUCH_PAD_n == GPIO_(n+1)). */
typedef struct {
    touch_t      pad;
    media_key_t  key;
    const char  *name;
} touch_map_t;

static const touch_map_t s_map[] = {
    { TOUCH_PAD_4, MEDIA_KEY_NEXT,     "NEXT" },
    { TOUCH_PAD_5, MEDIA_KEY_VOL_UP,   "VOL+" },
    { TOUCH_PAD_6, MEDIA_KEY_PAUSE,    "PAUSE" },
    { TOUCH_PAD_7, MEDIA_KEY_PLAY,     "PLAY" },
    { TOUCH_PAD_8, MEDIA_KEY_VOL_DOWN, "VOL-" },
    { TOUCH_PAD_9, MEDIA_KEY_PREV,     "BACK" },
};
#define N_MAP (sizeof(s_map) / sizeof(s_map[0]))

static uint32_t s_baseline[N_MAP] = {0};
static bool     s_touched_prev[N_MAP] = {false};

static const char *TAG = "HID_TOUCH";

/*==================[internal functions definition]==========================*/
static esp_err_t hid_init(void)
{
#if defined(HID_USE_USB)
    ESP_RETURN_ON_ERROR(usb_hid_driver_init(), TAG, "usb_hid_driver_init failed");
#endif
#if defined(HID_USE_BLE)
    ESP_RETURN_ON_ERROR(ble_hid_driver_init("ESP32-S3 Touch"), TAG, "ble_hid_driver_init failed");
#endif
    return ESP_OK;
}

static void hid_send(media_key_t key)
{
#if defined(HID_USE_USB)
    usb_hid_driver_send_key(key);
#endif
#if defined(HID_USE_BLE)
    ble_hid_driver_send_key(key);
#endif
}

static bool hid_connected(void)
{
#if defined(HID_USE_USB)
    return usb_hid_driver_is_connected();
#endif
#if defined(HID_USE_BLE)
    return ble_hid_driver_is_connected();
#endif
    return false;
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    ESP_ERROR_CHECK(hid_init());
    ESP_ERROR_CHECK(TouchHalInit() ? ESP_OK : ESP_FAIL);

    /* Configure every channel. The hardware threshold is irrelevant here because
     * we do our own baseline-delta detection, so a placeholder (0) is fine. */
    for (int i = 0; i < N_MAP; i++) {
        if (!TouchHalChannelConfig(s_map[i].pad, 0)) {
            ESP_LOGE(TAG, "channel config failed for pad %d", s_map[i].pad);
        }
    }
    ESP_ERROR_CHECK(TouchHalStart() ? ESP_OK : ESP_FAIL);

    /* Calibration: average the baseline of each channel (value rises when touched). */
    for (int i = 0; i < N_MAP; i++) {
        uint32_t sum = 0, v = 0;
        for (int s = 0; s < TOUCH_CAL_SAMPLES; s++) {
            if (TouchHalRead(s_map[i].pad, &v)) {
                sum += v;
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        s_baseline[i] = sum / TOUCH_CAL_SAMPLES;
        ESP_LOGI(TAG, "calibrated %s (pad %d) baseline=%lu",
                 s_map[i].name, s_map[i].pad, s_baseline[i]);
    }
    ESP_LOGI(TAG, "touch controller ready - touch a pad to send a media key");

    uint32_t host_poll = 0;
    while (true) {
        bool connected = hid_connected();
        if (!connected) {
            if ((host_poll++ % 10) == 0) {
                ESP_LOGI(TAG, "waiting for a HID host (connect USB / pair BLE)...");
            }
            vTaskDelay(pdMS_TO_TICKS(TOUCH_HOST_POLL_MS));
            continue;
        }

        for (int i = 0; i < N_MAP; i++) {
            uint32_t v = 0;
            if (!TouchHalRead(s_map[i].pad, &v)) {
                continue;
            }
            /* On ESP32-S3 the value INCREASES when touched. */
            bool touched = (v >= s_baseline[i] + TOUCH_DELTA_TRIGGER);
            if (touched && !s_touched_prev[i]) {
                hid_send(s_map[i].key);
                ESP_LOGI(TAG, "%s touched (val=%lu base=%lu)",
                         s_map[i].name, v, s_baseline[i]);
            }
            s_touched_prev[i] = touched;
        }
        vTaskDelay(pdMS_TO_TICKS(TOUCH_READ_PERIOD_MS));
    }
}
