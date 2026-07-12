# Standalone C ABI over the portable TapTools DSP cores (conv_engine, ...), used by the
# verification notebooks in ../../notebooks via ctypes. It depends only on the header-only DSP —
# no min-api / Max SDK — so it builds on its own, independent of the Max-external CMake at the repo
# root:
#
#     cmake -B build -S tools/capi
#     cmake --build build
#
# Copyright 1999-2026 Timothy Place. Distributed under the New BSD License.

cmake_minimum_required(VERSION 3.19)
project(taptools_capi CXX)

add_library(taptools_capi SHARED taptools_capi.cpp)

set_target_properties(taptools_capi PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME taptools_capi
    CXX_VISIBILITY_PRESET hidden)
