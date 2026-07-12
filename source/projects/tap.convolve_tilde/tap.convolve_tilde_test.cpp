/// @file
/// @brief      Unit tests for tap.convolve~ (the partitioned convolution engine).
/// @details    The DSP lives in the Min-free `conv_engine`, so these tests drive it directly — no
///             buffer~ or live Max needed — and check its output against a plain time-domain
///             convolution computed here as the reference.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "conv_engine.h"      // the Min-free DSP core — tested directly (the buffer~-backed Min
                              // object can't link against the mock test kernel)

#include <cmath>
#include <vector>

namespace {

// Deterministic pseudo-random sequence in [-1, 1] (a small LCG — no <random> needed, reproducible).
std::vector<double> noise(int n, unsigned seed) {
    std::vector<double> v(n);
    unsigned            s = seed;
    for (int i = 0; i < n; ++i) {
        s    = s * 1664525u + 1013904223u;
        v[i] = (static_cast<double>(s) / 4294967295.0) * 2.0 - 1.0;
    }
    return v;
}

// Direct (naive) linear convolution reference: out[t] = sum_k h[k] * x[t - k].
std::vector<double> direct_convolve(const std::vector<double>& x, const std::vector<float>& h) {
    std::vector<double> out(x.size(), 0.0);
    for (int t = 0; t < static_cast<int>(x.size()); ++t) {
        double acc = 0.0;
        for (int k = 0; k < static_cast<int>(h.size()); ++k) {
            if (t - k >= 0)
                acc += static_cast<double>(h[k]) * x[t - k];
        }
        out[t] = acc;
    }
    return out;
}

} // namespace

SCENARIO("the engine reproduces a true-stereo convolution, latency-shifted by one block") {
    const int B    = 16;  // partition size (power of two)
    const int Pmax = 8;   // capacity 128 samples
    const int len  = 40;  // IR length -> 3 partitions
    const int n    = 300; // signal length

    conv_engine engine;
    engine.configure(B, Pmax);

    GIVEN("four distinct IR paths and two distinct input channels") {
        // Independent IRs on every path so cross-terms can't cancel a bug.
        std::vector<float> hLL(len), hLR(len), hRL(len), hRR(len);
        for (int k = 0; k < len; ++k) {
            hLL[k] = static_cast<float>(std::sin(0.13 * k) * std::exp(-0.04 * k));
            hLR[k] = static_cast<float>(std::cos(0.21 * k) * std::exp(-0.05 * k));
            hRL[k] = static_cast<float>(0.5 * std::sin(0.30 * k) * std::exp(-0.06 * k));
            hRR[k] = static_cast<float>(std::cos(0.17 * k) * std::exp(-0.03 * k));
        }
        const float* paths[4] = {hLL.data(), hLR.data(), hRL.data(), hRR.data()};
        engine.load_ir(paths, len, 1.0);

        const std::vector<double> xL = noise(n, 12345u);
        const std::vector<double> xR = noise(n, 99999u);

        // Reference: outL = xL*hLL + xR*hRL ; outR = xL*hLR + xR*hRR (no latency shift yet).
        const std::vector<double> refLL = direct_convolve(xL, hLL);
        const std::vector<double> refRL = direct_convolve(xR, hRL);
        const std::vector<double> refLR = direct_convolve(xL, hLR);
        const std::vector<double> refRR = direct_convolve(xR, hRR);

        WHEN("processed in awkward chunk sizes that straddle block boundaries") {
            std::vector<double> outL(n, 0.0), outR(n, 0.0);
            int                 i = 0;
            const int           chunk = 10; // not a divisor of B — exercises mid-vector block crossing
            while (i < n) {
                const long c = std::min<long>(chunk, n - i);
                engine.process(&xL[i], &xR[i], &outL[i], &outR[i], c);
                i += c;
            }

            THEN("the output equals the reference convolution delayed by exactly one block (B samples)") {
                bool ok = true;
                for (int t = B; t < n; ++t) {
                    const double expL = refLL[t - B] + refRL[t - B];
                    const double expR = refLR[t - B] + refRR[t - B];
                    if (std::abs(outL[t] - expL) > 1e-9)
                        ok = false;
                    if (std::abs(outR[t] - expR) > 1e-9)
                        ok = false;
                }
                REQUIRE(ok);
            }
            THEN("the first block of output is the pre-roll latency (silence)") {
                bool silent = true;
                for (int t = 0; t < B; ++t) {
                    if (std::abs(outL[t]) > 1e-12 || std::abs(outR[t]) > 1e-12)
                        silent = false;
                }
                REQUIRE(silent);
            }
        }
    }
}

