idf_component_register(
    SRCS
        "drivers_hal/test_gpio_hal_multi.c"
    INCLUDE_DIRS "."
    REQUIRES unity drivers_hal test_utils
    WHOLE_ARCHIVE
)
