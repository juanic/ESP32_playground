# Ejemplo touch

Este ejemplo demuestra el uso de la nueva capa `touch_hal` (drivers_hal) para
leer los pines capacitivos (touch) que trae el ESP32 clásico, en conjunto con
la capa de BSP `led` para dar una realimentación visual inmediata: mientras se
toca el pad `TOUCH_PAD_8` (GPIO_33) se enciende el `LED_1`, y mientras se toca
`TOUCH_PAD_9` (GPIO_32) se enciende el `LED_2`. Además, cada 100 ms se imprime
por consola el valor filtrado ("smooth") de ambos canales, lo que sirve para
calibrar el umbral de detección (`TOUCH_THRESHOLD`) según tu cableado.

## Hardware

| Peripheral        | ESP32 GPIO |
|:------------------|:-----------|
| Touch pad (T8)     | GPIO_33    |
| Touch pad (T9)     | GPIO_32    |
| LED_1              | GPIO_4     |
| LED_2              | GPIO_5     |

No hace falta ningún componente externo: alcanza con tocar con un dedo el pin
físico correspondiente en la placa (o soldar un cable/lámina metálica a modo
de antena para mejorar la sensibilidad).

## Calibración del umbral

El valor "smooth" que devuelve `TouchHalRead()` baja cuando el pad es tocado
(mayor capacitancia). Para calibrar:

1. Compilar y flashear el ejemplo dejando el valor por defecto de
   `TOUCH_THRESHOLD` (500) en [main/1_touch.c](main/1_touch.c).
2. Abrir el monitor serie y anotar el valor típico *sin* tocar el pad.
3. Ajustar `TOUCH_THRESHOLD` a, aproximadamente, 2/3 de ese valor sin tocar.
4. Volver a compilar y flashear, y verificar que el LED correspondiente
   reacciona correctamente al tocar/soltar el pad.

## Cómo usar el ejemplo

Este proyecto todavía no fue compilado en este entorno (el target se dejó
preconfigurado en `esp32` en el archivo `sdkconfig`, pero hace falta correr
`idf.py set-target esp32` o `idf.py build` para regenerar la configuración
completa la primera vez).

Seguir los pasos detallados en los siguientes instructivos (según sea
necesario):

1. [Instalación](../../../documentación/instalación.md)
2. [Compilación](../../../documentación/compilación.md)
