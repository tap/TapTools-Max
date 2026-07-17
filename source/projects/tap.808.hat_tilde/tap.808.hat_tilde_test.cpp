/// @file
/// @brief      Unit tests for tap.808.hat~ (Min-level behavior; DSP correctness lives in the
///             kernel repo's tr808_hat_test.cpp).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.808.hat_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.808.hat~ instantiates with the hardware defaults and clamps") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<hat808> an_instance;
        hat808&              my_object = an_instance;

        THEN("the panel defaults are noon decay, full levels, stock bends") {
            REQUIRE(static_cast<double>(my_object.decay) == 0.5);
            REQUIRE(static_cast<double>(my_object.closed_level) == 1.0);
            REQUIRE(static_cast<double>(my_object.open_level) == 1.0);
            REQUIRE(static_cast<double>(my_object.tuning) == 1.0);
            REQUIRE(static_cast<double>(my_object.tolerance) == 0.0);
            REQUIRE(static_cast<int>(my_object.seed) == 1);
        }
        WHEN("attributes are pushed out of range") {
            my_object.decay     = 2.0;
            my_object.tolerance = -1.0;
            THEN("they clamp") {
                REQUIRE(static_cast<double>(my_object.decay) == 1.0);
                REQUIRE(static_cast<double>(my_object.tolerance) == 0.0);
            }
        }
    }
}

SCENARIO("tap.808.hat~ is silent at rest, ticks on bang, opens on the open message") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<hat808> an_instance;
        hat808&              my_object = an_instance;

        double peak = 0.0;
        for (int i = 0; i < 4800; ++i) {
            peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0, 0.0))));
        }
        REQUIRE(peak == 0.0);

        my_object.bang(); // closed
        peak = 0.0;
        for (int i = 0; i < 9600; ++i) {
            peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0, 0.0))));
        }
        REQUIRE(peak > 0.1);

        my_object.clear();
        my_object.open(atoms{1.0});
        peak = 0.0;
        for (int i = 0; i < 9600; ++i) {
            peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0, 0.0))));
        }
        REQUIRE(peak > 0.1);
    }
}

SCENARIO("tap.808.hat~ triggers from both signal inlets and chokes") {
    ext_main(nullptr);

    GIVEN("an instance with a sounding open hat") {
        test_wrapper<hat808> an_instance;
        hat808&              my_object = an_instance;
        my_object.decay                = 1.0;

        my_object(0.0, 1.0); // open-inlet edge
        double open_early = 0.0;
        for (int i = 0; i < 4800; ++i) {
            open_early = std::max(open_early, std::abs(static_cast<double>(my_object(0.0, 1.0))));
        }
        REQUIRE(open_early > 0.05);

        WHEN("a closed edge arrives on the first inlet") {
            my_object(1.0, 1.0);
            // Let the choke run its course, then measure the would-be tail window.
            for (int i = 0; i < 9600; ++i) {
                my_object(1.0, 1.0);
            }
            double tail = 0.0;
            for (int i = 0; i < 9600; ++i) {
                tail = std::max(tail, std::abs(static_cast<double>(my_object(1.0, 1.0))));
            }
            THEN("the open tail is choked away") {
                REQUIRE(tail < open_early * 0.5);
            }
        }
    }
}
