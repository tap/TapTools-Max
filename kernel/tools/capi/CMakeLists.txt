# Standalone C ABI over the portable TapTools DSP kernels (conv_engine, ...), used by the
# verification notebooks in ../../notebooks via ctypes. It depends only on the header-only kernel —
# no min-api / Max SDK — so it builds on its own, independent of the Max-external CMake at the repo
# root (and is also built by the kernel project when TAPTOOLS_BUILD_TOOLS is on):
#
#     cmake -B build_capi -S kernel/tools/capi
#     cmake --build build_capi
#
# Copyright 1999-2026 Timothy Place. Distributed under the New BSD License.

cmake_minimum_required(VERSION 3.19)
project(taptools_capi CXX)

add_library(taptools_capi SHARED taptools_capi.cpp)
target_include_directories(taptools_capi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

set_target_properties(taptools_capi PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME taptools_capi
    CXX_VISIBILITY_PRESET hidden)
