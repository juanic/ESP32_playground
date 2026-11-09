# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

list(APPEND EXTRA_COMPONENT_DIRS "../../middleware")
list(APPEND EXTRA_COMPONENT_DIRS "../../board_support")
# La app usa board_support y middleware; drivers_hal se usa
# implícitamente cuando la FSM necesita modo PENDRIVE (usando TinyUSB/MSC).
# No hacer set(COMPONENTS ...) para que los componentes locales (esp_tinyusb,
# tinyusb, esp_vfs_fat) queden visibles y no fallen el build.

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(usb_msc_laser_bridge)