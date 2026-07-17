/// @file
/// @brief      Unit tests for tap.808.clap~ (Min-level behavior; DSP correctness lives in the
///             kernel repo's tr808_clap_test.cpp).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"     // required unit-test header (defines main via Catch)
#include "tap.808.clap_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.808.clap~ instantiates with the hardware defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<clap808> an_instance;
        clap808&              my_object = an_instance;

        THEN("the model defaults to clap, full level, stock tail") {
            REQUIRE(my_object.model == symbol{"clap"});
            REQUIRE(static_cast<double>(my_object.level) == 1.0);
            REQUIRE(static_cast<double>(my_object.tail) == 1.0);
            REQUIRE(static_cast<int>(my_object.seed) == 1);
        }
    }
}

SCENARIO("tap.808.clap~ switches models and clamps attributes") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<clap808> an_instance;
        clap808&              my_object = an_instance;

        WHEN("the model is switched to maracas") {
            my_object.model = symbol("maracas");
            THEN("the kernel follows") {
                REQUIRE(my_object.model == symbol{"maracas"});
            }
        }
        WHEN("attributes are pushed out of range") {
            my_object.level = 3.0;
            my_object.tail  = -1.0;
            THEN("they clamp") {
                REQUIRE(static_cast<double>(my_object.level) == 1.0);
                REQUIRE(static_cast<double>(my_object.tail) == 0.0);
            }
        }
    }
}

SCENARIO("tap.808.clap~ is silent at rest and sounds on bang in both models") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<clap808> an_instance;
        clap808&              my_object = an_instance;

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

        my_object.model = symbol("maracas");
        my_object.clear();
        my_object.bang();
        peak = 0.0;
        for (int i = 0; i < 4800; ++i) {
            peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0))));
        }
        REQUIRE(peak > 0.1);
    }
}

SCENARIO("tap.808.clap~ reads accent from the trigger edge amplitude") {
    ext_main(nullptr);

    GIVEN("two instances") {
        test_wrapper<clap808> a_instance;
        test_wrapper<clap808> b_instance;
        clap808&              soft = a_instance;
        clap808&              hard = b_instance;

        double peak_soft = std::abs(static_cast<double>(soft(0.2)));
        double peak_hard = std::abs(static_cast<double>(hard(1.0)));
        for (int i = 0; i < 9600; ++i) {
            peak_soft = std::max(peak_soft, std::abs(static_cast<double>(soft(0.2))));
            peak_hard = std::max(peak_hard, std::abs(static_cast<double>(hard(1.0))));
        }
        THEN("both sound, and the accented hit is louder") {
            REQUIRE(peak_soft > 0.01);
            REQUIRE(peak_hard > peak_soft);
        }
    }
}