SCENARIO("a delayed unit impulse on one path is a pure delay on that path only") {
    const int B    = 8;
    const int Pmax = 8;

    conv_engine engine;
    engine.configure(B, Pmax);

    GIVEN("hLL = delta at index 5, all other paths silent") {
        std::vector<float> hLL(16, 0.0f);
        hLL[5]                = 1.0f;
        const float* paths[4] = {hLL.data(), nullptr, nullptr, nullptr};
        engine.load_ir(paths, 16, 1.0);

        const int                 n  = 128;
        const std::vector<double> xL = noise(n, 7u);
        const std::vector<double> xR = noise(n, 8u);

        WHEN("processed") {
            std::vector<double> outL(n, 0.0), outR(n, 0.0);
            engine.process(xL.data(), xR.data(), outL.data(), outR.data(), n);

            THEN("outL is inL delayed by (block latency + 5), and outR is silent") {
                const int delay = B + 5;
                bool      okL = true, okR = true;
                for (int t = delay; t < n; ++t) {
                    if (std::abs(outL[t] - xL[t - delay]) > 1e-9)
                        okL = false;
                }
                for (int t = 0; t < n; ++t) {
                    if (std::abs(outR[t]) > 1e-12)
                        okR = false;
                }
                REQUIRE(okL);
                REQUIRE(okR);
            }
        }
    }
}

SCENARIO("with no IR loaded the engine is silent") {
    conv_engine engine;
    engine.configure(32, 4);
    REQUIRE(engine.has_ir() == false);

    const int                 n  = 96;
    const std::vector<double> xL = noise(n, 1u);
    const std::vector<double> xR = noise(n, 2u);
    std::vector<double>       outL(n, 0.0), outR(n, 0.0);
    engine.process(xL.data(), xR.data(), outL.data(), outR.data(), n);

    bool silent = true;
    for (int t = 0; t < n; ++t) {
        if (std::abs(outL[t]) > 1e-12 || std::abs(outR[t]) > 1e-12)
            silent = false;
    }
    REQUIRE(silent);
}

SCENARIO("a hot IR swap while running settles to the new response") {
    const int B    = 16;
    const int Pmax = 8;

    conv_engine engine;
    engine.configure(B, Pmax);

    GIVEN("an initial decaying IR on the diagonal paths, running mid-stream") {
        std::vector<float> h1(20), h2(20);
        for (int k = 0; k < 20; ++k) {
            h1[k] = static_cast<float>(std::exp(-0.1 * k));
            h2[k] = (k == 3) ? 1.0f : 0.0f; // the swap target: a delta at index 3
        }
        const float* pa[4] = {h1.data(), nullptr, nullptr, h1.data()};
        engine.load_ir(pa, 20, 1.0);

        const int                 n  = 200;
        const std::vector<double> xL = noise(n, 5u);
        const std::vector<double> xR = noise(n, 6u);
        std::vector<double>       outL(n, 0.0), outR(n, 0.0);

        WHEN("a new IR is published atomically partway through the stream") {
            engine.process(xL.data(), xR.data(), outL.data(), outR.data(), 100);
            const float* pb[4] = {h2.data(), nullptr, nullptr, h2.data()};
            engine.load_ir(pb, 20, 1.0); // lock-free slot swap
            engine.process(&xL[100], &xR[100], &outL[100], &outR[100], 100);

            THEN("after the swap settles, outL is inL delayed by (block latency + 3)") {
                const int delay = B + 3;
                bool      ok    = true;
                for (int t = 150; t < n; ++t) {
                    if (std::abs(outL[t] - xL[t - delay]) > 1e-9)
                        ok = false;
                }
                REQUIRE(ok);
            }
        }
    }
}
