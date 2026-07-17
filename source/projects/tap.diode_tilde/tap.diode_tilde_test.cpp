/// @file
/// @brief      Unit tests for tap.diode~ (Min-level behavior).
/// @details    DSP correctness for the diode-ladder kernel lives in the kernel repo
///             (submodules/taptools/tests/diode_ladder_test.cpp — the Stinchcombe response,
///             oscillation behaviors, solvers, aliasing). These scenarios cover the wrapper:
///             attribute defaults and clamping, message plumbing, and a short smoke of the
///             kernel through the wrapper surface. The harness pins this translation unit to
///             C++17.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <cmath>
#include <cstddef>
#include <vector>

#include "c74_min_unittest.h"
#include "tap.diode_tilde.cpp"

namespace kdf = taptools::diode;

namespace {

    constexpr double k_c_sr = 48000.0;

    kdf::diode_filter make_filter() {
        kdf::diode_filter f;
        f.prepare(k_c_sr);
        f.set_smooth_ms(0.0);
        return f;
    }

    double goertzel(const std::vector<double>& x, size_t lo, size_t hi, double freq_hz) {
        const double w    = 2.0 * kdf::k_pi * freq_hz / k_c_sr;
        const double coef = 2.0 * std::cos(w);
        double       s1 = 0.0, s2 = 0.0;
        for (size_t i = lo; i < hi && i < x.size(); ++i) {
            const double s = x[i] + coef * s1 - s2;
            s2             = s1;
            s1             = s;
        }
        return s1 * s1 + s2 * s2 - coef * s1 * s2;
    }

    size_t at_s(double seconds) {
        return static_cast<size_t>(seconds * k_c_sr);
    }

} // namespace

SCENARIO("the Min wrapper instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("an instance of tap.diode~") {
        test_wrapper<diode> an_instance;
        diode&              my_object = an_instance;

        THEN("attribute defaults match the spec") {
            REQUIRE(static_cast<double>(my_object.frequency) == 1000.0);
            REQUIRE(static_cast<double>(my_object.resonance) == 0.0);
            REQUIRE(static_cast<double>(my_object.drive) == 0.0);
            REQUIRE(static_cast<double>(my_object.fbhp) == kdf::k_fbhp_default_hz);
            REQUIRE(static_cast<double>(my_object.gain) == 0.0);
            REQUIRE(static_cast<int>(my_object.solver) == kdf::solver_fast);
            REQUIRE(static_cast<int>(my_object.oversample) == 2);
            REQUIRE(static_cast<double>(my_object.smooth) == kdf::k_default_smooth_ms);
        }
        THEN("out-of-range values are clamped") {
            my_object.resonance = 2.0;
            REQUIRE(static_cast<double>(my_object.resonance) == kdf::k_res_max);
            my_object.frequency = 1.0;
            REQUIRE(static_cast<double>(my_object.frequency) == kdf::k_freq_min);
            my_object.frequency = 90000.0;
            REQUIRE(static_cast<double>(my_object.frequency) == kdf::k_freq_max);
            my_object.fbhp = -10.0;
            REQUIRE(static_cast<double>(my_object.fbhp) == 0.0);
            my_object.fbhp = 99999.0;
            REQUIRE(static_cast<double>(my_object.fbhp) == kdf::k_fbhp_max_hz);
            my_object.drive = 99.0;
            REQUIRE(static_cast<double>(my_object.drive) == kdf::k_drive_range_db);
            my_object.oversample = 3;
            REQUIRE(static_cast<int>(my_object.oversample) == 2);
            my_object.solver = 99;
            REQUIRE(static_cast<int>(my_object.solver) == kdf::k_num_solvers - 1);
        }
        THEN("attribute changes reach the kernel") {
            my_object.resonance = 0.75;
            REQUIRE(my_object.filter().snap_targets().v[kdf::p_resonance] == 0.75);
            my_object.fbhp = 300.0;
            REQUIRE(my_object.filter().snap_targets().v[kdf::p_fbhp] == 300.0);
        }
        THEN("preset and clear messages are callable") {
            my_object.store(atoms{1});
            my_object.recall(atoms{1});
            my_object.recall(atoms{1, 250.0});
            my_object.clear(atoms{});
            REQUIRE(true);
        }
    }
}

SCENARIO("the wrapped kernel filters: lowpass shape and the stock-vs-bent resonance trait") {
    GIVEN("a 500 Hz cutoff") {
        auto tone_power = [](double freq) {
            auto                f = make_filter();
            f.set_frequency(500.0);
            std::vector<double> out(at_s(0.5));
            for (size_t i = 0; i < out.size(); ++i) {
                out[i] = f.process(0.05 * std::sin(2.0 * kdf::k_pi * freq * i / k_c_sr));
            }
            return goertzel(out, at_s(0.25), at_s(0.5), freq);
        };
        THEN("three octaves above cutoff sits far below the passband floor") {
            REQUIRE(tone_power(4000.0) < 0.001 * tone_power(100.0));
        }
    }
    GIVEN("a ping at stock maximum resonance") {
        auto f = make_filter();
        f.set_frequency(1000.0);
        f.set_resonance(1.0);
        f.process(0.5);
        double tail = 0.0;
        for (size_t i = 0; i < at_s(2.0); ++i) {
            const double y = f.process(0.0);
            if (i >= at_s(1.5)) {
                tail += y * y;
            }
        }
        THEN("it rings down — a stock 303 does not self-oscillate") {
            REQUIRE(std::sqrt(tail / at_s(0.5)) < 1e-4);
        }
    }
    GIVEN("a ping with the fbhp bent to 0 and resonance past threshold") {
        auto f = make_filter();
        f.set_fbhp(0.0);
        f.set_frequency(1000.0);
        f.set_resonance(1.05);
        f.process(0.5);
        std::vector<double> out(at_s(3.0));
        for (size_t i = 0; i < out.size(); ++i) {
            out[i] = f.process(0.0);
        }
        size_t zc = 0;
        for (size_t i = at_s(2.0) + 1; i < out.size(); ++i) {
            if ((out[i] >= 0.0) != (out[i - 1] >= 0.0)) {
                ++zc;
            }
        }
        const double freq = zc / (2.0 * 1.0);
        THEN("it sings at the tuned cutoff") {
            REQUIRE(freq > 950.0);
            REQUIRE(freq < 1050.0);
        }
    }
}
