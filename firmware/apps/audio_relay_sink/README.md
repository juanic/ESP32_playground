# Audio Relay Sink

## Descripcion

Receptor de audio Bluetooth A2DP que envia la salida PCM por I2S. Se conecta a un celular/PC como parlante Bluetooth y reenvia el audio por I2S a la placa `audio_relay_source`.

```
[Celular/PC] --A2DP--> [ESP32 #1: audio_relay_sink] --I2S--> [ESP32 #2: audio_relay_source]
      |--BLE: App de control (GATT)-->|
```

Ademas del audio A2DP, el sink expone un **canal de control BLE (GATT)** para que una App pueda gestionar la coneccion, filtrar MACs, habilitar/deshabilitar la transmision de audio, consultar metadata (AVRCP) y cambiar la contrasena de acceso.

## Hardware

| Señal | GPIO |
|-------|------|
| BCLK  | 26   |
| WS    | 25   |
| DATA  | 22   |
| GND   | comun con ESP32 #2 |

El sink opera como **maestro I2S**: genera `BCLK` y `WS` y transmite el
PCM por `DATA`. Conectar BCLK-BCLK, WS-WS, DATA-DATA y GND con la placa
source; no conectar otro dispositivo que genere esos relojes.

### Control inter-placa (UART)

| Señal | GPIO sink | GPIO source |
|-------|:---------:|:-----------:|
| TX    | 32        | RX 32       |
| RX    | 33        | TX 33       |
| GND   | común     | común       |

El enlace usa UART a 115200, 8N1. Debe cablearse cruzado: GPIO 32 del sink
(TX) a GPIO 32 del source (RX), y GPIO 33 del source (TX) a GPIO 33 del sink
(RX). El protocolo usa lineas ASCII terminadas en `\n`.

## Compilar y flashear

```bash
idf.py set-target esp32
idf.py build
idf.py -p <PUERTO> flash monitor
```

## Configuracion minima (sdkconfig)

El archivo `sdkconfig.defaults` ya configura el modo dual (BLE + Classic) sobre Bluedroid:

- `CONFIG_BT_ENABLED=y` — Bluetooth habilitado
- `CONFIG_BT_BLUEDROID_ENABLED=y` — Stack Bluedroid
- `CONFIG_BT_BLE_ENABLED=y` — Host BLE habilitado (canal de control)
- `CONFIG_BT_CLASSIC_ENABLED=y` — Classic BT (A2DP)
- `CONFIG_BTDM_CTRL_MODE_BTDM=y` — Controlador dual mode (BLE + BR/EDR)
- `CONFIG_BT_A2DP_ENABLE=y` — Perfil A2DP (incluye AVRCP)
- `CONFIG_BT_GATTS_ENABLE=y` — GATT server BLE
- `CONFIG_PARTITION_TABLE_DEFAULT=y` — Particion NVS disponible

> Nota: el controlador dual consume mas RAM que el modo classic-only. Si el firmware no entra en memoria, ajusta `CONFIG_BTDM_CTRL_BLE_MAX_CONN` y `CONFIG_BT_ACL_CONNECTIONS` en `menuconfig`.

## Uso

1. Flashear el firmware en un ESP32
2. Encender el monitor serie (`idf.py monitor`)
3. El dispositivo aparece como **"Audio Relay Sink"** en la busqueda Bluetooth (classic A2DP) y como **"Audio Relay CTL"** en la busqueda BLE
4. Emparejar desde el celular/PC y reproducir audio — el PCM sale por los pines I2S configurados
5. Conectar la **App de control** por BLE para gestionar el dispositivo

## Canal de control BLE (GATT)

Expone el **Nordic UART Service (NUS)** — UUID **`6E400001-B5A3-F393-E0A9-E50E24DCCA9E`** — para poder usar librerias/clientes NUS estandar (nRF Connect, SDKs NUS Android/iOS) sin necesidad de descubrimiento GATT custom:

| Caracteristica | UUID | Propiedades |
|----------------|------|-------------|
| RX (comandos)  | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` | Write / Write No Response |
| TX (respuestas + eventos) | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` | Notify |

