/// @file
/// @brief      Unit tests for tap.808.rim~ (Min-level behavior; DSP correctness lives in the
///             kernel repo's tr808_rim_test.cpp).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.808.rim_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.808.rim~ instantiates with the hardware defaults and switches models") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<rim808> an_instance;
        rim808&              my_object = an_instance;

        THEN("the defaults are rimshot at full level") {
            REQUIRE(my_object.model == symbol{"rimshot"});
            REQUIRE(static_cast<double>(my_object.level) == 1.0);
        }
        WHEN("the model switches to claves and level clamps") {
            my_object.model = symbol("claves");
            my_object.level = 2.0;
            THEN("they hold") {
                REQUIRE(my_object.model == symbol{"claves"});
                REQUIRE(static_cast<double>(my_object.level) == 1.0);
            }
        }
    }
}

SCENARIO("tap.808.rim~ is silent at rest and sounds on bang in both models") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<rim808> an_instance;
        rim808&              my_object = an_instance;

        double peak = 0.0;
        for (int i = 0; i < 4800; ++i) {
            peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0))));
        }
        REQUIRE(peak == 0.0);

        my_object.bang();
        peak = 0.0;
        for (int i = 0; i < 4800; ++i) {
            peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0))));
        }
        REQUIRE(peak > 0.1);

        my_object.model = symbol("claves");
        my_object.clear();
        my_object.trigger(atoms{0.8});
        peak = 0.0;
        for (int i = 0; i < 4800; ++i) {
            peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0))));
        }
        REQUIRE(peak > 0.05);
    }
}
