# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
if(EXISTS "C:/Users/magla/Documents/MiniCO2/esp-idf-lib/components")
    message(STATUS "esp-idf-lib/components EXISTS")
else()
    message(FATAL_ERROR "esp-idf-lib/components DOES NOT EXIST")
endif()
set(EXTRA_COMPONENT_DIRS "C:\\Users\\magla\\Documents\\MiniCO2\\esp-idf-lib\\components")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(scd4x)
set(COMPONENTS ["scd4x", "led_strip"])
