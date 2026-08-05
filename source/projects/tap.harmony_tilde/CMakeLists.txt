# Copyright 1999-2026 Timothy Place. Distributed under the New BSD License.

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
    ${TAPTOOLS_KERNEL_DIR}/include/taptools/harmonizer.h
)

add_library(
    ${PROJECT_NAME}
    MODULE
    ${SOURCES}
)

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)

# The harmonizer kernel composes the DspTap phase vocoder (tap::dsp): link it into the external.
# (Needs C++20 for DspTap's headers; the root sets C++20 on every object target centrally.)
target_link_libraries(${PROJECT_NAME} PRIVATE tap::dsp)
