/// @file
/// @brief      CPU benchmark for the tap.svf~ kernel (svf.h) — no Max dependency.
/// @details    Times the kernel over a matrix of configurations (circuit x order x oversampling,
///             static vs signal-rate-modulated cutoff) on deterministic noise. Each config runs
///             several repetitions and reports the BEST (minimum) time — the standard microbench
///             estimator, least polluted by scheduler noise. A checksum of every output sample is
///             accumulated and printed so the optimizer cannot elide the work.
///
///             Output: a human table on stdout, or machine-readable JSON with --json. The ratchet
///             workflow lives in benchmarks/: run with --json, compare against the committed
///             baseline with benchmarks/compare.py, and update the baseline when an optimization
///             lands. Absolute numbers are machine-specific; the ratchet compares like with like.
///
///             Usage: svf_bench [--json] [--seconds N]
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include "svf.h"

namespace ksv = taptools::svf;

namespace {

    constexpr double k_sr = 48000.0;

    struct bench_case {
        std::string                           name;
        std::function<void(ksv::svf_filter&)> setup;
        bool modulated; // true: per-sample tick(cutoff) — the signal-rate-cutoff path
    };

    // Deterministic LCG noise in [-0.25, 0.25].
    struct noise {
        unsigned s{22222u};
        double   next() {
            s = s * 1664525u + 1013904223u;
            return (static_cast<double>(s) / 2147483648.0 - 1.0) * 0.25;
        }
    };

    struct result {
        std::string name;
        double      ns_per_sample;
        double      checksum;
    };

    result run_case(const bench_case& bc, double seconds, int reps) {
        const size_t n = static_cast<size_t>(seconds * k_sr);

        // Pre-generate input (and the modulation ramp) so we time the filter, not the generator.
        std::vector<double> in(n), cf(n);
        noise               ng;
        for (size_t i = 0; i < n; ++i) {
            in[i] = ng.next();
            // 20 Hz exponential cutoff sweep 200..8000 Hz — a demanding but musical modulator
            const double ph = std::fmod(20.0 * i / k_sr, 1.0);
            cf[i]           = 200.0 * std::pow(40.0, ph < 0.5 ? 2.0 * ph : 2.0 - 2.0 * ph);
        }

        double best     = 1e30;
        double checksum = 0.0;
        for (int r = 0; r < reps; ++r) {
            ksv::svf_filter f;
            f.prepare(k_sr, 1);
            f.set_smooth_ms(0.0);
            bc.setup(f);

            double     acc = 0.0;
            const auto t0  = std::chrono::steady_clock::now();
            if (bc.modulated) {
                for (size_t i = 0; i < n; ++i) {
                    acc += f.process(in[i], cf[i]);
                }
            }
            else {
                for (size_t i = 0; i < n; ++i) {
                    acc += f.process(in[i]);
                }
            }
            const auto t1   = std::chrono::steady_clock::now();
            checksum        = acc;
            const double ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / static_cast<double>(n);
            if (ns < best) {
                best = ns;
            }
        }
        return {bc.name, best, checksum};
    }

} // namespace

int main(int argc, char** argv) {
    bool   json    = false;
    double seconds = 4.0;
    int    reps    = 5;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--json")) {
            json = true;
        }
        else if (!std::strcmp(argv[i], "--seconds") && i + 1 < argc) {
            seconds = std::atof(argv[++i]);
        }
        else if (!std::strcmp(argv[i], "--reps") && i + 1 < argc) {
            reps = std::atoi(argv[++i]);
        }
    }

    auto cfg = [](int mode, int order, int circuit, int os, double res) {
        return [=](ksv::svf_filter& f) {
            f.set_mode(mode);
            f.set_order(order);
            f.set_circuit(circuit);
            f.set_oversample(os);
            f.set_frequency(1000.0);
            f.set_resonance(res);
            f.set_drive(6.0);
            f.set_morph(0.375);
        };
    };

    const std::vector<bench_case> cases = {
        {"clean.lp.o2.static", cfg(ksv::mode_lowpass, 2, ksv::circuit_clean, 1, 0.5), false},
        {"clean.lp.o4.static", cfg(ksv::mode_lowpass, 4, ksv::circuit_clean, 1, 0.5), false},
        {"clean.lp.o8.static", cfg(ksv::mode_lowpass, 8, ksv::circuit_clean, 1, 0.5), false},
        {"clean.morph.o4.static", cfg(ksv::mode_morph, 4, ksv::circuit_clean, 1, 0.5), false},
        {"clean.lp.o2.mod", cfg(ksv::mode_lowpass, 2, ksv::circuit_clean, 1, 0.5), true},
        {"clean.lp.o8.mod", cfg(ksv::mode_lowpass, 8, ksv::circuit_clean, 1, 0.5), true},
        {"clean.morph.o4.mod", cfg(ksv::mode_morph, 4, ksv::circuit_clean, 1, 0.5), true},
        {"driven.lp.o2.os1", cfg(ksv::mode_lowpass, 2, ksv::circuit_driven, 1, 0.8), false},
        {"driven.lp.o2.os2", cfg(ksv::mode_lowpass, 2, ksv::circuit_driven, 2, 0.8), false},
        {"driven.lp.o2.os4", cfg(ksv::mode_lowpass, 2, ksv::circuit_driven, 4, 0.8), false},
        {"driven.lp.o4.os2", cfg(ksv::mode_lowpass, 4, ksv::circuit_driven, 2, 0.8), false},
        {"driven.lp.o8.os2", cfg(ksv::mode_lowpass, 8, ksv::circuit_driven, 2, 0.8), false},
        {"driven.lp.o8.os2.mod", cfg(ksv::mode_lowpass, 8, ksv::circuit_driven, 2, 0.8), true},
    };

    std::vector<result> results;
    for (const auto& bc : cases) {
        results.push_back(run_case(bc, seconds, reps));
    }

    if (json) {
        std::printf("{\n  \"samplerate\": %.0f,\n  \"cases\": {\n", k_sr);
        for (size_t i = 0; i < results.size(); ++i) {
            std::printf("    \"%s\": { \"ns_per_sample\": %.3f }%s\n", results[i].name.c_str(),
                        results[i].ns_per_sample, i + 1 < results.size() ? "," : "");
        }
        std::printf("  }\n}\n");
    }
    else {
        std::printf("%-24s %14s %12s %14s\n", "case", "ns/sample", "x realtime", "checksum");
        for (const auto& r : results) {
            const double xrt = 1e9 / (r.ns_per_sample * k_sr);
            std::printf("%-24s %14.2f %12.0f %14.6f\n", r.name.c_str(), r.ns_per_sample, xrt, r.checksum);
        }
    }
    return 0;
}
