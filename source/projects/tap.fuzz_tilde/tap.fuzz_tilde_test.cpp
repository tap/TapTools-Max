/// @file
/// @brief      Unit tests for tap.fuzz~ (the two-stage tone-stacked fuzz).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/fuzz_test.cpp)
///             carries the DSP promises. Here: documented defaults (including the oversample
///             default of 2, which is a measured choice rather than a typo and is therefore
///             worth pinning), the attribute clamps, the oversample snapping, and the one
///             contract a patcher can check without a scope — silence in, silence out, exactly,
///             even wide open and asymmetric.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.fuzz_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.fuzz~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<fuzz> an_instance;
        fuzz&              my_object = an_instance;

        THEN("mid gain and edge, no asymmetry, a flat pair of shelves and a little scoop") {
            REQUIRE(static_cast<double>(my_object.gain) == 0.5);
            REQUIRE(static_cast<double>(my_object.edge) == 0.5);
            REQUIRE(static_cast<double>(my_object.asymmetry) == 0.0);
            REQUIRE(static_cast<double>(my_object.bass) == 0.0);
            REQUIRE(static_cast<double>(my_object.treble) == 0.0);
            REQUIRE(static_cast<double>(my_object.contrast) == 0.35);
            REQUIRE(static_cast<double>(my_object.level) == 0.0);
        }
        THEN("oversample defaults to 2 — the measured best, not the largest") {
            REQUIRE(static_cast<int>(my_object.oversample) == 2);
        }
    }
}

SCENARIO("tap.fuzz~ clamps its controls to their documented ranges") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<fuzz> an_instance;
        fuzz&              my_object = an_instance;

        THEN("the 0..1 controls are probabilities-shaped") {
            my_object.gain = 4.0;
            REQUIRE(static_cast<double>(my_object.gain) == 1.0);
            my_object.edge = -2.0;
            REQUIRE(static_cast<double>(my_object.edge) == 0.0);
            my_object.asymmetry = 9.0;
            REQUIRE(static_cast<double>(my_object.asymmetry) == 1.0);
            my_object.contrast = -1.0;
            REQUIRE(static_cast<double>(my_object.contrast) == 0.0);
        }
        THEN("the shelves travel both ways") {
            my_object.bass = -9.0;
            REQUIRE(static_cast<double>(my_object.bass) == -1.0);
            my_object.treble = 9.0;
            REQUIRE(static_cast<double>(my_object.treble) == 1.0);
        }
        THEN("level stops at the kernel's range") {
            my_object.level = 99.0;
            REQUIRE(static_cast<double>(my_object.level) == 24.0);
        }
    }
}

SCENARIO("tap.fuzz~ snaps oversample to a legal factor") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<fuzz> an_instance;
        fuzz&              my_object = an_instance;

        THEN("in-between values snap down to the next legal factor") {
            my_object.oversample = 3;
            REQUIRE(static_cast<int>(my_object.oversample) == 2);
            my_object.oversample = 7;
            REQUIRE(static_cast<int>(my_object.oversample) == 4);
            my_object.oversample = 99;
            REQUIRE(static_cast<int>(my_object.oversample) == 8);
            my_object.oversample = 0;
            REQUIRE(static_cast<int>(my_object.oversample) == 1);
        }
    }
}

// The bias that buys even harmonics lives inside the curve and is corrected at the stage
// output, so an asymmetric pedal must still be exactly quiet on silence — no DC pedestal.
SCENARIO("tap.fuzz~ is exactly silent on silence, even wide open and asymmetric") {
    ext_main(nullptr);

    GIVEN("the pedal wide open and lopsided") {
        test_wrapper<fuzz> an_instance;
        fuzz&              my_object = an_instance;
        my_object.smooth             = 0.0;
        my_object.gain               = 1.0;
        my_object.edge               = 1.0;
        my_object.asymmetry          = 1.0;

        THEN("every output sample is bitwise zero") {
            bool silent = true;
            for (int i = 0; i < 8000; ++i) {
                silent = silent && (my_object(0.0) == 0.0);
            }
            REQUIRE(silent);
        }
    }
}

SCENARIO("tap.fuzz~ distorts more as the gain rises") {
    ext_main(nullptr);

    GIVEN("a fixed tone at two gain settings") {
        constexpr double k_mock_sr = 44100.0;
        auto             peak_of   = [&](double g) {
            test_wrapper<fuzz> an_instance;
            fuzz&              my_object = an_instance;
            my_object.smooth             = 0.0;
            my_object.contrast           = 0.0;
            my_object.gain               = g;
            double top                   = 0.0;
            for (int i = 0; i < 8000; ++i) {
                const double x = 0.3 * std::sin(2.0 * 3.14159265358979323846 * 220.0 * i / k_mock_sr);
                const double y = my_object(x);
                if (i > 4000) {
                    top = std::max(top, std::abs(y));
                }
            }
            return top;
        };

        THEN("a hotter gain makes a hotter output") {
            const double quiet = peak_of(0.0);
            const double loud  = peak_of(1.0);
            INFO("peak at gain 0 = " << quiet << ", at gain 1 = " << loud);
            REQUIRE(loud > quiet * 2.0);
        }
    }
}
