
cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS components)

# Conditionally exclude components based on target
# Use IDF_TARGET from environment or sdkconfig
if(DEFINED ENV{IDF_TARGET})
    set(target $ENV{IDF_TARGET})
else()
    # Read from sdkconfig if environment variable not set
    file(STRINGS "${CMAKE_SOURCE_DIR}/sdkconfig" idf_target_line REGEX "^CONFIG_IDF_TARGET=")
    if(idf_target_line)
        string(REGEX REPLACE "CONFIG_IDF_TARGET=\"(.+)\"" "\\1" target "${idf_target_line}")
    endif()
endif()

if(target STREQUAL "esp32c3" OR target STREQUAL "esp32c6")
    # For RISC-V targets, exclude Xtensa libraries
    set(EXCLUDE_COMPONENTS "bmv080"  "bsec" )
elseif(target STREQUAL "esp32s3")
    # For Xtensa targets, exclude RISC-V libraries
    set(EXCLUDE_COMPONENTS "_bmv080" "_bsec")
endif()
message("ROOT CMakeLists: target='${target}', EXCLUDE_COMPONENTS='${EXCLUDE_COMPONENTS}'")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(ISD_2026_METROPOLIA)
