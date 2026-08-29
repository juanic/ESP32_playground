#include "unity.h"
#include "gpio_hal.h"

TEST_CASE("GPIOInit configures output pin without error", "[drivers_hal][gpio]")
{
    GPIOInit(GPIO_4, GPIO_OUTPUT);
    TEST_ASSERT_TRUE(true);
}


TEST_CASE("GPIOOn and GPIORead return high", "[drivers_hal][gpio]")
{
    GPIOInit(GPIO_4, GPIO_OUTPUT);
    GPIOOn(GPIO_4);
    TEST_ASSERT_TRUE(GPIORead(GPIO_4));
}

TEST_CASE("GPIOOff and GPIORead return low", "[drivers_hal][gpio]")
{
    GPIOInit(GPIO_4, GPIO_OUTPUT);
    GPIOOff(GPIO_4);
    TEST_ASSERT_FALSE(GPIORead(GPIO_4));
}

TEST_CASE("GPIOState drives high and low", "[drivers_hal][gpio]")
{
    GPIOInit(GPIO_4, GPIO_OUTPUT);

    GPIOState(GPIO_4, true);
    TEST_ASSERT_TRUE(GPIORead(GPIO_4));

    GPIOState(GPIO_4, false);
    TEST_ASSERT_FALSE(GPIORead(GPIO_4));
}

TEST_CASE("GPIOToggle inverts current state", "[drivers_hal][gpio]")
{
    GPIOInit(GPIO_4, GPIO_OUTPUT);
    GPIOOff(GPIO_4);
    TEST_ASSERT_FALSE(GPIORead(GPIO_4));

    GPIOToggle(GPIO_4);
    TEST_ASSERT_TRUE(GPIORead(GPIO_4));

    GPIOToggle(GPIO_4);
    TEST_ASSERT_FALSE(GPIORead(GPIO_4));
}

TEST_CASE("GPIOInit and GPIORead do not crash on valid pins", "[drivers_hal][gpio]")
{
    GPIOInit(GPIO_19, GPIO_INPUT);
    (void)GPIORead(GPIO_19);
    // Si llega aqui, no hubo hard fault ni watchdog reset.
    TEST_ASSERT_TRUE(true);
}
