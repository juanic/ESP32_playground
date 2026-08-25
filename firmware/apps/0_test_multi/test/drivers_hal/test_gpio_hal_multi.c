#include "unity.h"
#include "unity_test_runner.h"
#include "unity_test_utils.h"
#include "gpio_hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "test_utils.h"

#define GPIO_LINK_PIN GPIO_4

static void role_master(void)
{
    GPIOInit(GPIO_LINK_PIN, GPIO_OUTPUT);
    GPIOOff(GPIO_LINK_PIN);
    vTaskDelay(pdMS_TO_TICKS(20));

    GPIOOn(GPIO_LINK_PIN);
    unity_send_signal("master_high");
    unity_wait_for_signal("slave_checked_high");

    GPIOOff(GPIO_LINK_PIN);
    unity_send_signal("master_low");
    unity_wait_for_signal("slave_checked_low");
}

static void role_slave(void)
{
    GPIOInit(GPIO_LINK_PIN, GPIO_INPUT);

    unity_wait_for_signal("master_high");
    TEST_ASSERT_TRUE(GPIORead(GPIO_LINK_PIN));
    unity_send_signal("slave_checked_high");

    unity_wait_for_signal("master_low");
    TEST_ASSERT_FALSE(GPIORead(GPIO_LINK_PIN));
    unity_send_signal("slave_checked_low");
}

TEST_CASE_MULTIPLE_DEVICES(
    "GPIO HAL multi-device high/low propagation",
    "[drivers_hal][gpio]",
    role_master,
    role_slave
);
