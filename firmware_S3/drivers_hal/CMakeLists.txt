idf_build_get_property(target IDF_TARGET)

# Always compiled source files
set(srcs "src/gpio_hal.c"
          "src/touch_hal.c"
    )

# Always included headers
set(includes "inc")

# Base private requirements: GPIO + touch sensor drivers (always used by the HAL).
set(priv_reqs esp_driver_gpio esp_driver_touch_sens)

# HID transport selection (see drivers_hal/Kconfig). Only the selected transport's
# source file is compiled and only its stack is required, so apps that do not use
# HID at all (e.g. the touch / LED examples) do not pull in Bluetooth or TinyUSB.
if(CONFIG_HID_TRANSPORT_BLE OR CONFIG_HID_TRANSPORT_BOTH)
    list(APPEND srcs "src/ble_hid_kb.c")
    list(APPEND priv_reqs bt esp_hid)
endif()

if(CONFIG_HID_TRANSPORT_USB OR CONFIG_HID_TRANSPORT_BOTH)
    list(APPEND srcs "src/usb_hid_kb.c")
    list(APPEND priv_reqs esp_tinyusb tinyusb)
endif()

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS ${includes}
                       PRIV_REQUIRES ${priv_reqs})

# NOTE: For the USB transport, esp_tinyusb + tinyusb are local components under
# apps/<usb-app>/components/. PRIV_REQUIRES links them, but the include-dir +
# compile-definition + link-edge propagation to this EXTRA component does not
# happen automatically in this setup, so we add them explicitly (USB/BOTH only).
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
