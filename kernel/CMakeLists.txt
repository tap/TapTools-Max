# Copyright 2002-2026 Timothy Place. Distributed under the New BSD License.
#
# TapTools kernel — the portable DSP library behind the TapTools Max package.
# Header-only, plain C++ (C++17 minimum): no Max SDK, no min-api, no Jamoma. The kernels live
# under include/taptools/, one self-contained header per object, in the `taptools` namespace.
#
# This directory is a complete standalone CMake project, staged here for extraction into its
# own repository (the AmbiTap / AmbiTap-Max pattern). The Max externals at the repo root
# consume only include/ (via TAPTOOLS_KERNEL_DIR in the root CMakeLists.txt); the kernel's own
# tests, tools, and benchmarks build from here:
#
#     cmake -S kernel -B build-kernel -DCMAKE_BUILD_TYPE=Release
#     cmake --build build-kernel
#     ctest --test-dir build-kernel

cmake_minimum_required(VERSION 3.19)
project(TapToolsKernel VERSION 0.1.0 LANGUAGES CXX)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(TAPTOOLS_KERNEL_TOP_LEVEL ON)
else ()
    set(TAPTOOLS_KERNEL_TOP_LEVEL OFF)
endif ()

option(TAPTOOLS_BUILD_TESTS "Build the kernel unit tests" ${TAPTOOLS_KERNEL_TOP_LEVEL})
option(TAPTOOLS_BUILD_TOOLS "Build the offline render tools and the C ABI" ${TAPTOOLS_KERNEL_TOP_LEVEL})
option(TAPTOOLS_BUILD_BENCH "Build the CPU benchmarks" ${TAPTOOLS_KERNEL_TOP_LEVEL})
option(TAPTOOLS_INSTALL "Generate install and package-export rules" ${TAPTOOLS_KERNEL_TOP_LEVEL})

add_library(taptools INTERFACE)
add_library(TapTools::taptools ALIAS taptools)
target_compile_features(taptools INTERFACE cxx_std_17)
target_include_directories(taptools INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

if (TAPTOOLS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (TAPTOOLS_BUILD_TOOLS)
    add_subdirectory(tools/render)
    add_subdirectory(tools/capi)
endif ()

if (TAPTOOLS_BUILD_BENCH)
    add_subdirectory(bench)
endif ()

if (TAPTOOLS_INSTALL)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(TARGETS taptools EXPORT TapToolsKernelTargets)
    install(EXPORT TapToolsKernelTargets
        NAMESPACE TapTools::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TapToolsKernel)

    configure_package_config_file(cmake/TapToolsKernelConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/TapToolsKernelConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TapToolsKernel)
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/TapToolsKernelConfigVersion.cmake
        COMPATIBILITY SameMajorVersion
        ARCH_INDEPENDENT)
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/TapToolsKernelConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/TapToolsKernelConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TapToolsKernel)
endif ()
