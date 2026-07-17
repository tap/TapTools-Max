/// @file
/// @brief      Unit tests for tap.808.tom~ (Min-level behavior; DSP correctness lives in the
///             kernel repo's tr808_tom_test.cpp).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.808.tom_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.808.tom~ instantiates with the hardware defaults and clamps") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<tom808> an_instance;
        tom808&              my_object = an_instance;

        THEN("the defaults are low tom, noon tuning, full level") {
            REQUIRE(my_object.size == symbol{"low"});
            REQUIRE(my_object.model == symbol{"tom"});
            REQUIRE(static_cast<double>(my_object.tuning) == 0.5);
            REQUIRE(static_cast<double>(my_object.level) == 1.0);
            REQUIRE(static_cast<int>(my_object.seed) == 1);
        }
        WHEN("attributes are pushed out of range") {
            my_object.tuning = 3.0;
            my_object.level  = -1.0;
            THEN("they clamp") {
                REQUIRE(static_cast<double>(my_object.tuning) == 1.0);
                REQUIRE(static_cast<double>(my_object.level) == 0.0);
            }
        }
        WHEN("size and model switch") {
            my_object.size  = symbol("high");
            my_object.model = symbol("conga");
            THEN("they hold") {
                REQUIRE(my_object.size == symbol{"high"});
                REQUIRE(my_object.model == symbol{"conga"});
            }
        }
    }
}

SCENARIO("tap.808.tom~ is silent at rest, sounds on bang, and reads edge accents") {
    ext_main(nullptr);

    GIVEN("instances") {
        test_wrapper<tom808> a_instance;
        test_wrapper<tom808> b_instance;
        tom808&              my_object = a_instance;
        tom808&              hard      = b_instance;

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
