/// @file
/// @brief      Unit tests for tap.808.snare~ (Min-level behavior; DSP correctness lives in the
///             kernel repo's tr808_snare_test.cpp).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"      // required unit-test header (defines main via Catch)
#include "tap.808.snare_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.808.snare~ instantiates with the hardware-panel defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<snare808> an_instance;
        snare808&              my_object = an_instance;

        THEN("the panel attributes default to noon tone/snappy and full level") {
            REQUIRE(static_cast<double>(my_object.tone) == 0.5);
            REQUIRE(static_cast<double>(my_object.snappy) == 0.5);
            REQUIRE(static_cast<double>(my_object.level) == 1.0);
        }
        THEN("the extensions default to stock") {
            REQUIRE(static_cast<double>(my_object.tuning) == 1.0);
            REQUIRE(static_cast<int>(my_object.seed) == 1);
        }
    }
}

SCENARIO("tap.808.snare~ clamps attributes to their legal ranges") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<snare808> an_instance;
        snare808&              my_object = an_instance;

        WHEN("attributes are pushed out of range") {
            my_object.tone   = 2.0;
            my_object.snappy = -1.0;
            my_object.tuning = 0.0;
            THEN("they clamp") {
                REQUIRE(static_cast<double>(my_object.tone) == 1.0);
                REQUIRE(static_cast<double>(my_object.snappy) == 0.0);
                REQUIRE(static_cast<double>(my_object.tuning) == 0.25);
            }
        }
    }
}

SCENARIO("tap.808.snare~ is silent at rest, sounds on bang, and clear silences it") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<snare808> an_instance;
        snare808&              my_object = an_instance;

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
        REQUIRE(static_cast<double>(my_object(0.0)) == 0.0);
    }
}

SCENARIO("tap.808.snare~ reads accent from the trigger edge amplitude") {
    ext_main(nullptr);

    GIVEN("two instances") {
        test_wrapper<snare808> a_instance;
        test_wrapper<snare808> b_instance;
        snare808&              soft = a_instance;
        snare808&              hard = b_instance;

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
