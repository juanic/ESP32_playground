# 4_cam_stream — Stream MJPEG con ESP32-CAM

Ejemplo mínimo para **ESP-IDF** (no Arduino) que inicializa una cámara DVP
OV2640, se conecta a Wi-Fi y sirve un stream MJPEG desde un servidor HTTP
embebido en el ESP32. Se visualiza directamente desde el navegador.

```
Navegador  <--HTTP/MJPEG-->  ESP32-CAM  <--DVP/SCCB-->  OV2640
```

## Hardware

Pensado para el módulo **AI-Thinker ESP32-CAM** (ESP32 + OV2640 + 4 MB PSRAM,
4 MB flash).

| Señal            | ESP32-CAM |
|------------------|-----------|
| Alimentación     | 5 V       |
| GND              | GND       |
| TX (programador) | GPIO1     |
| RX (programador) | GPIO3     |
| GPIO0            | GND para flashear |

> [!IMPORTANT]
> Alimentá el módulo con **5 V y una fuente capaz de entregar al menos 500 mA**.
> Alimentarlo desde el pin 3.3 V de un FTDI suele provocar reinicios o imágenes
> corruptas. Para flashear, uní GPIO0 a GND, reseteá, programá y luego retirá el
> jumper y reseteá de nuevo para ejecutar.

## Configuración

Las credenciales y la calidad del stream se configuran por Kconfig:

```bash
cd firmware/apps/4_cam_stream
idf.py menuconfig
```

Ruta: **ESP32-CAM stream example**

| Opción | Descripción | Default |
|--------|-------------|---------|
| `CAM_STREAM_WIFI_SSID` | Nombre de la red Wi-Fi | `myssid` |
| `CAM_STREAM_WIFI_PASSWORD` | Clave WPA2 (vacía para red abierta) | `mypassword` |
| `CAM_STREAM_FRAME_SIZE` | QVGA / VGA / SVGA / XGA | VGA |
| `CAM_STREAM_JPEG_QUALITY` | 0–63, menor = mejor calidad | 12 |
| `CAM_STREAM_FB_COUNT` | Cantidad de frame buffers | 2 |

También podés editar directamente `sdkconfig.defaults` y volver a compilar.

## Compilar y flashear

```bash
cd firmware/apps/4_cam_stream
idf.py set-target esp32
idf.py build
idf.py -p COMx flash monitor
```

La primera compilación descarga automáticamente el componente
`espressif/esp32-camera` mediante el component manager de ESP-IDF.

En el monitor serie vas a ver algo como:

```
I (1234) wifi_hal: IP obtenida: 192.168.0.42
I (1234) cam_stream: Visor listo en http://192.168.0.42/
```

Abrí esa URL desde cualquier dispositivo conectado a la misma red.

## Endpoints

| Ruta       | Descripción |
|------------|-------------|
| `/`        | Página HTML con el visor |
| `/stream`  | Stream MJPEG (`multipart/x-mixed-replace`) |
| `/capture` | Una captura JPEG |

Sólo un cliente puede usar la cámara a la vez. Si abrís otra pestaña o pedís
`/capture` mientras hay un stream activo, la respuesta es `503`.

El stream se puede consumir desde VLC, ffmpeg u otra aplicación:

```bash
ffplay http://192.168.0.42/stream
```

## Estructura

| Archivo | Capa | Rol |
|---------|------|-----|
| `drivers_hal/inc/camera_hal.h` | HAL | Inicialización y captura de frames |
| `drivers_hal/inc/wifi_hal.h` | HAL | Wi-Fi STA con reconexión automática |
| `board_support/inc/camera_ai_thinker_bsp.h` | BSP | Pinout del módulo AI-Thinker |
| `middleware/inc/camera_stream_server.h` | Middleware | Servidor HTTP y MJPEG |
| `main/main.c` | App | Orquesta cámara, Wi-Fi y servidor |

La HAL de cámara y la de Wi-Fi son **opcionales** (`CONFIG_CAMERA_HAL_ENABLED`
y `CONFIG_WIFI_HAL_ENABLED`), por lo que el resto de las apps del repositorio
no compilan ni descargan esas dependencias.

## Notas y problemas frecuentes

- **Se ve una sola imagen o el stream se corta:** el ejemplo está pensado para
  un visor por vez. Mientras un cliente consume `/stream`, cualquier otra
  petición a `/stream` o `/capture` recibe `503 Service Unavailable` en lugar
  de bloquear el servidor.
- **`E BOD: Brownout detector was triggered` o reinicios aleatorios:** casi
  siempre es alimentación insuficiente. Al arrancar, la calibración de RF del
  Wi-Fi pide picos de 400-500 mA. Usá una fuente de 5 V / 2 A con GND común y
  un capacitor de 470-1000 µF junto al módulo; el pin de 3.3 V de un CH340 o
  FTDI no alcanza.
- **No hay PSRAM / el módulo es de 2 MB:** la HAL cae a DRAM y VGA puede fallar.
  Bajá la resolución a QVGA y los frame buffers a 1, o ajustá
  `sdkconfig.defaults`.
- **Otra placa o módulo (por ejemplo ESP32-S3-CAM):** creá un BSP equivalente en
  `firmware_S3/board_support` con su pinout; la HAL, el middleware y la app se
  reutilizan cambiando sólo el BSP.
- **La IP cambia:** el ejemplo no fija IP estática; reservala en el router si
  necesitás una URL estable.
- **Latencia:** para menor latencia usá VGA/QVGA y calidad ~15–20. Para más
  detalle, SVGA/XGA con calidad ~10–12.
