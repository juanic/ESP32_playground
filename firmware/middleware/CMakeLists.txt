idf_build_get_property(target IDF_TARGET)

# Always compiled source files
set(srcs "src/animation.c"
         "src/bt_access_control.c"
         "src/inter_board_link.c"
         "src/relay_control.c"
    )

# Optional HTTP MJPEG camera server.
if(CONFIG_CAMERA_HAL_ENABLED)
    list(APPEND srcs "src/camera_stream_server.c")
endif()

# Optional people-counter stack: SD repository, external detector client and
# the monitoring web UI. It requires the SD HAL.
if(CONFIG_SD_HAL_ENABLED)
    list(APPEND srcs "src/photo_repository.c"
                     "src/detector_client.c"
                     "src/monitor_web_server.c")
endif()

# Always included headers
set(includes "inc")

set(deps board_support drivers_hal esp_timer esp_http_server)

# As in the HAL components, these requirements are also resolved during the
# preliminary expansion pass, where CONFIG_* values are not available yet.
if(CONFIG_SD_HAL_ENABLED OR CMAKE_BUILD_EARLY_EXPANSION)
    list(APPEND deps esp_http_client fatfs vfs)
endif()

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${includes}
                       PRIV_REQUIRES ${deps})
