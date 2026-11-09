# firmware_S3

Firmware para el módulo **ESP32-S3** (probado en la placa **XIAO ESP32-S3**),
desarrollado con **ESP-IDF v6.0.1** y organizado en capas reutilizables
(HAL, BSP y middleware) más una carpeta de aplicaciones de ejemplo.

## Estructura del proyecto

```
firmware_S3/
├── middleware/        # Servicios lógicos intermedios (p.ej. FSM, HTTP server,
                       # animaciones de LEDs)
├── board_support/     # Board Support Package (LEDs de la placa, definiciones
                       # de pines SD y botón)
├── drivers_hal/       # Hardware Abstraction Layer: GPIO, touch, HID (BLE/USB),
                       # SD card, botón con antirrebote, MSC USB, LED
└── apps/              # Aplicaciones de ejemplo
    ├── 0_blinking/    # Parpadeo de LED (plantilla mínima)
    ├── 1_touch/       # Lectura de pines táctiles capacitivos con detección
    #                   relativa (baseline + delta). Ideal para calibrar
    #      umbrales antes de usar en 4_hid_touch.
    ├── 2_ble_hid/     # HID por Bluetooth Low Energy (teclas multimedia)
    ├── 3_usb_hid/     # HID por USB (TinyUSB) - teclas multimedia cíclicas
    ├── 4_hid_touch/   # HID táctil: cada pad envía una tecla multimedia
    ├── 5_usb_msc_laser/  # **NUEVA APP**: Server <-> MSC mode switch
    │                   # (botón físico alterna entre modo HTTP server
    # y modo USB MSC para controladora de láser).
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
- **nueva capa agregada**:
  - **sd_hal**: Montaje/desmontaje de FAT en microSD (`sd_mount`, `sd_unmount`).
  - **button_hal**: Botón con antirrebounce software (`btn_hal_init`, `btn_hal_read`).
  - **tinyusb_msc**: Controlador TinyUSB MSC (init, connect, disconnect).
  - **LED wrapper**: Control de la LED addressable de la placa.

El transporte se elige en `drivers_hal/Kconfig` con
`CONFIG_HID_TRANSPORT_NONE` / `_BLE` / `_USB` / `_BOTH` (por defecto **None**,
de modo que las apps que no usan HID — como `1_touch` — no arrastran Bluetooth
ni TinyUSB).

## Requisitos

- ESP-IDF **v6.0.1** (IDF en `C:\esp\v6.0.1\esp-idf`, herramientas en
  `C:\Espressif\tools`).
- Placa **ESP32-S3**. Las apps USB / HID táctil usan el USB nativo, ideal en
  la XIAO ESP32-S3.

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
- **Ninguno (default)** (`1_touch`, `usb_msc_laser`):
  ```
  CONFIG_HID_TRANSPORT_NONE=y
  ```

## App `1_touch` (Depuración de touch)

Depura la lectura de los pines táctiles T4..T9 del ESP32-S3 y muestra los
valores crudos y la desviación respecto a la baseline capturada al arranque.
Útil para afinar `TOUCH_DELTA_TRIGGER` (200) antes de usar en `4_hid_touch`.

Mapeo T4..T9 → GPIO 5..10. El `d=` mostrado en consola es la desviación
respecto a la baseline; el `*` indica que supera el umbral de 200.

## App `usb_msc_laser_bridge` (Server <-> MSC)

Esta aplicación alterna entre dos modos de operación en el XIAO ESP32S3 Sense:

- **Modo SERVIDOR** (por defecto): El ESP32-S3 actúa como un servidor HTTP
  simple con un endpoint `/upload` que recibe archivos y los guarda en la
  microSD. El botón mantiene este modo.

- **Modo PENDRIVE**: Al pulsar el botón, el dispositivo se transforma en un
  pendrive USB MSC (Mass Storage Class) para la controladora del láser. La
  microSD es el almacenamiento interno. Las funciones clave:
  - **Transición Servidor → PENDRIVE**:
    * Desactiva el servidor HTTP.
    * Desmonta el sistema de archivos FAT.
    * Inicializa TinyUSB MSC y llama a `tud_connect()` para que el host
      enumerate el drive sin replug físico.
  - **Transición PENDRIVE → Servidor**:
    * Llama a `tud_disconnect()` primero (eject prolijo).
    * Espera ~250 ms para que el host termine de leer/escribir.
    * Vuelve a montar el sistema de archivos FAT local.
    * Reactiva el servidor HTTP.

- **LED indicador**: El LED addressable de la placa cambia de color/patrón
  según el modo actual (verde = Servidor, azul = PENDRIVE, rojo = transición).
  El patrón de "transición" parpadea durante el cambio de modo.

- **Alimentación**: El dispositivo queda alimentado permanentemente por el
  puerto USB de la controladora del láser (bus-powered). No asumir que hay
  alimentación externa.

## Estructura de dependencias

```
apps/              ──────────► board_support, middleware
board_support/ ──────► drivers_hal
middleware/ ───────► board_support, drivers_hal
drivers_hal/ ────────► APIs de ESP-IDF directamente
```

## Autor

Juan Cerrudo