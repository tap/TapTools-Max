/// @file
/// @brief      Unit tests for tap.stammer~ (the live buffer-stutter rig).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/stammer_test.cpp)
///             carries the DSP promises, including the pinned-dice one-step-delay identity. Here:
///             documented defaults, the attribute clamps forwarding into the kernel, the two
///             contracts a patcher can actually observe (density 0 is a bitwise bypass whatever
///             the seed and whatever the mix; the same seed replays the same performance through
///             the wrapper), and clear rewinding the stream.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.stammer_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    /// The documented material: transients, not a sine.
    std::vector<double> plucks(long n) {
        std::vector<double> x(static_cast<size_t>(n), 0.0);
        for (long i = 0; i < n; ++i) {
            const double t   = static_cast<double>(i) / k_mock_sr;
            const double phi = std::fmod(t, 0.31);
            double       sum = 0.0;
            for (int k = 1; k <= 5; ++k) {
                const double kk = static_cast<double>(k);
                sum += (1.0 / kk) * std::exp(-phi * (4.0 + 3.0 * kk))
                       * std::sin(2.0 * 3.14159265358979323846 * 196.0 * kk * phi);
            }
            x[static_cast<size_t>(i)] = 0.5 * sum;
        }
        return x;
    }

} // namespace

SCENARIO("tap.stammer~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<stammer> an_instance;
        stammer&              my_object = an_instance;

        THEN("a 4 s capture on a 250 ms grid, chopping into quarters, holding up to four") {
            REQUIRE(static_cast<double>(my_object.maxhistory) == 4.0);
            REQUIRE(static_cast<double>(my_object.step) == 250.0);
            REQUIRE(static_cast<double>(my_object.density) == 0.5);
            REQUIRE(static_cast<int>(my_object.divisions) == 4);
            REQUIRE(static_cast<int>(my_object.repeats) == 4);
            REQUIRE(static_cast<double>(my_object.reverse) == 0.25);
            REQUIRE(static_cast<double>(my_object.jump) == 0.0);
            REQUIRE(static_cast<double>(my_object.fade) == 3.0);
            REQUIRE(static_cast<double>(my_object.mix) == 100.0);
        }
    }
}

SCENARIO("tap.stammer~ clamps the dice to their documented ranges") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<stammer> an_instance;
        stammer&              my_object = an_instance;

        THEN("density and reverse are probabilities") {
            my_object.density = 4.0;
            REQUIRE(static_cast<double>(my_object.density) == 1.0);
            my_object.density = -1.0;
            REQUIRE(static_cast<double>(my_object.density) == 0.0);
            my_object.reverse = 9.0;
            REQUIRE(static_cast<double>(my_object.reverse) == 1.0);
        }
        THEN("divisions and repeats stop at the kernel's slots") {
            my_object.divisions = 99;
            REQUIRE(static_cast<int>(my_object.divisions) == 8);
            my_object.divisions = 0;
            REQUIRE(static_cast<int>(my_object.divisions) == 1);
            my_object.repeats = 99;
            REQUIRE(static_cast<int>(my_object.repeats) == 16);
            my_object.repeats = 0;
            REQUIRE(static_cast<int>(my_object.repeats) == 1);
        }
    }
}

// The contract a patcher can check without a scope: switched off, this object is not "nearly"
// transparent, it is the input.
SCENARIO("tap.stammer~ at density 0 is a bitwise bypass, whatever the seed and the mix") {
    ext_main(nullptr);

    GIVEN("the dice switched off and a mid mix") {
        test_wrapper<stammer> an_instance;
        stammer&              my_object = an_instance;
        my_object.smooth                = 0.0;
        my_object.density               = 0.0;
        my_object.mix                   = 50.0;
        my_object.seed                  = 4242;

        THEN("the output is the input, bitwise") {
            const std::vector<double> x     = plucks(8000);
            bool                      exact = true;
            for (double v : x) {
                exact = exact && (my_object(v) == v);
            }
            REQUIRE(exact);
        }
    }
}

SCENARIO("tap.stammer~ replays the same performance for the same seed") {
    ext_main(nullptr);

    GIVEN("a busy machine driven twice from the same seed") {
        const std::vector<double> x = plucks(20000);

        auto run = [&](int seed) {
            test_wrapper<stammer> an_instance;
            stammer&              my_object = an_instance;
            my_object.smooth                = 0.0;
            my_object.step                  = 70.0;
            my_object.density               = 0.8;
            my_object.divisions             = 4;
            my_object.repeats               = 6;
            my_object.reverse               = 0.4;
            my_object.seed                  = seed;
            std::vector<double> y;
            y.reserve(x.size());
            for (double v : x) {
                y.push_back(my_object(v));
            }
            return y;
        };

        const std::vector<double> a = run(12345);
        const std::vector<double> b = run(12345);
        const std::vector<double> c = run(999);

        THEN("the same seed is bit-identical") {
            bool same = true;
            for (size_t i = 0; i < a.size(); ++i) {
                same = same && (a[i] == b[i]);
            }
            REQUIRE(same);
        }
        THEN("and a different seed is a different performance") {
            bool differs = false;
            for (size_t i = 0; i < a.size(); ++i) {
                differs = differs || (a[i] != c[i]);
            }
            REQUIRE(differs);
        }
    }
}

SCENARIO("tap.stammer~ clear rewinds the seeded stream") {
    ext_main(nullptr);

    GIVEN("a machine run once, cleared, then run again on the same material") {
        test_wrapper<stammer> an_instance;
        stammer&              my_object = an_instance;
        my_object.smooth                = 0.0;
        my_object.step                  = 60.0;
        my_object.density               = 0.7;
        my_object.repeats               = 6;
        my_object.seed                  = 7;

        const std::vector<double> x = plucks(16000);
        std::vector<double>       a, b;
        a.reserve(x.size());
        b.reserve(x.size());
        for (double v : x) {
            a.push_back(my_object(v));
        }
        my_object.clear();
        for (double v : x) {
            b.push_back(my_object(v));
        }

        THEN("the second run is the first, bit for bit") {
            bool same = true;
            for (size_t i = 0; i < a.size(); ++i) {
                same = same && (a[i] == b[i]);
            }
            REQUIRE(same);
        }
    }
}
