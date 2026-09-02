# Audio Relay Source

## Descripcion

Transmisor de audio Bluetooth A2DP que recibe PCM por I2S y lo envia a un
amplificador Bluetooth. Busca y se empareja automaticamente con el receptor
destino configurado.

```
[ESP32 #1: audio_relay_sink] --I2S--> [ESP32 #2: audio_relay_source] --A2DP--> [Amplificador BT]
```

## Hardware

| Señal | GPIO |
|-------|------|
| BCLK  | 26   |
| WS    | 25   |
| DATA  | 22   |
| GND   | comun con ESP32 #1 |

Las patas I2S deben conectarse en espejo con `audio_relay_sink`
(`BCLK-BCLK`, `WS-WS`, `DATA-DATA` y `GND-GND`). El source opera como
**esclavo I2S**: recibe los relojes BCLK y WS generados por el sink. Es
importante no usar dos maestros I2S sobre las mismas lineas de reloj.

El formato actual es PCM estereo, 16 bits y 44.1 kHz.

## Flujo de audio

ESP-IDF 6.x entrega audio a A2DP Source mediante un callback *pull-based*:
el stack pide muestras cuando debe construir una trama. Por ello el firmware
usa un buffer circular PCM de 32 KiB entre los dos contextos de ejecucion:

```
I2S RX (tarea principal) -> buffer PCM -> callback A2DP Source -> amplificador
```

La tarea principal lee hasta 1024 bytes de I2S y los coloca en el buffer. El
callback Bluetooth consume los bytes solicitados; si el buffer no alcanza a
abastecerlos, la HAL completa el resto con silencio para mantener la cadencia
del stream. Si el buffer se llena, se descartan las muestras nuevas y se
registra una advertencia en el monitor serie.

## Compilar y flashear

```bash
idf.py set-target esp32
idf.py build
idf.py -p <PUERTO> flash monitor
```

## Configuracion minima (sdkconfig)

El archivo `sdkconfig.defaults` ya configura:

- `CONFIG_BT_ENABLED=y` — Bluetooth clasico habilitado
- `CONFIG_BT_BLUEDROID_ENABLED=y` — Stack Bluedroid
- `CONFIG_BT_BLE_ENABLED=n` — BLE deshabilitado (no se usa)
- `CONFIG_BT_SSP_ENABLED=y` — Secure Simple Pairing
- `CONFIG_PARTITION_TABLE_DEFAULT=y` — Particion NVS disponible

Si necesitas modificar algo, usa `idf.py menuconfig` y revisa:
- **Component config > Bluetooth > Bluedroid Enable**
- **Partition Table**

## Uso

1. Flashear el firmware en un ESP32
2. Encender el monitor serie (`idf.py monitor`)
3. El dispositivo inicia busqueda automatica del amplificador destino
4. Por defecto busca un dispositivo llamado **"BT-WUZHI"** (configurable en `main.c` via `TARGET_BT_NAME`)
5. Al encontrarlo, se empareja y conecta automaticamente
6. El audio que llega por I2S se transmite por A2DP al amplificador

## Conexion y reconexion

Si se corta la conexion A2DP, el dispositivo reintentara automaticamente cada
3 segundos hasta reconectar. La lectura y el encolado de I2S comienzan cuando
el callback A2DP confirma que el amplificador quedo conectado.

## Capas

```
apps/audio_relay_source/main.c          (orquestacion)
board_support/audio_relay_source_bsp.c  (config de placa)
drivers_hal/i2s_hal.c                   (wrapper I2S)
drivers_hal/bt_classic_hal.c            (wrapper A2DP)
```

## Notas

- El A2DP Sink y Source no pueden correr simultaneamente en el mismo ESP32 (limitacion de Bluedroid)
- El sample rate por defecto es 44.1 kHz stereo 16-bit
- El nombre del dispositivo destino se cambia modificando `TARGET_BT_NAME` en `main/main.c`
- Si no se encuentra el dispositivo destino, la busqueda se repite periodicamente
- El A2DP Source en IDF 6.x es pull-based: el stack llama a un callback para obtener datos PCM
- El buffer PCM desacopla la tarea de I2S del callback Bluetooth y absorbe variaciones breves de ritmo
