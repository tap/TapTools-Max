# Copyright 2026 Timothy Place. Distributed under the New BSD License.

cmake_minimum_required(VERSION 3.19)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)

#############################################################
# MAX EXTERNAL
#############################################################

include_directories("${C74_INCLUDES}")
# The portable DSP kernel (header-only), pinned as the submodules/taptools submodule.
include_directories("${TAPTOOLS_KERNEL_DIR}/include")

set(SOURCES
    ${PROJECT_NAME}.cpp
    ${TAPTOOLS_KERNEL_DIR}/include/taptools/ladder.h
)

add_library(
    ${PROJECT_NAME}
    MODULE
    ${SOURCES}
)

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)

# Catch-based unit tests (built when the _test.cpp is present; run via ctest).
include(${C74_MIN_API_DIR}/test/min-object-unittest.cmake)

