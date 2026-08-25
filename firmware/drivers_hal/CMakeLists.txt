idf_build_get_property(target IDF_TARGET)

# Always compiled source files
set(srcs "src/gpio_hal.c"
         "src/touch_hal.c"
    )

# Always included headers
set(includes "inc")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${includes}
                       PRIV_REQUIRES esp_driver_gpio esp_driver_touch_sens)