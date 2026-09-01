# 4_hid_touch — Controlador HID táctil (USB o BLE) para ESP32-S3

Convierte los pads capacitivos del ESP32-S3 en un **HID Consumer Control**
(teclas multimedia). Cada canal táctil está asignado a una tecla; al tocar un
pad se envía esa tecla al host conectado (PC/telefono), por **USB nativo** o por
**Bluetooth LE**, segun el transporte activo.

A diferencia de los ejemplos ciclicos `2_ble_hid`/`3_usb_hid`, aqui cada pad
dispara **su propia tecla** y la deteccion es por flanco: mantener el dedo
enviara la tecla una sola vez.

## Funcionamiento

El flujo está en [main/4_hid_touch.c](main/4_hid_touch.c):

1. `hid_init()` inicializa el transporte HID activo (USB y/o BLE) y
   `TouchHalInit()` el sensor táctil (`drivers_hal/touch_hal`).
2. Se configuran los canales de la tabla `s_map[]` y se arranca el escaneo
   continuo.
3. Se espera 1 s para que el filtro del sensor se asiente (las primeras
   lecturas tras arrancar pueden dispararse a valores enormes).
4. **Calibracion de línea base**: se promedian 32 muestras de cada canal
   (`TOUCH_CAL_SAMPLES`). En el ESP32-S3 la lectura **aumenta** al tocar el pad
   (al revés del ESP32 clásico), así que la detección compara contra la base:
   `valor >= base + TOUCH_DELTA_TRIGGER (250)`.
5. En el bucle principal: si no hay host conectado, se imprime
   "waiting for a HID host..." cada 2 s con un periodo lento (200 ms); cuando
   hay host se pollan los pads cada 20 ms y se envía la tecla en el flanco de
   subida.

### Mapa táctil -> tecla

```c
static const touch_map_t s_map[] = {
    { TOUCH_PAD_4, MEDIA_KEY_NEXT,     "NEXT" },
    { TOUCH_PAD_5, MEDIA_KEY_VOL_UP,   "VOL+" },
    { TOUCH_PAD_6, MEDIA_KEY_PAUSE,    "PAUSE" },
    { TOUCH_PAD_7, MEDIA_KEY_PLAY,     "PLAY" },
    { TOUCH_PAD_8, MEDIA_KEY_VOL_DOWN, "VOL-" },
    { TOUCH_PAD_9, MEDIA_KEY_PREV,     "BACK" },
};
```

En el ESP32-S3 `TOUCH_PAD_n` == `GPIO_(n+1)`, así que T4–T9 son **GPIO 5–10**.
Ajusta `s_map[]` a los pads que realmente cables en tu placa (evita los
`TOUCH_PAD_0/2/3`, compartidos con pines de bootstrapping). Para ver los valores
crudos y calibrar el umbral puedes usar la app `1_touch`.

## Cómo cambiar de USB a BLE

El transporte se elige únicamente por **Kconfig** en
`drivers_hal/Kconfig` (`CONFIG_HID_TRANSPORT_USB` / `_BLE` / `_BOTH`). La
aplicación compila solo el driver del transporte seleccionado (y llama a la API
pública, idéntica para ambos: `*_driver_init`, `send_key()`, `is_connected()`).

### 1. Editar `sdkconfig.defaults`

El archivo ya trae ambos bloques listos; solo hay que **comentar** USB y
**descomentar** BLE:

```ini
# --- USB transport (active) ---
# CONFIG_HID_TRANSPORT_USB=y
# CONFIG_TINYUSB_HID_COUNT=1

# --- BLE transport (uncomment and comment USB block above to use) ---
CONFIG_HID_TRANSPORT_BLE=y
CONFIG_BT_ENABLED=y
CONFIG_BT_BLUEDROID_ENABLED=y
CONFIG_BT_CONTROLLER_ENABLED=y
CONFIG_BT_GATTS_ENABLE=y
CONFIG_BT_BLE_42_FEATURES_SUPPORTED=y
```

- `CONFIG_BT_BLE_42_FEATURES_SUPPORTED=y` es necesario porque el driver usa
  *advertising* legacy (y en S3 las features 4.2 vienen apagadas por defecto).
- `CONFIG_TINYUSB_HID_COUNT=1` solo aplica al transporte TinyUSB.

### 2. Reconfigurar y compilar

```bat
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

`set-target` regenera `sdkconfig` a partir de `sdkconfig.defaults` (no edites
`sdkconfig` a mano). También puedes cambiarlo interactivamente con
`idf.py menuconfig` → **Component config → HID Driver (drivers_hal) → Active
HID transport**.

### 3. Usarlo

- **USB**: conecta la placa por cable de datos (USB nativo, conector de la
  XIAO ESP32-S3).
- **BLE**: el dispositivo aparece con el nombre **ESP32-S3 Touch** (lo fija
  `ble_hid_driver_init("ESP32-S3 Touch")` en `app_main`); pearéalo desde el
  host. No hace falta cable.
- En ambos casos mira el monitor serie para ver la calibración de cada canal y
  la tecla enviada.

### Opción BOTH (USB + BLE simultaneo)

```ini
CONFIG_HID_TRANSPORT_BOTH=y
CONFIG_TINYUSB_HID_COUNT=1
CONFIG_BT_ENABLED=y
CONFIG_BT_BLUEDROID_ENABLED=y
CONFIG_BT_CONTROLLER_ENABLED=y
CONFIG_BT_GATTS_ENABLE=y
CONFIG_BT_BLE_42_FEATURES_SUPPORTED=y
```

## Notas de build

- **No listes `COMPONENTS` explícitamente** en el `CMakeLists.txt` de la app:
  el transporte USB depende de los componentes locales `esp_tinyusb`/`tinyusb`
  (bajo `components/`), que `drivers_hal` trae por `PRIV_REQUIRES` y se
  descubren automáticamente desde la carpeta `components/` del proyecto.
- En `drivers_hal/CMakeLists.txt` los REQUIRES de la pila BT (`bt esp_hid`)
  son **incondicionales** (IDF resuelve los REQUIRES antes de tener los
  `CONFIG_*`, así que un `if(CONFIG_HID_TRANSPORT_BLE...)` alrededor fallaría
  en el early-pass). El gating por CONFIG solo decide qué source se compila;
  `esp_tinyusb`/`tinyusb`, al ser locales, sí quedan condicionados al
  transporte USB.