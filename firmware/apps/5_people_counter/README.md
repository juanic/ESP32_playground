# 5_people_counter — Contador de personas con ESP32-CAM

Captura una foto **UXGA (1600×1200)** cada 5 minutos, la guarda en la
microSD, la sube a un servidor de detección local (YOLO) y registra cuántas
personas había en un CSV diario. La web embebida muestra la última captura
con las personas recuadradas, permite ajustar la imagen y descargar los
archivos.

No hay streaming en vivo: la cámara solo se usa para las capturas
periódicas y el botón *Capturar ahora*.

---

## Arquitectura

```
apps/5_people_counter        orquestación: NVS → cámara → SD → Wi-Fi → SNTP → web + tarea
middleware
  photo_repository           fotos, .det y CSV diario en la SD
  detector_client            POST del JPEG al PC y parseo de las cajas
  monitor_web_server         UI + API + descargas
board_support
  camera_ai_thinker_bsp      pinout de cámara y montaje de la microSD
drivers_hal
  camera_hal                 OV2640 + controles de imagen
  sd_hal                     SDMMC en modo 1-bit
  wifi_hal                   estación Wi-Fi + SNTP
```

---

## Hardware

| Señal | GPIO |
|---|---|
| Cámara | pinout estándar AI-Thinker (OV2640) |
| microSD CLK | 14 |
| microSD CMD | 15 |
| microSD D0 | 2 |

La tarjeta se monta en **modo 1-bit** para no usar GPIO12 (strapping) ni
GPIO4 (LED de flash). Formatear la tarjeta como **FAT32**.

> La placa necesita una fuente de **5 V / 2 A**. El brownout al inicializar
> la cámara o el Wi-Fi casi siempre es alimentación insuficiente.

---

## Estructura en la tarjeta

```
/sdcard/photos/2026-11-09/19-05-00.jpg    captura original
/sdcard/photos/2026-11-09/19-05-00.det    detecciones en JSON
/sdcard/data/2026-11-09.csv               una fila por captura
```

CSV:

```csv
timestamp,archivo,personas,confianza_media,confianza_min,confianza_max,procesado
2026-11-09 19:05:00,19-05-00.jpg,3,87.3,81.2,94.1,1
```

`procesado=0` indica que la foto todavía no fue analizada (por ejemplo, el
PC estaba apagado). La tarea de captura reintenta las fotos pendientes en
cada ciclo y reescribe la fila con los valores definitivos.

---

## Servidor de detección

El ESP32 espera un endpoint que reciba el JPEG por `POST` y responda texto
plano:

```
count=2
box=0.121,0.304,0.082,0.421,0.913
box=0.548,0.281,0.101,0.447,0.874
```

Las cajas van normalizadas `x,y,ancho,alto,confianza` (0..1). El servidor
listo para usar está en [`server`](../../../server).

Configurar la URL en `idf.py menuconfig` →
*People counter example* → *Detection server URL* (por defecto
`http://192.168.0.70:8080/detect`). El PC y el ESP32 deben estar en la misma
red y el firewall tiene que permitir el puerto.

---

## Configuración

`idf.py -C firmware/apps/5_people_counter menuconfig`

| Opción | Por defecto | Descripción |
|---|---|---|
| Wi-Fi SSID / password | — | red de la oficina |
| Capture interval | 300 s | período entre capturas |
| JPEG quality | 10 | 0..63, menor = mejor calidad |
| Detection server URL | `http://192.168.0.70:8080/detect` | endpoint del PC |
| Confidence threshold | 50 % | se puede cambiar en vivo desde la web |
| Pending retry | 3 | fotos pendientes reintentadas por ciclo |
| NTP server | `pool.ntp.org` | requiere internet |
| Timezone | `ART3` | hora local de los nombres y el CSV |
| Work start hour | 10 | hora de inicio de capturas automáticas (inclusive) |
| Work end hour | 17 | hora de fin de capturas automáticas (hasta las 16:59) |
| Work days only | y | capturas automáticas solo de lunes a viernes |

---

## Compilar y flashear

```powershell
python $env:IDF_PATH\tools\idf.py -C firmware/apps/5_people_counter build
python $env:IDF_PATH\tools\idf.py -C firmware/apps/5_people_counter -p COM10 flash monitor
```

Para flashear: jumper `GPIO0` a GND + reset. Para ejecutar: quitar el jumper
y resetear (el reset por RTS del CH340 no siempre funciona; usar el botón).

---

## Web

`http://<IP_DEL_ESP32>/`

| Endpoint | Método | Descripción |
|---|---|---|
| `/` | GET | visor + controles |
| `/api/status` | GET | IP, SD libre, sensor, hora, ajustes |
| `/api/latest` | GET | última foto y sus detecciones |
| `/api/capture` | POST | captura manual inmediata (disponible siempre) |
| `/api/settings` | POST | brillo, contraste, saturación, WB, exposición, ganancia, espejo, calidad y umbral |
| `/api/days` | GET | días guardados y cantidad de fotos |
| `/api/files?day=AAAA-MM-DD` | GET | archivos de un día |
| `/files/...` | GET | descarga (`?dl=1`) de JPG y CSV diarios |
| `/api/delete?day=...[&file=...]` | POST | borrado de fotos individuales o días enteros y actualización de CSV |

Las cajas se dibujan en el navegador sobre la imagen original, así que la
foto descargada queda limpia (sin recuadros grabados).

### Controles de imagen

Brillo, contraste, saturación, balance de blancos (auto/soleado/nublado/
oficina/hogar), exposición automática o manual, techo de ganancia, espejo
horizontal, volteo vertical y calidad JPEG. Los ajustes se aplican al sensor
OV2640 y se conservan mientras la placa esté encendida.

---

## Notas

- Si no hay internet, la hora no se sincroniza y las capturas se omiten
  (para no generar archivos con fecha 1970). El log avisa cada 15 s.
- Si la SD no está presente al arrancar, el servidor web igual funciona y
  el montaje se reintenta en cada ciclo.
- La detección corre en el PC: el ESP32 solo sube la foto y guarda el
  resultado. Esto permite usar YOLO a resolución completa sin límites de
  flash ni de cómputo.
