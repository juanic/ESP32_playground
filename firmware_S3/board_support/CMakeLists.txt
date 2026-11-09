idf_build_get_property(target IDF_TARGET)

# Always compiled source files
set(srcs "src/led.c"
    )

# Always included headers
set(includes "inc")

# board_support usa drivers_hal (regla del repo: board_support -> drivers_hal)
idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${includes}
                       PRIV_REQUIRES drivers_hal)