idf_component_register(
    SRCS
        "drivers_hal/test_gpio_hal.c"
        "board_support/test_led.c"
    INCLUDE_DIRS "."
    REQUIRES unity drivers_hal board_support
    WHOLE_ARCHIVE
)
