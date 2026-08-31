# Touch Sweep — Firma capacitiva multi-parámetro

Barrido de 50 configuraciones de excitación sobre un pin touch del ESP32, variando
simultáneamente la velocidad de carga, el nivel de voltaje inicial y los límites
de voltaje de carga/descarga. El objetivo es obtener una "firma capacitiva" que
permita caracterizar la respuesta del sensor ante distintos regímenes de excitación
y detectar variaciones provocadas por distintos gestos (toque, presión, proximidad).

## Principio de funcionamiento

El sensor capacitivo del ESP32 mide el tiempo que tarda un pin en cargarse entre
un voltaje bajo (`volt_lim_l`) y uno alto (`volt_lim_h`). Cuanto mayor sea la
capacitancia conectada al pin (por ejemplo, un dedo tocando), más lenta será la
carga y menor será el valor leído.

Cada medición ("sweep step") reconfigura los parámetros de excitación y ejecuta
un escaneo oneshot, devolviendo un valor crudo (`raw_value`). Al recorrer las 50
configuraciones se obtiene un vector de 50 valores que representa la respuesta
del canal ante distintos regímenes de excitación.

## Parámetros que se varían

| Parámetro | Qty | Valores | Descripción |
|:----------|:---:|:--------|:------------|
| `charge_speed` | 7 | 1–7 | Velocidad de carga/descarga (pendiente). 1 = más lento, 7 = más rápido |
| `init_charge_volt` | 3 | LOW, FLOAT, HIGH | Nivel inicial antes de medir. FLOAT = pin en alta impedancia entre mediciones |
| `volt_lim_l` | 3 | 0.5V, 0.6V, 0.7V | Límite inferior del rango de voltaje durante la descarga |
| `volt_lim_h` | 5 | 1.5V, 1.7V, 2.0V, 2.2V, 2.5V | Límite superior del rango de voltaje durante la carga |
| `charge_duration_ms` | 1 | **5.0 ms (fijo)** | Ventana de muestreo — no se varía |

Combinaciones cíclicas: 7 × 3 × 3 × 5 = 315 posibles, pero se muestrean 50 pasos
(en teoría los primeros 50 de la secuencia cíclica). Si se necesitan todas las
combinaciones, cambiar `STEP_COUNT` a 315.

## Flujo de ejecución

```
1. Init         TouchHalInit() + TouchHalSetSampleConfig(5ms) + TouchHalChannelConfig()
2. Calibración  3 sweeps completos → min/max baseline para escala del gráfico
3. Loop         Cada 500 ms:
                  a. Recorrer 50 configuraciones → raw_values[50]
                  b. Según modo: gráfica ASCII (#PLOT) o estadísticas (#STAT)
```

Cada paso del barrido ejecuta un ciclo completo en el hardware:

```
disable → reconfig_controller(volt_lim) → reconfig_channel(charge_speed, init_charge_volt)
       → enable → oneshot_scan → read raw_value
```

La ventana de muestreo (5ms) se configura una sola vez al inicio y no se toca
durante el barrido, lo que reduce el overhead de reconfiguración.

## Modos de operación

Se controlan por comandos de serie (UART):

| Comando | Modo | Descripción |
|:--------|:-----|:------------|
| `#PLOT` | Gráfica | Muestra una gráfica ASCII de los 50 valores con escala fija (calibrada). También imprime el vector `raw:` en CSV |
| `#STAT` | Estadísticas | Calcula media, desvío estándar, mínimo, máximo y coeficiente de variación (CV%) de los 50 valores del sweep actual. También imprime `raw:` en CSV |

El modo por defecto es `#PLOT`. Para cambiar, escribir el comando por la terminal
serie y presionar Enter.

### Ejemplo de salida `#STAT`

```
--- SWEEP STATS (50 steps) ---
Mean: 4521.3 | StdDev: 187.6 | Min: 4102 | Max: 4890 | CV: 4.15%
raw:4102,4150,4201,...
```

El CV% (coeficiente de variación = stddev/mean × 100) indica cuánto varía la
respuesta entre las 50 configuraciones de excitación. Un CV alto significa que
algunas configuraciones son más sensibles que otras al estado del sensor (toque
vs. sin toque).

## Hardware

| Peripheral      | ESP32 GPIO |
|:----------------|:-----------|
| Touch pad (T8)  | GPIO_33    |

No se requieren componentes externos. El pin GPIO_33 ( TOUCH_PAD_8 ) se puede
conectar a una lámina conductora, antena o directamente usar el pin de la placa.

## Archivos

| Archivo | Descripción |
|:--------|:------------|
| `main/main.c` | Aplicación: sweep, gráfica, estadísticas, parser de comandos |
| `drivers_hal/touch_hal.c` | Driver touch: `TouchHalSweepExcite()` reconfig controller+channel en un solo ciclo |
| `drivers_hal/touch_hal.h` | API del driver touch |

## Construcción y flashear

```bash
idf.py set-target esp32
idf.py build
idf.py -p PORT flash monitor
```

Seguir los pasos detallados en los instructivos (según sea necesario):

1. [Instalación](../../../documentación/instalación.md)
2. [Compilación](../../../documentación/compilación.md)
