idf_build_get_property(target IDF_TARGET)

# Always compiled source files
set(srcs "src/gpio_hal.c"
          "src/touch_hal.c"
    )

# Always included headers
set(includes "inc")

# The HID stack components (esp_hid + bt for BLE) are required unconditionally so
# their Kconfig symbols (CONFIG_BT_ENABLED, ...) are always present in the build.
# Without this, sdkconfig.defaults lines referencing those symbols are silently
# ignored by conf and the Bluetooth menus do not appear in menuconfig.
set(priv_reqs esp_driver_gpio esp_driver_touch_sens esp_hid bt)

# HID transport selection (see drivers_hal/Kconfig): only decides which driver
# source file is compiled. esp_tinyusb + tinyusb are vendored as LOCAL components
# under apps/3_usb_hid/components/ (no idf_component.yml), so they are pulled in
# via PRIV_REQUIRES only for the USB/BOTH transports.
if(CONFIG_HID_TRANSPORT_BLE OR CONFIG_HID_TRANSPORT_BOTH)
    list(APPEND srcs "src/ble_hid_kb.c")
endif()

if(CONFIG_HID_TRANSPORT_USB OR CONFIG_HID_TRANSPORT_BOTH)
    list(APPEND srcs "src/usb_hid_kb.c")
    list(APPEND priv_reqs esp_tinyusb tinyusb)
endif()

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${includes}
                       PRIV_REQUIRES ${priv_reqs})

# NOTE: For the USB transport, esp_tinyusb + tinyusb are local components under
# apps/3_usb_hid/components/. PRIV_REQUIRES is needed for LINKING, but the
# include-dir + compile-definition propagation to this EXTRA component does not
# happen automatically in this setup, so we add them explicitly. (Guarded so the
# BLE app, which has no such components, is unaffected.)
if(CONFIG_HID_TRANSPORT_USB OR CONFIG_HID_TRANSPORT_BOTH)
    idf_component_get_property(et_dir esp_tinyusb COMPONENT_DIR)
    idf_component_get_property(tb_dir tinyusb COMPONENT_DIR)
    target_include_directories(${COMPONENT_TARGET} PRIVATE
        "${et_dir}/include"
        "${tb_dir}/src")
    # CFG_TUSB_MCU / CFG_TUSB_OS_INC_PATH are normally injected by the tinyusb
    # target's PUBLIC compile options, but those don't reach this EXTRA component.
    target_compile_definitions(${COMPONENT_TARGET} PRIVATE
        CFG_TUSB_MCU=OPT_MCU_ESP32S3
        CFG_TUSB_OS_INC_PATH=freertos/)
    # PRIV_REQUIRES pulls them in for the build, but the link edge from this EXTRA
    # component is not established automatically, so link the libraries explicitly.
    target_link_libraries(${COMPONENT_TARGET} PUBLIC
        idf::esp_tinyusb
        idf::tinyusb)
endif()
