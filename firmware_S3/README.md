# firmware_S3

Firmware para el módulo **ESP32-S3** (probado en la placa **XIAO ESP32-S3**),
desarrollado con **ESP-IDF v6.0.1** y organizado en capas reutilizables
(HAL, BSP y middleware) más una carpeta de aplicaciones de ejemplo.

## Estructura del proyecto

```
firmware_S3/
├── middleware/        # Servicios lógicos intermedios (p.ej. animaciones de LEDs)
├── board_support/     # Board Support Package (LEDs de la placa)
├── drivers_hal/       # Hardware Abstraction Layer: GPIO, touch, HID (BLE/USB)
└── apps/              # Aplicaciones de ejemplo
    ├── 0_blinking/    # Parpadeo de LED (plantilla mínima)
    ├── 1_touch/       # Lectura de pines táctiles capacitivos
    ├── 2_ble_hid/     # HID por Bluetooth Low Energy (teclas multimedia)
    ├── 3_usb_hid/     # HID por USB (TinyUSB) - teclas multimedia cíclicas
    ├── 4_hid_touch/   # HID táctil: cada pad envía una tecla multimedia
    └── x_template/    # Plantilla genérica para crear nuevas apps
```

## Capa `drivers_hal`

- **gpio_hal**: configuración y manejo de los GPIO del ESP32-S3.
- **touch_hal**: driver del sensor táctil capacitivo (versión 2 del hardware,
  propia del ESP32-S3). En este SoC la lectura **aumenta** al tocar el pad.
  API: `TouchHalInit`, `TouchHalChannelConfig`, `TouchHalStart`, `TouchHalRead`,
  `TouchHalIsTouched`, `TouchHalDeinit`. El enum de canales es `touch_t`
  (`TOUCH_PAD_0` = GPIO_1 … `TOUCH_PAD_13` = GPIO_14).
- **hid (transporte seleccionable)**:
  - `ble_hid_kb` : HID over GATT (HOGP) con el componente `esp_hid`.
  - `usb_hid_kb` : HID Consumer Control por USB nativo con `esp_tinyusb` (TinyUSB).
  - Ambos exponen la **misma API** (`*_driver_init`, `send_key(media_key_t)`,
    `is_connected`) y comparten el tipo `media_key_t` en `hid_common.h`.

El transporte se elige en `drivers_hal/Kconfig` con
`CONFIG_HID_TRANSPORT_NONE` / `_BLE` / `_USB` / `_BOTH` (por defecto **None**,
de modo que las apps que no usan HID — como `1_touch` — no arrastran Bluetooth
ni TinyUSB).

## Requisitos

- ESP-IDF **v6.0.1** (IDF en `C:\esp\v6.0.1\esp-idf`, herramientas en
  `C:\Espressif\tools`).
- Placa **ESP32-S3**. Las apps USB / HID táctil (`3_usb_hid`, `4_hid_touch`)
  usan el USB nativo, ideal en la XIAO ESP32-S3.

## Compilación

Desde la carpeta de la app deseada:

```bat
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

> **Nota de configuración:** no edites `sdkconfig` directamente; usa
> `sdkconfig.defaults` (se regenera al ejecutar `idf.py set-target`).

### Opciones `sdkconfig.defaults` por transporte

- **USB HID** (`3_usb_hid`, `4_hid_touch`):
  ```
  CONFIG_HID_TRANSPORT_USB=y
  CONFIG_TINYUSB_ENABLED=y
  CONFIG_TINYUSB_HID_COUNT=1
  ```
- **BLE HID** (`2_ble_hid`):
  ```
  CONFIG_HID_TRANSPORT_BLE=y
  CONFIG_BT_ENABLED=y
  CONFIG_BTDM_CTRL_MODE_BLE_ONLY=y
  CONFIG_BT_BLUEDROID_ENABLED=y
  CONFIG_BT_BLE_ENABLED=y
  CONFIG_BT_HID_DEVICE_ENABLED=y
  CONFIG_BT_GATTS_ENABLE=y
  ```

## App `4_hid_touch` (HID táctil)

Cada canal táctil se mapea a una tecla multimedia (play/pause, next, prev,
vol+, vol−, stop). Al arrancar se **calibra la línea base** de cada canal
(promedio de 32 muestras); luego se dispara la tecla cuando la lectura supera
la base en **200** puntos (detección por flanco, un solo envío aunque se mantenga
el dedo).

Mapeo en `s_map[]` (`apps/4_hid_touch/main/4_hid_touch.c`):
T4=Next, T5=Vol+, T6=Pause, T7=Play, T8=Vol−, T9=Back
(`TOUCH_PAD_4..9` → GPIO 5..10). Ajusta la tabla a los pines que realmente
uses en tu placa. Para observar los valores crudos de esos canales y calibrar
el umbral, compila la app `1_touch`.

## Componentes locales (USB)

Para el transporte USB, `esp_tinyusb` y `tinyusb` se incluyen como componentes
**locales** bajo `apps/<app>/components/` (sin metadatos de gestor de
componentes), por lo que no requieren conexión ni `idf_component.yml`.

## Autor

Juan Cerrudo
