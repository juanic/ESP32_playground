# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED IDF_TARGET OR "${IDF_TARGET}" STREQUAL "")
	set(IDF_TARGET "esp32" CACHE STRING "ESP-IDF target" FORCE)
endif()

list(APPEND EXTRA_COMPONENT_DIRS "../../middleware")
list(APPEND EXTRA_COMPONENT_DIRS "../../board_support")
list(APPEND EXTRA_COMPONENT_DIRS "../../drivers_hal")
list(APPEND EXTRA_COMPONENT_DIRS "test")
list(APPEND EXTRA_COMPONENT_DIRS "$ENV{IDF_PATH}/tools/test_apps/components")

include_directories(
	"$ENV{IDF_PATH}/components/idf_test/include"
	"$ENV{IDF_PATH}/components/idf_test/include/${IDF_TARGET}"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(0_test_multi)
