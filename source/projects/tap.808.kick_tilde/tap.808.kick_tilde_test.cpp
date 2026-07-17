/// @file
/// @brief      Unit tests for tap.808.kick~ (Min-level behavior; DSP correctness lives in the
///             kernel repo's tr808_kick_test.cpp).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"     // required unit-test header (defines main via Catch)
#include "tap.808.kick_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.808.kick~ instantiates with the hardware-panel defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<kick808> an_instance;
        kick808&              my_object = an_instance;

        THEN("the panel attributes default to noon decay/tone and full level") {
            REQUIRE(static_cast<double>(my_object.decay) == 0.5);
            REQUIRE(static_cast<double>(my_object.tone) == 0.5);
            REQUIRE(static_cast<double>(my_object.level) == 1.0);
        }
        THEN("the circuit bends default to stock hardware") {
            REQUIRE(static_cast<double>(my_object.tuning) == 1.0);
            REQUIRE(static_cast<double>(my_object.pulse) == 1.0);
            REQUIRE(static_cast<double>(my_object.sigh) == 1.0);
            REQUIRE(static_cast<double>(my_object.attack) == 1.0);
        }
    }
}

SCENARIO("tap.808.kick~ clamps attributes to their legal ranges") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<kick808> an_instance;
        kick808&              my_object = an_instance;

        WHEN("panel attributes are pushed out of range") {
            my_object.decay = 3.0;
            my_object.tone  = -1.0;
            my_object.level = 2.0;
            THEN("they clamp to 0..1") {
                REQUIRE(static_cast<double>(my_object.decay) == 1.0);
                REQUIRE(static_cast<double>(my_object.tone) == 0.0);
                REQUIRE(static_cast<double>(my_object.level) == 1.0);
            }
        }
        WHEN("the tuning bend is pushed out of range") {
            my_object.tuning = 100.0;
            THEN("it clamps to the 0.25..4 bend range") {
                REQUIRE(static_cast<double>(my_object.tuning) == 4.0);
            }
        }
        WHEN("the pulse width is pushed out of range") {
            my_object.pulse = 0.0;
            THEN("it clamps to the minimum width") {
                REQUIRE(static_cast<double>(my_object.pulse) == 0.05);
            }
        }
    }
}

SCENARIO("tap.808.kick~ is silent at rest and sounds on bang") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<kick808> an_instance;
        kick808&              my_object = an_instance;

        WHEN("no trigger has arrived") {
            double peak = 0.0;
            for (int i = 0; i < 4800; ++i) {
                peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0))));
            }
            THEN("the output is silence") {
                REQUIRE(peak == 0.0);
            }
        }

        WHEN("a bang triggers the voice") {
            my_object.bang();
            double peak = 0.0;
            for (int i = 0; i < 9600; ++i) {
                peak = std::max(peak, std::abs(static_cast<double>(my_object(0.0))));
            }
            THEN("the drum sounds") {
                REQUIRE(peak > 0.1);
            }
        }
    }
}

SCENARIO("tap.808.kick~ reads accent from the trigger edge amplitude") {
    ext_main(nullptr);

    GIVEN("two instances") {
        test_wrapper<kick808> a_instance;
        test_wrapper<kick808> b_instance;
        kick808&              soft = a_instance;
        kick808&              hard = b_instance;

        WHEN("one is triggered by a soft signal edge and the other by a full-scale edge") {
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
}

SCENARIO("tap.808.kick~ clear silences a ringing tail") {
    ext_main(nullptr);

    GIVEN("a ringing instance") {
        test_wrapper<kick808> an_instance;
        kick808&              my_object = an_instance;

        my_object.bang();
        for (int i = 0; i < 4800; ++i) {
            my_object(0.0);
        }

        WHEN("clear arrives") {
            my_object.clear();
            THEN("the output is immediately silent") {
                REQUIRE(static_cast<double>(my_object(0.0)) == 0.0);
            }
        }
    }
}
