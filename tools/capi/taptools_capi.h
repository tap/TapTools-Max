/// @file taptools_capi.h
/// @brief Minimal C ABI over the portable TapTools DSP cores, for language bindings and the
///        verification notebooks (notebooks/ drive it via ctypes).
///
///        Conventions: plain C types only; the caller owns all arrays and sizes them. Handle-based
///        functions return 0 on success and -1 on any error (bad argument, unconfigured engine).
///        No global state. Currently exposes tap.convolve~'s conv_engine (uniformly-partitioned
///        overlap-save convolution).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#define TAPTOOLS_API __declspec(dllexport)
#else
#define TAPTOOLS_API __attribute__((visibility("default")))
#endif

typedef void* taptools_conv;

/// Create / destroy a convolution engine instance.
TAPTOOLS_API taptools_conv taptools_conv_create(void);
TAPTOOLS_API void          taptools_conv_destroy(taptools_conv engine);

/// Allocate the engine for a partition size (samples, power of two) and a maximum partition count.
/// Discards any loaded IR and clears all state. Returns 0, or -1 on a bad handle.
TAPTOOLS_API int taptools_conv_configure(taptools_conv engine, int blocksize, int max_partitions);

/// Load the four true-stereo IR paths (LL, LR, RL, RR) of `length` samples into the engine and
/// publish them atomically. Any of the four pointers may be NULL for a silent path; `scale` is
/// applied to every sample. Returns 0, or -1 on a bad/unconfigured handle.
TAPTOOLS_API int taptools_conv_load_ir(taptools_conv engine, const float* ll, const float* lr, const float* rl,
                                       const float* rr, int length, double scale);

/// Flush the running state (input history + pending output); keeps the loaded IR. Returns 0/-1.
TAPTOOLS_API int taptools_conv_clear(taptools_conv engine);

/// Process n stereo samples. Wet (fully convolved) output is written to outL/outR (double).
/// Input and output buffers must not alias. Returns 0, or -1 on a bad handle.
TAPTOOLS_API int taptools_conv_process(taptools_conv engine, const double* inL, const double* inR, double* outL,
                                       double* outR, int n);

/// Introspection.
TAPTOOLS_API int taptools_conv_block_size(taptools_conv engine);     // partition size (= latency)
TAPTOOLS_API int taptools_conv_max_partitions(taptools_conv engine); // capacity in partitions
TAPTOOLS_API int taptools_conv_has_ir(taptools_conv engine);         // 1 if an IR is published, else 0

#ifdef __cplusplus
}
#endif
