idf_build_get_property(target IDF_TARGET)

# Always compiled source files
set(srcs "src/led.c"
         "src/audio_relay_sink_bsp.c"
         "src/audio_relay_source_bsp.c"
    )

# Always included headers
set(includes "inc")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${includes}
                       PRIV_REQUIRES drivers_hal)