# Capa de Middleware

Esta capa contiene software lógico que depende de los controladores de hardware subyacentes (BSP/HAL) pero no contiene código específico de la aplicación final. Ofrece servicios generales que facilitan el desarrollo de la lógica de negocio.

En este ejemplo, el middleware expone un **controlador de animaciones de LEDs**.

---

## Módulos Disponibles

### Controlador de Animaciones (`animation`)
Este módulo organiza secuencias de encendido y efectos visuales sobre los LEDs de la placa de desarrollo sin bloquear la ejecución de la aplicación.

* **Cabecera:** [animation.h](./inc/animation.h)

### Acceso por MAC (`bt_access_control`)
Gestiona listas blancas/negras de direcciones Bluetooth (MAC) persistiéndolas en NVS. Lógica pura de negocio, sin dependencia de hardware más allá del almacenamiento.

* **Cabecera:** [bt_access_control.h](./inc/bt_access_control.h)

### Control del Relay (`relay_control`)
Plano de control del audio relay (sink): autenticación por contraseña, habilitación/deshabilitación de la transmisión de audio, desconexión del peer A2DP, filtrado de MACs, consulta de metadata y reenvío de eventos a una App vía BLE.

* **Cabecera:** [relay_control.h](./inc/relay_control.h)

---

## Modos de Animación (`animation_t`)

El middleware implementa tres patrones de encendido:

| Animación | Descripción | Comportamiento |
| :--- | :--- | :--- |
| **`ANIMATION_BLINK_ALL`** | Destello general | Invierte simultáneamente el estado de todos los LEDs (`LED_1`, `LED_2` y `LED_3`). |
| **`ANIMATION_CHASE`** | Secuencia simple | Enciende un LED a la vez, desplazando la luz en un único sentido: `L1` $\rightarrow$ `L2` $\rightarrow$ `L3` $\rightarrow$ `L1` ... |
| **`ANIMATION_PING_PONG`** | Ida y vuelta | Desplaza la luz de izquierda a derecha y luego de regreso: `L1` $\rightarrow$ `L2` $\rightarrow$ `L3` $\rightarrow$ `L2` $\rightarrow$ `L1` ... |

---

## Interfaz de Programación (API)

* **`uint8_t AnimationInit(void)`**: Inicializa el middleware de animación y configura los LEDs subyacentes inicializando el BSP.
* **`void AnimationRunStep(animation_t anim)`**: Ejecuta un paso elemental de la animación seleccionada. Esta función se debe llamar periódicamente a la tasa en que se desee actualizar el efecto visual.
