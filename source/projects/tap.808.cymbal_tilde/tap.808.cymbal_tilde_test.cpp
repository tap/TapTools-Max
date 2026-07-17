/// @file
/// @brief      Unit tests for tap.808.cymbal~ (Min-level behavior; DSP correctness lives in
///             the kernel repo's tr808_cymbal_test.cpp).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"       // required unit-test header (defines main via Catch)
#include "tap.808.cymbal_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.808.cymbal~ instantiates with the hardware defaults and clamps") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<cymbal808> an_instance;
        cymbal808&              my_object = an_instance;

        THEN("the panel defaults are noon tone/decay, full level, stock bends") {
            REQUIRE(static_cast<double>(my_object.tone) == 0.5);
            REQUIRE(static_cast<double>(my_object.decay) == 0.5);
            REQUIRE(static_cast<double>(my_object.level) == 1.0);
            REQUIRE(static_cast<double>(my_object.tuning) == 1.0);
            REQUIRE(static_cast<double>(my_object.tolerance) == 0.0);
        }
        WHEN("attributes are pushed out of range") {
            my_object.tone   = -2.0;
            my_object.tuning = 100.0;
            THEN("they clamp") {
                REQUIRE(static_cast<double>(my_object.tone) == 0.0);
                REQUIRE(static_cast<double>(my_object.tuning) == 4.0);
            }
        }
    }
}

SCENARIO("tap.808.cymbal~ is silent at rest, sounds on bang, and reads edge accents") {
    ext_main(nullptr);

    GIVEN("instances") {
        test_wrapper<cymbal808> a_instance;
        test_wrapper<cymbal808> b_instance;
        cymbal808&              my_object = a_instance;
        cymbal808&              hard      = b_instance;

        double peak = 0.0;
        for (int i = 0; i < 4800; ++i) {
            peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0))));
        }
        REQUIRE(peak == 0.0);

        my_object.bang();
        peak = 0.0;
        for (int i = 0; i < 9600; ++i) {
            peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0))));
        }
        REQUIRE(peak > 0.1);

        // Edge accent: a soft edge on a fresh instance vs the bang above.
        my_object.clear();
        double peak_soft = std::abs(static_cast<double>(my_object(0.2)));
        double peak_hard = std::abs(static_cast<double>(hard(1.0)));
        for (int i = 0; i < 9600; ++i) {
            peak_soft = std::max(peak_soft, std::abs(static_cast<double>(my_object(0.2))));
            peak_hard = std::max(peak_hard, std::abs(static_cast<double>(hard(1.0))));
        }
        REQUIRE(peak_soft > 0.01);
        REQUIRE(peak_hard > peak_soft);
    }
}
