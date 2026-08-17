/// @file
/// @brief      Unit tests for tap.ondes~ (the Ondes Martenot voice).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/ondes_test.cpp)
///             carries the tube model, the closed-form detector's equivalence to the full
///             heterodyne simulation, and the harmonic measurements. Here: the documented
///             defaults, the clamps, and the two behaviours a patcher would file as bugs — a key
///             at rest is exactly silent, and the object makes sound with nothing plugged into it,
///             because it is a source.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"  // required unit-test header (defines main via Catch)
#include "tap.ondes_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.ondes~ instantiates with its documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<ondes> an_instance;
        ondes&              my_object = an_instance;

        THEN("the performance surface and the circuit are where the header says") {
            REQUIRE(static_cast<double>(my_object.ribbon) == 24.0);
            REQUIRE(static_cast<double>(my_object.key) == 1.0);
            REQUIRE(static_cast<double>(my_object.depth) == kernel::k_default_depth);
            REQUIRE(static_cast<double>(my_object.detect) == kernel::k_detect_ms);
            REQUIRE(static_cast<double>(my_object.drive) == 1.0);
            REQUIRE(static_cast<int>(my_object.keyplacement) == kernel::key_after);
            REQUIRE(static_cast<int>(my_object.polarity) == 1);
            REQUIRE(static_cast<bool>(my_object.power) == false);
            REQUIRE(static_cast<int>(my_object.oversample) == kernel::k_default_os);
        }
        THEN("the published detector time constant is the default, in ms") {
            REQUIRE(std::abs(static_cast<double>(my_object.detect) - 0.2) < 1e-12);
        }
    }
}

SCENARIO("tap.ondes~ clamps its performance surface") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<ondes> an_instance;
        ondes&              my_object = an_instance;

        THEN("the ribbon stays on the instrument") {
            my_object.ribbon = 500.0;
            REQUIRE(static_cast<double>(my_object.ribbon) == kernel::k_max_semitones);
            my_object.ribbon = -5.0;
            REQUIRE(static_cast<double>(my_object.ribbon) == 0.0);
        }
        THEN("the key stays on its travel") {
            my_object.key = 4.0;
            REQUIRE(static_cast<double>(my_object.key) == 1.0);
            my_object.key = -1.0;
            REQUIRE(static_cast<double>(my_object.key) == 0.0);
        }
        THEN("depth is a fraction and drive is bounded") {
            my_object.depth = 3.0;
            REQUIRE(static_cast<double>(my_object.depth) == 1.0);
            my_object.drive = 99.0;
            REQUIRE(static_cast<double>(my_object.drive) == 8.0);
            my_object.drive = -1.0;
            REQUIRE(static_cast<double>(my_object.drive) == 0.0);
        }
        THEN("oversample snaps to a power of two inside the range") {
            my_object.oversample = 7;
            REQUIRE(static_cast<int>(my_object.oversample) == 4);
            my_object.oversample = 99;
            REQUIRE(static_cast<int>(my_object.oversample) == 8);
            my_object.oversample = 0;
            REQUIRE(static_cast<int>(my_object.oversample) == 1);
        }
        THEN("polarity is a sign, and key placement one of the two readings offered") {
            my_object.polarity = -7;
            REQUIRE(static_cast<int>(my_object.polarity) == -1);
            my_object.polarity = 3;
            REQUIRE(static_cast<int>(my_object.polarity) == 1);
            my_object.keyplacement = 9;
            REQUIRE(static_cast<int>(my_object.keyplacement) == kernel::k_num_key_placements - 1);
        }
    }
}

// The two things a patcher would otherwise file as bugs.
SCENARIO("tap.ondes~ is a source, and it is exactly silent with the key at rest") {
    ext_main(nullptr);

    GIVEN("a key at rest") {
        test_wrapper<ondes> an_instance;
        ondes&              my_object = an_instance;
        my_object.smooth              = 0.0;
        my_object.key                 = 0.0;

        THEN("nothing comes out at all — the bottom of the key's travel is true silence") {
            bool silent = true;
            for (int i = 0; i < 8000; ++i) {
                silent = silent && (my_object(0.0, 0.0) == 0.0);
            }
            REQUIRE(silent);
        }
    }

    GIVEN("a key at full press and nothing patched into the inlets") {
        test_wrapper<ondes> an_instance;
        ondes&              my_object = an_instance;
        my_object.smooth              = 0.0;
        my_object.key                 = 1.0;
        my_object.level               = 1.0;

        THEN("it plays anyway, because it is an instrument rather than an effect") {
            double peak = 0.0;
            for (int i = 0; i < 24000; ++i) {
                peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0, 0.0))));
            }
            INFO("peak with no input at all: " << peak);
            REQUIRE(peak > 0.05);
        }
    }
}

SCENARIO("tap.ondes~ plays the note the ribbon asks for") {
    ext_main(nullptr);

    GIVEN("the ribbon two octaves up from A1") {
        test_wrapper<ondes> an_instance;
        ondes&              my_object = an_instance;
        my_object.smooth              = 0.0;
        my_object.key                 = 1.0;
        my_object.level               = 1.0;
        my_object.ribbon              = 24.0;

        THEN("the strongest partial is A3, and it is stronger than its own harmonics") {
            const double        sr = my_object.samplerate();
            const double        f0 = 55.0 * 4.0;
            const int           n  = static_cast<int>(sr);
            std::vector<double> y(static_cast<size_t>(n));
            for (int i = 0; i < n; ++i) {
                y[static_cast<size_t>(i)] = static_cast<double>(my_object(0.0, 0.0));
            }
            auto bin = [&](double hz) {
                const double w  = 2.0 * 3.14159265358979323846 * hz / sr;
                const double c  = 2.0 * std::cos(w);
                double       s1 = 0.0, s2 = 0.0;
                for (size_t i = y.size() / 2; i < y.size(); ++i) {
                    const double s = y[i] + c * s1 - s2;
                    s2             = s1;
                    s1             = s;
                }
                return std::sqrt(std::max(0.0, s1 * s1 + s2 * s2 - c * s1 * s2)) * 2.0
                       / static_cast<double>(y.size() / 2);
            };
            const double h1 = bin(f0);
            INFO("fundamental " << h1 << ", H2 " << bin(2 * f0) << ", H3 " << bin(3 * f0));
            REQUIRE(h1 > 0.05);
            REQUIRE(bin(2 * f0) < h1);
            REQUIRE(bin(3 * f0) < h1);
        }
    }
}

SCENARIO("tap.ondes~ clears without disturbing its settings") {
    ext_main(nullptr);

    GIVEN("a sounding voice") {
        test_wrapper<ondes> an_instance;
        ondes&              my_object = an_instance;
        my_object.ribbon              = 31.0;
        my_object.drive               = 3.0;
        for (int i = 0; i < 4800; ++i) {
            my_object(0.0, 0.0);
        }

        WHEN("it is cleared") {
            my_object.clear();
            THEN("the parameters are untouched") {
                REQUIRE(static_cast<double>(my_object.ribbon) == 31.0);
                REQUIRE(static_cast<double>(my_object.drive) == 3.0);
            }
        }
    }
}
