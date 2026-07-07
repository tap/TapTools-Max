/// @file
/// @brief      Unit tests for tap.verb~ (focus: the oversampling stage).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <cstdlib>
#include <vector>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.verb_tilde.cpp" // include the object source so we can instantiate it

// Run `count` samples of an impulse (first sample = 1.0, rest = 0.0) through the object's
// per-sample operator and collect the left-channel output. The reverb cores randomize their
// delays via std::rand(), so seed before constructing to make the run reproducible.
static std::vector<double> run_impulse(verb& v, int count) {
    std::vector<double> out;
    out.reserve(count);
    for (int i = 0; i < count; ++i) {
        const double x = (i == 0) ? 1.0 : 0.0;
        auto         y = v(x, x);
        out.push_back(y[0]);
    }
    return out;
}

SCENARIO("tap.verb~ oversampling defaults to off and 1x is unchanged") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        std::srand(12345);
        test_wrapper<verb> an_instance;
        verb&              my_object = an_instance;

        THEN("oversampling defaults to 1 (off)") {
            REQUIRE(static_cast<int>(my_object.oversampling) == 1);
        }
    }

    GIVEN("two default instances built from the same RNG seed") {
        // With oversampling == 1 the resampler is bypassed entirely, so two identically-seeded
        // instances must produce bit-identical output for the same input.
        // Run long enough to clear the 100-sample limiter look-ahead latency and the ~4 ms first
        // early reflection so the reverb tail actually reaches the output.
        std::srand(999);
        test_wrapper<verb> wrap_a;
        verb&              obj_a = wrap_a;
        auto               out_a = run_impulse(obj_a, 2048);

        std::srand(999);
        test_wrapper<verb> wrap_b;
        verb&              obj_b = wrap_b;
        auto               out_b = run_impulse(obj_b, 2048);

        THEN("the 1x output is deterministic and identical across the two runs") {
            REQUIRE(out_a.size() == out_b.size());
            for (size_t i = 0; i < out_a.size(); ++i) {
                REQUIRE(out_a[i] == out_b[i]);
            }
        }
        THEN("the impulse produces a non-silent reverb tail") {
            double energy = 0.0;
            for (double s : out_a) {
                energy += s * s;
            }
            REQUIRE(energy > 0.0);
        }
    }

    GIVEN("an instance with oversampling raised to 4") {
        std::srand(2024);
        test_wrapper<verb> an_instance;
        verb&              my_object = an_instance;
        my_object.oversampling       = 4;

        THEN("the value snaps to the allowed factor set and stays 4") {
            REQUIRE(static_cast<int>(my_object.oversampling) == 4);
        }
        THEN("processing remains finite (no NaN/Inf from the resampling path)") {
            auto out = run_impulse(my_object, 64);
            for (double s : out) {
                REQUIRE(std::isfinite(s));
            }
        }
    }

    GIVEN("an instance fed an out-of-range oversampling value") {
        std::srand(7);
        test_wrapper<verb> an_instance;
        verb&              my_object = an_instance;
        my_object.oversampling       = 3; // not in {1,2,4,8}

        THEN("it rounds down to the nearest allowed factor (2)") {
            REQUIRE(static_cast<int>(my_object.oversampling) == 2);
        }
    }
}
