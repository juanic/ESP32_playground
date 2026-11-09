# Servidor local de detección de personas

Servicio HTTP mínimo que recibe las fotos del ESP32-CAM, corre YOLO y
devuelve las personas detectadas en texto plano. Es la contraparte de la app
[`5_people_counter`](../firmware/apps/5_people_counter).

```
server/
  detector_server.py    servidor (POST /detect, GET /health)
  requirements.txt      ultralytics + pillow
  run.ps1               lanzador que usa el .venv local
  .venv/                entorno virtual (uv, Python 3.11)
  yolo11n.pt            pesos locales (no se re-descargan)
```

Todo queda dentro de esta carpeta: el entorno, PyTorch y los pesos. No hace
falta tocar el resto del repositorio ni el Python de ESP-IDF.

---

## Arrancar

```powershell
cd server
.\run.ps1
```

Por defecto escucha en `0.0.0.0:8080` con `imgsz=960`. Opciones:

```powershell
.\run.ps1 --imgsz 640          # más rápido en CPU
.\run.ps1 --device 0           # primera GPU NVIDIA
.\run.ps1 --conf 0.35          # umbral por defecto si el ESP32 no lo manda
.\run.ps1 --port 8080 --verbose
```

Comprobar que responde:

```powershell
curl http://127.0.0.1:8080/health
```

Desde otro equipo de la red (`<IP_DEL_PC>`):

```powershell
curl http://<IP_DEL_PC>:8080/health
```

| Opción | Por defecto | Descripción |
|---|---|---|
| `--host` | `0.0.0.0` | interfaz de escucha |
| `--port` | `8080` | puerto TCP |
| `--model` | `yolo11n.pt` de esta carpeta | modelo de Ultralytics |
| `--imgsz` | `960` | tamaño de inferencia |
| `--conf` | `0.5` | umbral si el ESP32 no manda `X-Threshold` |
| `--device` | auto | `cpu`, `0`, `cuda:0`, ... |
| `--verbose` | — | log de depuración |

Con GPU conviene `--imgsz 1280 --device 0`; en CPU, `--imgsz 640` es más
rápido y suele alcanzar para una oficina.

---

## Recrear el entorno

El `.venv` y los pesos ya están instalados en esta carpeta. Si hace falta
rehacerlo (otra máquina, otro Python):

```powershell
cd server
uv venv .venv --python 3.11
uv pip install --python .venv\Scripts\python.exe -r requirements.txt
curl.exe -L -o yolo11n.pt `
  https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11n.pt
```

Sin `uv`, sirve el camino clásico:

```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
```

---

## Protocolo

```
POST /detect
Content-Type: image/jpeg
X-Threshold: 0.50

<jpeg>
```

Respuesta:

```
count=2
box=0.121,0.304,0.082,0.421,0.913
box=0.548,0.281,0.101,0.447,0.874
```

- `box=x,y,ancho,alto,confianza`, coordenadas normalizadas 0..1.
- Solo se devuelve la clase `person` (COCO 0).
- `GET /health` responde JSON con el estado del modelo.
- Errores: `400` JPEG inválido, `413` cuerpo demasiado grande, `503` modelo
  todavía cargando.

---

## Configurar el ESP32

```powershell
python %IDF_PATH%\tools\idf.py -C firmware/apps/5_people_counter menuconfig
```

*People counter example* → *Detection server URL*:
`http://<IP_DEL_PC>:8080/detect`.

El valor por defecto del proyecto es `http://192.168.0.70:8080/detect`.
El ESP32 y el PC tienen que estar en la misma red.

---

## Firewall

En Windows, permitir el puerto en la red privada (PowerShell como
administrador):

```powershell
New-NetFirewallRule -DisplayName "People detector" -Direction Inbound `
  -Protocol TCP -LocalPort 8080 -Action Allow -Profile Private
```

---

## Prueba rápida con una foto

```powershell
curl -X POST --data-binary "@foto.jpg" `
  -H "Content-Type: image/jpeg" -H "X-Threshold: 0.5" `
  http://127.0.0.1:8080/detect
```

---

## Notas

- El servidor es mono-proceso con un lock alrededor del modelo: las
  peticiones se atienden de a una, que es lo adecuado para una captura cada
  cinco minutos.
- Si el ESP32 no puede conectarse, revisar IP, puerto, firewall y que el
  equipo esté encendido. La foto queda marcada como pendiente en el CSV y se
  reintenta en el siguiente ciclo.
- El modelo corre en CPU por defecto. Con GPU NVIDIA, `--device 0` baja el
  tiempo de inferencia de ~1 s a decenas de milisegundos.
