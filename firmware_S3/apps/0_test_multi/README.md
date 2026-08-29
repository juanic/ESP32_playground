# 0_test_multi

Proyecto de pruebas multi-device para validar `drivers_hal/gpio_hal` con dos ESP32 usando la estrategia de Unity en ESP-IDF (`TEST_CASE_MULTIPLE_DEVICES`).

## Objetivo

Verificar propagacion de nivel logico entre dos DUT:

- DUT A (master): configura GPIO como salida y conmuta HIGH/LOW.
- DUT B (slave): configura GPIO como entrada y valida `GPIORead()` en cada transicion.

## Cableado (2 ESP32)

- Conectar `GND` de ambas placas.
- Conectar `GPIO4` de DUT A <-> `GPIO4` de DUT B.

> Si quieres usar otro pin, cambia `GPIO_LINK_PIN` en `test/drivers_hal/test_gpio_hal_multi.c` en ambos roles.

## Estructura

- `main/test_runner.c`: runner Unity interactivo.
- `test/drivers_hal/test_gpio_hal_multi.c`: caso multi-device.
- `pytest_gpio_multi.py`: automatizacion host-side con pytest-embedded.

## Build y prueba manual por terminales

Abrir dos terminales (una por cada placa) y usar la misma app en ambos DUT.

1. Compilar:

```bash
idf.py set-target esp32 build
```

2. Flashear DUT A:

```bash
idf.py -p COM_A flash monitor
```

3. Flashear DUT B:

```bash
idf.py -p COM_B flash monitor
```

4. En ambos monitores, cuando aparezca el menu, elegir `1` (mismo test en ambos DUT):

- `"GPIO HAL multi-device high/low propagation"`

5. Luego, en el submenu de roles, elegir distinto en cada DUT:

- DUT A: `1` (`role_master`)
- DUT B: `2` (`role_slave`)

6. Seguir prompts de Unity (`send/wait for signal`) hasta obtener `PASS`.

## Automatizacion con pytest-embedded

Instalar dependencias en el entorno ESP-IDF:

```bash
pip install -U pytest pytest-embedded pytest-embedded-idf
```

Ejecutar:

```bash
pytest -s -v pytest_gpio_multi.py \
  --embedded-services esp,idf \
  --count 2 \
  --target esp32|esp32 \
  --port COM_A|COM_B
```

Tambien puedes ejecutar el script directo (usa defaults para 2 DUT):

```bash
python pytest_gpio_multi.py
```

Opcionalmente, puedes pasar puertos/targets sin escribir toda la linea de pytest:

```bash
python pytest_gpio_multi.py --port-a COM6 --port-b COM7 --target-a esp32 --target-b esp32
```

Notas:

- El script usa `unity_tester.run_all_multi_dev_cases()`.
- Si ya flasheaste manualmente y solo quieres correr sin reflash, agrega:

```bash
--skip-autoflash y
```