- La App **escribe** comandos en texto ASCII (terminados en `\n`) en `RX`
- El firmware **responde** y **emite eventos no solicitados** por la misma caracteristica `TX` (NUS solo define un canal de salida)
- Se envian como **notifications** (no requieren indicate/ACK), pero sigue siendo obligatorio habilitar notificaciones escribiendo `0x0100` en el CCCD (`0x2902`) de `TX` — la mayoria de librerias NUS lo hacen automaticamente
- El UUID del servicio va en el **scan response** (no entra en el paquete de advertising junto con el nombre)

### Comandos

Los comandos con `*` requieren autenticacion previa (`AUTH`).

| Comando | Argumento | Descripcion |
|---------|-----------|-------------|
| `AUTH` | `<password>` | Autentica con la contrasena de acceso. Resp: `AUTH OK` / `AUTH FAIL` |
| `SETPASS *` | `<nueva>` | Cambia la contrasena de acceso (persistida en NVS) |
| `TX *` | `0` / `1` | Deshabilita / habilita la transmision de audio a I2S |
| `DISCONNECT *` | — | Desconecta el peer A2DP actual |
| `WL_ADD *` | `<AA:BB:CC:DD:EE:FF>` | Agrega MAC a la whitelist |
| `BL_ADD *` | `<AA:BB:CC:DD:EE:FF>` | Agrega MAC a la blacklist |
| `REMOVE *` | `<AA:BB:CC:DD:EE:FF>` | Quita la MAC de ambas listas |
| `LIST` | — | Reporta el peer actual y si esta en whitelist/blacklist |
| `STATUS` | — | Estado: conexion, TX, autenticado, peer, hora |
| `GET_META` | — | Metadata "now playing" (titulo, artista, album, estado) |
| `SYNC_TIME *` | `<epoch_unix>` | Sincroniza el reloj del dispositivo (hora actual en segundos Unix) |
| `GRANT *` | `<AA:BB:CC:DD:EE:FF>` `<min>` | Concede a esa MAC `min` minutos de reproduccion desde ahora |
| `SCAN_TARGET *` | — | Pide al source buscar parlantes Bluetooth cercanos |
| `SET_TARGET *` | `<nombre_o_MAC>` | Configura y persiste el parlante destino en el source |
| `GET_TARGET` | — | Consulta el target y estado de conexión del source |

### Respuestas (RSP)

- `AUTH OK` / `AUTH FAIL`
- `ERR AUTH_REQUIRED` — comando restringido sin autenticar
- `ERR ARG` — falta el argumento o formato invalido
- `ERR MAC` — formato de MAC invalido
- `ERR UNKNOWN` — comando desconocido
- `ERR NO_TIME` — `GRANT` pedido sin reloj sincronizado
- `ERR TIME` — fallo al guardar la referencia de hora
- `ERR FULL` — no quedan slots de concesion
- `STATUS conn=... tx=... auth=... peer=... time=... now=...`
- `META title=... artist=... album=... play=...`
- `PEER <mac> WL=... BL=... GRANT=... EXP=...`
- `TIME OK epoch=...` / `GRANT OK mac=... expiry=...`
- `TARGET OK name=... mac=...` — target guardado por el source
- `TARGET name=... mac=... connected=0|1` — estado del target configurado
- `ERR TARGET_TIMEOUT` — el source no respondió por UART en dos segundos

### Eventos (EVT)

- `A2DP connected=1 peer=<mac>` / `A2DP connected=0`
- `TX 1` / `TX 0`
- `PASSWORD_CHANGED`
- `META title=... artist=... album=... play=... track_new=...`
- `TIME epoch=...`
- `GRANT mac=... minutes=...`
- `SPEAKER_FOUND name=... mac=...`
- `SPEAKER_CONNECTED name=... mac=...` / `SPEAKER_DISCONNECTED name=... mac=...`

### Contrasena por defecto

`relay1234`. Se guarda en NVS (espacio `relay`, clave `passwd`) en el primer arranque y puede cambiarse con `SETPASS`.

## Limite por tiempo

