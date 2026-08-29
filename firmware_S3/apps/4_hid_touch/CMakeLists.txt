# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

list(APPEND EXTRA_COMPONENT_DIRS "../../middleware")
list(APPEND EXTRA_COMPONENT_DIRS "../../board_support")
list(APPEND EXTRA_COMPONENT_DIRS "../../drivers_hal")

include_directories(${PROJECT_NAME} ../../middleware)
include_directories(${PROJECT_NAME} ../../board_support)
include_directories(${PROJECT_NAME} ../../drivers_hal)

# NOTE: Do NOT restrict COMPONENTS here. drivers_hal PRIV_REQUIRES esp_tinyusb
# (and its tinyusb stack) for the USB HID transport; an explicit COMPONENTS list
# would exclude those local components (vendored under components/) and break the
# build. The local esp_tinyusb/tinyusb copies are discovered automatically from
# the project's components/ directory.

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(4_hid_touch)
