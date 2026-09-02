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

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${includes}
                       REQUIRES ${deps}
                       PRIV_REQUIRES esp_driver_gpio esp_driver_ledc freertos)
