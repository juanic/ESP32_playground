# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

list(APPEND EXTRA_COMPONENT_DIRS "../../middleware")
list(APPEND EXTRA_COMPONENT_DIRS "../../board_support")
list(APPEND EXTRA_COMPONENT_DIRS "../../drivers_hal")
list(APPEND EXTRA_COMPONENT_DIRS "test")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(0_test_runner)
