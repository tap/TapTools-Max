/// @file
/// @brief      Unit tests for tap.verb~ (focus: the oversampling stage and seeded determinism).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <vector>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.verb_tilde.cpp" // include the object source so we can instantiate it

// Run `count` samples of an impulse (first sample = 1.0, rest = 0.0) through the object's
// per-sample operator and collect the left-channel output. The reverb cores deviate their
// delays through a seeded LCG (seed attribute, default 1), so runs are reproducible by
// construction — no global RNG state to arrange.
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
        test_wrapper<verb> an_instance;
        verb&              my_object = an_instance;

        THEN("oversampling defaults to 1 (off)") {
            REQUIRE(static_cast<int>(my_object.oversampling) == 1);
        }
        THEN("seed defaults to 1") {
            REQUIRE(static_cast<int>(my_object.seed) == 1);
        }
    }

    GIVEN("two default instances") {
        // The deviated comb tuning is a pure function of (seed, parameters, sample rate), so two
        // default instances must produce bit-identical output for the same input — no srand()
        // arrangement required (the pre-seed wrapper drew from std::rand()).
        // Run long enough to clear the 100-sample limiter look-ahead latency and the ~4 ms first
        // early reflection so the reverb tail actually reaches the output.
        test_wrapper<verb> wrap_a;
        verb&              obj_a = wrap_a;
        auto               out_a = run_impulse(obj_a, 2048);

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
        test_wrapper<verb> an_instance;
        verb&              my_object = an_instance;
        my_object.oversampling       = 3; // not in {1,2,4,8}

        THEN("it rounds down to the nearest allowed factor (2)") {
            REQUIRE(static_cast<int>(my_object.oversampling) == 2);
        }
    }
}

SCENARIO("tap.verb~ is deterministic per seed — a seed is a serial number") {
    ext_main(nullptr);

    GIVEN("two instances with the same non-default seed") {
        test_wrapper<verb> wrap_a;
        verb&              obj_a = wrap_a;
        obj_a.seed               = 42;
        test_wrapper<verb> wrap_b;
        verb&              obj_b = wrap_b;
        obj_b.seed               = 42;

        THEN("their impulse responses are bit-identical") {
            auto out_a = run_impulse(obj_a, 2048);
            auto out_b = run_impulse(obj_b, 2048);
            for (size_t i = 0; i < out_a.size(); ++i) {
                REQUIRE(out_a[i] == out_b[i]);
            }
        }
    }

    GIVEN("two instances with different seeds") {
        test_wrapper<verb> wrap_a;
        verb&              obj_a = wrap_a;
        obj_a.seed               = 1;
        test_wrapper<verb> wrap_b;
        verb&              obj_b = wrap_b;
        obj_b.seed               = 2;

        THEN("their rooms differ (the deviated prime layouts are decorrelated)") {
            auto out_a          = run_impulse(obj_a, 4096);
            auto out_b          = run_impulse(obj_b, 4096);
            bool any_difference = false;
            for (size_t i = 0; i < out_a.size(); ++i) {
                if (out_a[i] != out_b[i]) {
                    any_difference = true;
                    break;
                }
            }
            REQUIRE(any_difference);
        }
    }

    GIVEN("one instance whose seed is re-set to the same value after processing") {
        test_wrapper<verb> wrap_a;
        verb&              obj_a = wrap_a;
        obj_a.seed               = 7;
        obj_a.dcblock            = false; // clear() resets the reverb + limiter but not the DC
                                          // blocker's one-pole state, so bit-compare without it
        auto first = run_impulse(obj_a, 2048);

        THEN("clearing and re-seeding reproduces the identical tail") {
            obj_a.seed = 7; // re-deviates from the same serial number
            obj_a.clear();
            auto again = run_impulse(obj_a, 2048);
            for (size_t i = 0; i < first.size(); ++i) {
                REQUIRE(first[i] == again[i]);
            }
        }
    }
}
