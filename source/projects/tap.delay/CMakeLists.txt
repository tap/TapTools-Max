# Copyright 1999-2026 Timothy Place. Distributed under the New BSD License.

cmake_minimum_required(VERSION 3.19)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)

#############################################################
# MAX EXTERNAL
#############################################################

include_directories("${C74_INCLUDES}")

set(SOURCES
    ${PROJECT_NAME}.cpp
)

add_library(
    ${PROJECT_NAME}
    MODULE
    ${SOURCES}
)

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)

# TapTools targets C++20. Min's posttarget pins the standard to C++17, so override it here
# (this runs after the include above).
# Catch-based unit tests (built when the _test.cpp is present; run via ctest).
include(${C74_MIN_API_DIR}/test/min-object-unittest.cmake)
