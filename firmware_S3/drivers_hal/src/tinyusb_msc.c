/*! @file tinyusb_msc.c
 *  @brief TinyUSB MSC (Mass Storage Class) driver wrapper.
 *  @defgroup tinyusb_msc TinyUSB MSC impl.
 *  @{
 *
 * Wrapper alrededor de la librería TinyUSB para operar el periférico
 * USB en modo Device MSC, usando la tarjeta SD como backing store.
 * Proporciona init, connect y disconnect para el modo Server <-> MSC
 * transition controlado por la capa middleware.

#include "tinyusb_msc.h"
#include "tusb.h"
#include "esp_log.h"
#include "esp_err.h"

/* Flag to track MSC state */
static bool msc_initialized = false;
static bool msc_connected = false;

/* Callback for MSC mount state (called by TinyUSB) */
static void tud_msc_mount_cb(void) {
    ESP_LOGI("tinyusb_msc", "MSC drive mounted");
}

static void tud_umount_cb(void) {
    ESP_LOGI("tinyusb_msc", "MSC drive unmounted");
}

bool tinyusb_msc_init(void) {
    if (msc_initialized) {
        return true;
    }

    /* Configure TinyUSB MSC device */
    tusb_config_t config = {
        .device_address = 0,
        .fs_configuration = 1,
        .fs_intr_tx_size = 64,
        .hs_intr_tx_size = 64,
    };

    /* Initialize TinyUSB stack */
    if (tusb_init() != ESP_OK) {
        ESP_LOGE("tinyusb_msc", "Failed to initialize TinyUSB");
        return false;
    }

    /* Register MSC callbacks */
    tusb_msc_device_set_mount_callback(tud_msc_mount_cb);
    tusb_msc_device_set_umount_callback(tud_umount_cb);

    msc_initialized = true;
    ESP_LOGI("tinyusb_msc", "TinyUSB MSC initialized");
    return true;
}

void tinyusb_msc_connect(void) {
    if (!msc_initialized) {
        ESP_LOGE("tinyusb_msc", "MSC not initialized");
        return;
    }
    if (!msc_connected) {
        MSC_TUD_CONNECT();
        msc_connected = true;
        ESP_LOGI("tinyusb_msc", "MSC connected - host can enumerate drive");
    }
}

void tinyusb_msc_disconnect(void) {
    if (!msc_initialized) {
        return;
    }
    if (msc_connected) {
        MSC_TUD_DISCONNECT();
        msc_connected = false;
        ESP_LOGI("tinyusb_msc", "MSC disconnected - host will eject drive");
    }
}

/*==================[end of file]============================================*/