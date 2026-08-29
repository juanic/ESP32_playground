import pytest
import argparse
import sys


# El marcador @pytest.mark.multi_dut_generic es reconocido por pytest-embedded
# para identificar tests que requieren múltiples dispositivos físicos conectados.
# pytest-embedded inyecta automáticamente el fixture 'unity_tester' cuando
# el test está marcado correctamente y se pasan --count 2 y --port A|B.
@pytest.mark.multi_dut_generic
def test_gpio_multi_device(unity_tester) -> None:
    """Runs all Unity multi-device GPIO cases in this app."""

    # run_all_multi_dev_cases() es un método de pytest-embedded que:
    # 1. Busca todos los TEST_CASE_MULTIPLE_DEVICES registrados en el firmware.
    # 2. Por cada caso, selecciona el rol correcto en cada DUT (role_master en DUT0,
    #    role_slave en DUT1).
    # 3. Reemplaza la interacción manual por UART (los "Press Enter") con
    #    comunicación automática entre los dos puertos serie.
    # 4. Recolecta los resultados PASS/FAIL de ambos dispositivos y los reporta
    #    como un único resultado en pytest.
    unity_tester.run_all_multi_dev_cases()


def main() -> int:
    # Parser de argumentos para poder invocar el script desde consola
    # sin tener que recordar la sintaxis completa de pytest-embedded.
    # Ejemplo de uso:
    #   python test_multi.py --port-a COM6 --port-b COM7
    #   python test_multi.py --port-a COM6 --port-b COM7 --autoflash
    parser = argparse.ArgumentParser(
        description="Run multi-device GPIO test with pytest-embedded defaults."
    )
    parser.add_argument("--port-a", default="COM6", help="Port for DUT A")
    parser.add_argument("--port-b", default="COM7", help="Port for DUT B")
    parser.add_argument("--target-a", default="esp32", help="Target for DUT A")
    parser.add_argument("--target-b", default="esp32", help="Target for DUT B")

    # --autoflash es un flag booleano: si se pasa, flashea el firmware antes
    # de correr los tests. Si no se pasa (default), asume que el firmware ya
    # está en el dispositivo y omite el flash para ir más rápido.
    parser.add_argument(
        "--autoflash",
        action="store_true",
        help="Enable autoflash (default is skip autoflash)",
    )

    args = parser.parse_args()

    # pytest-embedded usa "y"/"n" como string para --skip-autoflash,
    # no True/False. Se invierte la lógica: si el usuario pidió autoflash,
    # skip = "n" (no saltear); si no lo pidió, skip = "y" (saltear el flash).
    skip_autoflash_value = "n" if args.autoflash else "y"

    # Se construye la lista de argumentos que se pasarían a pytest desde la
    # línea de comandos, y se invoca pytest.main() programáticamente.
    # Esto permite correr el script con 'python test_multi.py' en lugar de
    # tener que escribir el comando completo de pytest cada vez.
    pytest_args = [
        "-s",           # No captura stdout: muestra la salida UART en tiempo real.
        "-v",           # Verbose: muestra el nombre de cada test y su resultado.
        __file__,       # Le dice a pytest que busque tests en este mismo archivo.

        "--embedded-services", "esp,idf",
        # Activa los servicios de pytest-embedded:
        # - 'esp': maneja la conexión serie con dispositivos ESP (escaneo de puertos,
        #          reset, lectura de UART).
        # - 'idf': integra con ESP-IDF para interpretar el output de Unity y manejar
        #          el protocolo de señales multi-device automáticamente.

        "--count", "2",
        # Indica que el test requiere exactamente 2 DUTs simultáneos.
        # pytest-embedded instancia dos objetos DUT y los asigna a dut[0] y dut[1],
        # que internamente corresponden a role_master y role_slave respectivamente.

        "--target", f"{args.target_a}|{args.target_b}",
        # Target de cada DUT separado por |. Ambos son esp32 en este caso,
        # pero podrían ser targets distintos si el test lo requiriera.

        "--port", f"{args.port_a}|{args.port_b}",
        # Puerto serie de cada DUT separado por |.
        # DUT0 (master) → port_a, DUT1 (slave) → port_b.

        "--skip-autoflash", skip_autoflash_value,
        # "y": no flashea, asume que el firmware ya está cargado.
        # "n": flashea el binario antes de correr los tests.
    ]

    # pytest.main() retorna 0 si todos los tests pasaron, distinto de 0 si hubo
    # fallos. Se propaga como exit code del proceso para que Jenkins u otro CI
    # pueda interpretar el resultado correctamente.
    return pytest.main(pytest_args)


if __name__ == "__main__":
    # Punto de entrada cuando se corre el script directamente.
    # sys.exit() convierte el código de retorno de pytest.main() en el
    # exit code del proceso, necesario para integración con CI.
    sys.exit(main())
