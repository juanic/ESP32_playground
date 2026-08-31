#ifndef BLE_HID_KB_H
#define BLE_HID_KB_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== Media Key Codes (2-byte report) ======================== */
typedef uint8_t media_key_t[2];

#define MEDIA_KEY_NONE              ((const media_key_t){0x00, 0x00})
#define MEDIA_KEY_NEXT_TRACK        ((const media_key_t){0x01, 0x00})
#define MEDIA_KEY_PREVIOUS_TRACK    ((const media_key_t){0x02, 0x00})
#define MEDIA_KEY_STOP              ((const media_key_t){0x04, 0x00})
#define MEDIA_KEY_PLAY_PAUSE        ((const media_key_t){0x08, 0x00})
#define MEDIA_KEY_MUTE              ((const media_key_t){0x10, 0x00})
#define MEDIA_KEY_VOLUME_UP         ((const media_key_t){0x20, 0x00})
#define MEDIA_KEY_VOLUME_DOWN       ((const media_key_t){0x40, 0x00})
#define MEDIA_KEY_WWW_HOME          ((const media_key_t){0x80, 0x00})
#define MEDIA_KEY_MY_COMPUTER       ((const media_key_t){0x00, 0x01})
#define MEDIA_KEY_CALCULATOR        ((const media_key_t){0x00, 0x02})
#define MEDIA_KEY_WWW_BOOKMARKS     ((const media_key_t){0x00, 0x04})
#define MEDIA_KEY_WWW_SEARCH        ((const media_key_t){0x00, 0x08})
#define MEDIA_KEY_WWW_STOP          ((const media_key_t){0x00, 0x10})
#define MEDIA_KEY_WWW_BACK          ((const media_key_t){0x00, 0x20})
#define MEDIA_KEY_MEDIA_SEL         ((const media_key_t){0x00, 0x40})
#define MEDIA_KEY_EMAIL             ((const media_key_t){0x00, 0x80})

/* ======================== Keyboard Modifier Bits ======================== */
#define KEY_MOD_LCTRL   0x01
#define KEY_MOD_LSHIFT  0x02
#define KEY_MOD_LALT    0x04
#define KEY_MOD_LGUI    0x08
#define KEY_MOD_RCTRL   0x10
#define KEY_MOD_RSHIFT  0x20
#define KEY_MOD_RALT    0x40
#define KEY_MOD_RGUI    0x80

/* ======================== HID Report Structures ======================== */
typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
} __attribute__((packed)) hid_keyboard_report_t;

typedef uint8_t hid_media_report_t[2];

/* ======================== Configuration ======================== */
typedef struct {
    const char *device_name;
    const char *device_manufacturer;
    uint8_t battery_level;
} ble_hid_kb_config_t;

#define BLE_HID_KB_DEFAULT_CONFIG() {          \
    .device_name = "ESP32 HID KB",             \
    .device_manufacturer = "Espressif",        \
    .battery_level = 100,                      \
}

/* ======================== API ======================== */

/**
 * @brief Inicializa el stack BLE y registra el servicio HID
 *        (teclado + media keys).
 *
 * @param config  Nombre del dispositivo, batería, etc.
 * @return true si todo ok.
 */
bool ble_hid_kb_init(const ble_hid_kb_config_t *config);

/**
 * @brief Inicia el advertising BLE (discoverable + connectable).
 */
bool ble_hid_kb_start(void);

/**
 * @brief Detiene advertising y cierra conexiones activas.
 */
void ble_hid_kb_stop(void);

/**
 * @brief Indica si hay un host BLE conectado.
 */
bool ble_hid_kb_is_connected(void);

/**
 * @brief Bloquea hasta que se establezca conexión BLE.
 *        Timeout en ms (0 = infinito).
 * @return true si conectó, false si timeout.
 */
bool ble_hid_kb_wait_connection(uint32_t timeout_ms);

/* ======================== Envío de reportes ======================== */

/**
 * @brief Envía un reporte de media key (press + release inmediato).
 */
bool ble_hid_kb_send_media(const media_key_t key);

/**
 * @brief Presiona una media key (mantiene presionada).
 */
bool ble_hid_kb_press_media(const media_key_t key);

/**
 * @brief Suelta la media key actualmente presionada.
 */
bool ble_hid_kb_release_media(void);

/**
 * @brief Envía una tecla de código HID (press + release).
 */
bool ble_hid_kb_send_key(uint8_t hid_keycode);

/**
 * @brief Presiona una tecla con modificadores.
 * @param modifiers  Mascara de modificadores (KEY_MOD_*)
 * @param keycode    Código HID de la tecla
 */
bool ble_hid_kb_press_key(uint8_t modifiers, uint8_t keycode);

/**
 * @brief Suelta todas las teclas (keyboard + media).
 */
void ble_hid_kb_release_all(void);

/**
 * @brief Envía un string completo como teclado HID.
 */
bool ble_hid_kb_send_text(const char *text);

/**
 * @brief Actualiza el nivel de batería reportado al host.
 */
void ble_hid_kb_set_battery(uint8_t level);

#ifdef __cplusplus
}
#endif

#endif /* BLE_HID_KB_H */
