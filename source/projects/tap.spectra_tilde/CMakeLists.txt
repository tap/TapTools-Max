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
    ${TAPTOOLS_KERNEL_DIR}/include/taptools/spectra.h
)

add_library(
    ${PROJECT_NAME}
    MODULE
    ${SOURCES}
)

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)

# Catch-based unit tests (built when the _test.cpp is present; run via ctest).
include(${C74_MIN_API_DIR}/test/min-object-unittest.cmake)

# The spectral kernel pulls its FFT from the shared DspTap library (tap::dsp): link the Ooura static
# lib into the external, and into the min-test target when present. (These need C++20 for DspTap's
# fft.h; the root sets C++20 on every object + test target centrally.)
target_link_libraries(${PROJECT_NAME} PRIVATE tap::dsp)
if (TARGET ${PROJECT_NAME}_test)
    target_link_libraries(${PROJECT_NAME}_test PUBLIC tap::dsp)
endif ()
