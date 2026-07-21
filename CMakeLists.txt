# Copyright 2002-2026 Timothy Place. Distributed under the New BSD License.
#
# Modern CMake build for TapTools, based on the Min-DevKit (min-api + max-sdk-base).
# This replaces the retired Ruby/Xcode-project + Travis build. Objects live in
# source/projects/<name>/ and are built as Max externals (.mxo / .mxe64).

cmake_minimum_required(VERSION 3.19)
project(TapTools)

enable_testing()

string(REGEX REPLACE "(.*)/" "" THIS_PACKAGE_NAME "${CMAKE_CURRENT_SOURCE_DIR}")

if (APPLE)
    # Build universal binaries (Apple Silicon + Intel) by default. The existing 2015-era
    # binaries were i386/x86_64 only and will not run on Apple Silicon; never repeat that.
    if (NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures" FORCE)
        message(STATUS "CMAKE_OSX_ARCHITECTURES set to ${CMAKE_OSX_ARCHITECTURES}")
    endif ()

    # Minimum macOS is 11.0, tracking Max 9's system requirement (and required by C++ features
    # such as std::filesystem). Min's min-pretarget.cmake force-pins CMAKE_OSX_DEPLOYMENT_TARGET
    # to 10.11 in every object, and CMake has no per-target deployment property, so we can't set
    # it through that variable. Append the deployment flag globally instead — it propagates to
    # every target added below, and the later flag wins on the compile/link command line.
    add_compile_options(-mmacosx-version-min=11.0)
    add_link_options(-mmacosx-version-min=11.0)
endif ()

# The portable DSP kernel (header-only, no Max dependency) lives in its own repository — the
# TapTools library — pinned here as the submodules/taptools submodule (the AmbiTap / AmbiTap-Max
# pattern). The externals consume only its headers. Override with -DTAPTOOLS_KERNEL_DIR=/path/to
# to build against a sibling checkout of the kernel instead of the pinned submodule.
set(TAPTOOLS_KERNEL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/submodules/taptools" CACHE PATH
    "Path to the TapTools DSP kernel (defaults to the pinned submodule; override for a dev checkout)")
if (NOT EXISTS "${TAPTOOLS_KERNEL_DIR}/include/taptools/taptools.h")
    message(FATAL_ERROR "TapTools kernel not found at ${TAPTOOLS_KERNEL_DIR}. "
                        "Run: git submodule update --init --recursive (or set -DTAPTOOLS_KERNEL_DIR).")
endif ()

# The spectral externals (tap.convolve~ / tap.nr~ / tap.spectra~) pull the kernel's shared real FFT
# (tap::dsp) from the kernel's own dsptap submodule — the Ooura transform plus its vDSP / CMSIS
# backends. Expose the target once here so those objects can link the static lib; the rest of the
# objects are header-only and never touch it. (Nested submodule — hence recursive init.)
if (NOT TARGET tap::dsp AND EXISTS "${TAPTOOLS_KERNEL_DIR}/submodules/dsptap/CMakeLists.txt")
    add_subdirectory(${TAPTOOLS_KERNEL_DIR}/submodules/dsptap ${CMAKE_CURRENT_BINARY_DIR}/dsptap)
endif ()

# Min-DevKit package helpers (defines SUBDIRLIST, packaging, etc.)
include(${CMAKE_CURRENT_SOURCE_DIR}/source/min-api/script/min-package.cmake)

# Add Min-Lib if present (used by DSP objects as the revival progresses).
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/min-lib/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/min-lib)
endif ()

# Generate a build target for every object folder in source/projects.
SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/projects)
foreach (project_dir ${PROJECT_DIRS})
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir}/CMakeLists.txt")
        message(STATUS "Generating: ${project_dir}")
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir})
    endif ()
endforeach ()
