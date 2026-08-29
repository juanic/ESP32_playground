#include <stdio.h>
#include "esp_err.h"
#include "esp_task_wdt.h"
#include "unity_test_runner.h"

void app_main(void)
{
    /* unity_run_menu waits for UART input, disable TWDT to avoid resets. */
    ESP_ERROR_CHECK(esp_task_wdt_deinit());

    printf("\n[0_test_multi] Unity menu ready.\n");
    printf("[0_test_multi] Manual sequence with 2 terminals:\n");
    printf("[0_test_multi] 1) In BOTH DUTs select test: 1\n");
    printf("[0_test_multi] 2) Role selection: DUT A -> 1 (master), DUT B -> 2 (slave)\n");
    printf("[0_test_multi] 3) Follow send/wait prompts and press ENTER on the waiting DUT\n\n");
    unity_run_menu();
}
