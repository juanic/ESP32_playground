#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "esp_err.h"
#include "esp_task_wdt.h"
#include "unity_test_runner.h"

void app_main(void)
{ 
    printf("Executing one test by its name");
    UNITY_BEGIN();
    unity_run_test_by_name("GPIOInit configures output pin");
    UNITY_END();

    printf("Running tests with [gpio] tag");
    UNITY_BEGIN();
    unity_run_tests_by_tag("[gpio]", false);
    UNITY_END();

    printf("Running tests without [led] tag");
    UNITY_BEGIN();
    unity_run_tests_by_tag("[led]", true);
    UNITY_END();

    printf("Running all the registered tests");
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();

    printf("Starting interactive test menu");
    /*When using the Unity unit testing framework in the ESP-IDF 
    on the ESP32, unity_run_menu() blocks execution indefinitely 
    while waiting for UART user input. Because no other tasks 
    execute or yield to feed the Task Watchdog Timer (TWDT), 
    the system will trigger a watchdog timeout and reset.*/
    ESP_ERROR_CHECK(esp_task_wdt_deinit()); // Disables the task watchdog entirely

    printf("\n[0_test_runner] Unity menu ready.\n");
    printf("[0_test_runner] Enter '*' to run all tests or [tag] to filter.\n\n");
    unity_run_menu();
}