Se puede conceder a una MAC especifica una ventana de uso desde el momento de la concesion (ej. "30 minutos a partir de ahora"). El control por tiempo opera **en paralelo e independiente** del filtro por MAC: aunque la MAC este permitida, si su ventana vencio se corta la reproduccion (y se desconecta).

### Como funciona (sin pila RTC)

El ESP32 no tiene reloj persistente ni pila. La hora la aporta la **App**:

1. La App se autentica (`AUTH`) y envia la hora real en segundos Unix con `SYNC_TIME <epoch>`.
2. El firmware guarda esa referencia (epoch + uptime) en NVS y la mantiene mientras este encendido via `drivers_hal/time_hal.c`.
3. La App concede tiempo con `GRANT <mac> <minutos>` (requiere reloj sincronizado).
4. El firmware guarda las concesiones por MAC en NVS y las evalua al decidir si reproducir (`RelayControlIsAllowedToPlay`).

- **Concesion vigente** -> la MAC puede reproducir (si ademas pasa el filtro MAC).
- **Concesion vencida** -> la MAC se rechaza aunque este en whitelist.
- **Sin concesion para esa MAC** -> solo aplica el filtro MAC normal.
- **Sin reloj sincronizado** -> una MAC con concesion se rechaza (fail-closed) hasta que la App sincronice la hora.

### Secuencia tipica de la App

```
AUTH <password>                    -> AUTH OK
SYNC_TIME 1750000000               -> TIME OK epoch=...
GRANT AA:BB:CC:DD:EE:FF 30         -> GRANT OK mac=... expiry=...
```

> El reloj virtual se pierde si se corta la alimentacion y no se re-sincroniza: la App debe enviar `SYNC_TIME` (idealmente despues de `AUTH`) en cada conexion.

## Metadata AVRCP

El sink actua como **AVRCP Controller** y consulta la metadata "now playing" al telefono (fuente A2DP): titulo, artista, album y estado de reproduccion. Al cambiar de track o de estado, se emite `EVT META ...`. El modulo lo maneja `drivers_hal/avrcp_hal.c`.

## Acceso por MAC

El middleware `bt_access_control` filtra dispositivos por MAC:

- **Sin whitelist configurada**: cualquier dispositivo se conecta (excepto los en la blacklist)
- **Con whitelist**: solo los dispositivos en la whitelist pueden conectarse
- **Blacklist**: los dispositivos en la blacklist son rechazados siempre

Las listas se gestionan en tiempo de ejecucion desde la App (`WL_ADD`, `BL_ADD`, `REMOVE`) o en codigo:
```c
uint8_t bda[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
AccessControlAdd(bda, ACL_WHITELIST);
AccessControlAdd(bda, ACL_BLACKLIST);
AccessControlRemove(bda);
```

## Capas

```
apps/audio_relay_sink/main.c          (orquestacion + gate de audio)
middleware/relay_control.c            (plano de control: auth, TX, filtros, tiempo, eventos)
middleware/bt_access_control.c        (logica de acceso por MAC)
board_support/audio_relay_sink_bsp.c  (config de placa)
drivers_hal/ble_control_hal.c         (transporte BLE GATT: NUS RX/TX)
drivers_hal/avrcp_hal.c               (metadata AVRCP)
drivers_hal/i2s_hal.c                 (wrapper I2S)
drivers_hal/bt_classic_hal.c          (wrapper A2DP + dual-mode controller)
drivers_hal/time_hal.c                (reloj virtual: epoch App + uptime)
middleware/inter_board_link.c         (protocolo de control UART entre placas)
drivers_hal/uart_link_hal.c           (transporte UART ASCII)
```

La transmision de audio se gatesa en `main.c` (`on_audio_data`) segun `RelayControlIsTxEnabled()`.

## Notas

- El A2DP Sink y Source no pueden correr simultaneamente en el mismo ESP32 (limitacion de Bluedroid)
- El sample rate por defecto es 44.1 kHz stereo 16-bit
- El sink genera el reloj I2S y envia el audio PCM sin procesamiento adicional
- BLE (control) y Classic (audio) coexisten sobre el mismo stack Bluedroid en modo dual
