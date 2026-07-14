/// @file taptools_capi.cpp
/// @brief C ABI implementation — see taptools_capi.h.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "taptools_capi.h"

// The DSP core is the same header the tap.convolve~ external compiles — no Max/Min dependency.
#include <taptools/conv_engine.h>

using taptools::conv_engine;

extern "C" {

taptools_conv taptools_conv_create(void) {
    return static_cast<taptools_conv>(new conv_engine());
}

void taptools_conv_destroy(taptools_conv engine) {
    delete static_cast<conv_engine*>(engine);
}

int taptools_conv_configure(taptools_conv engine, int blocksize, int max_partitions) {
    if (!engine)
        return -1;
    static_cast<conv_engine*>(engine)->configure(blocksize, max_partitions);
    return 0;
}

int taptools_conv_load_ir(taptools_conv engine, const float* ll, const float* lr, const float* rl, const float* rr,
                          int length, double scale) {
    if (!engine)
        return -1;
    const float* paths[conv_engine::k_paths] = {ll, lr, rl, rr};
    static_cast<conv_engine*>(engine)->load_ir(paths, length, scale);
    return 0;
}

int taptools_conv_clear(taptools_conv engine) {
    if (!engine)
        return -1;
    static_cast<conv_engine*>(engine)->clear();
    return 0;
}

int taptools_conv_process(taptools_conv engine, const double* inL, const double* inR, double* outL, double* outR,
                          int n) {
    if (!engine)
        return -1;
    static_cast<conv_engine*>(engine)->process(inL, inR, outL, outR, static_cast<long>(n));
    return 0;
}

int taptools_conv_block_size(taptools_conv engine) {
    return engine ? static_cast<conv_engine*>(engine)->block_size() : -1;
}

int taptools_conv_max_partitions(taptools_conv engine) {
    return engine ? static_cast<conv_engine*>(engine)->max_partitions() : -1;
}

int taptools_conv_has_ir(taptools_conv engine) {
    return (engine && static_cast<conv_engine*>(engine)->has_ir()) ? 1 : 0;
}

} // extern "C"
