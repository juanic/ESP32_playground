idf_build_get_property(target IDF_TARGET)

# Always compiled source files
set(srcs "src/gpio_hal.c"
         "src/touch_hal.c"
         "src/pwm_hal.c"
    )

# BLE HID driver — always listed, guarded by #if CONFIG_BT_ENABLED inside
list(APPEND srcs "src/ble_hid_kb.c")

# Audio relay drivers
list(APPEND srcs "src/i2s_hal.c"
                 "src/bt_classic_hal.c"
                 "src/nvs_hal.c"
                 "src/avrcp_hal.c"
                 "src/ble_control_hal.c"
                 "src/uart_link_hal.c"
                 "src/time_hal.c")

# Always included headers
set(includes "inc")

# REQUIRES: bt/bluedroid only needed when BT is enabled.
# Since ble_hid_kb.c guards itself with #if CONFIG_BT_ENABLED,
# the symbols are weak/empty when BT is off. Add deps always
# so cmake resolves them; when BT is off they just won't link.
set(deps esp_driver_touch_sens esp_driver_i2s esp_driver_uart bt nvs_flash)

# Optional camera HAL. The esp32-camera component is provided by the
# application's idf_component.yml, so applications that do not use the
# camera are not forced to download it. Managed components use the
# "<namespace>__<name>" naming convention in IDF.
#
# IMPORTANT: ESP-IDF expands component requirements in a preliminary pass
# where CONFIG_* variables are not defined yet. Therefore the dependency is
# detected by checking whether the component manager already provides the
# esp32-camera component (declared in the app's idf_component.yml), and
# CONFIG_CAMERA_HAL_ENABLED is only honoured in the final registration pass.
idf_build_get_property(_component_targets __COMPONENT_TARGETS)
set(_esp32_camera_available FALSE)
foreach(_target ${_component_targets})
    __component_get_property(_component_name ${_target} COMPONENT_NAME)
    if(_component_name STREQUAL "espressif__esp32-camera")
        set(_esp32_camera_available TRUE)
        break()
    endif()
endforeach()

if(_esp32_camera_available AND (CMAKE_BUILD_EARLY_EXPANSION OR CONFIG_CAMERA_HAL_ENABLED))
    list(APPEND srcs "src/camera_hal.c")
    list(APPEND deps espressif__esp32-camera)
endif()

# Optional Wi-Fi station HAL.
if(CONFIG_WIFI_HAL_ENABLED)
    list(APPEND srcs "src/wifi_hal.c")
    list(APPEND deps esp_wifi esp_netif esp_event esp_timer)
endif()

# Optional SD card HAL (SDMMC host + FATFS through VFS).
#
# As with the camera HAL above, component requirements are resolved during a
# preliminary pass where CONFIG_* values are not available yet, so the
# dependencies are also added during that early expansion.
if(CONFIG_SD_HAL_ENABLED OR CMAKE_BUILD_EARLY_EXPANSION)
    list(APPEND deps esp_driver_sdmmc fatfs vfs)
endif()
if(CONFIG_SD_HAL_ENABLED)
    list(APPEND srcs "src/sd_hal.c")
endif()

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${includes}
                       REQUIRES ${deps}
                       PRIV_REQUIRES esp_driver_gpio esp_driver_ledc freertos)
