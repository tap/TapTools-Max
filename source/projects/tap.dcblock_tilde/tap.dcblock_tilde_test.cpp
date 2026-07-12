/// @file
/// @brief      Unit tests for tap.dcblock~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2008-2026 Timothy Place.

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.dcblock_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.dcblock~ instantiates with the expected defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<dcblock> an_instance;
        dcblock&              my_object = an_instance;

        THEN("bypass defaults to false") {
            REQUIRE(static_cast<bool>(my_object.bypass) == false);
        }
        THEN("mute defaults to false") {
            REQUIRE(static_cast<bool>(my_object.mute) == false);
        }
    }
}

SCENARIO("tap.dcblock~ filters a DC offset") {
    ext_main(nullptr);

    // The filter is:  y[n] = x[n] - x[n-1] + R * y[n-1]   with R = 0.9997.
    constexpr double R = 0.9997;

    GIVEN("a default instance") {
        test_wrapper<dcblock> an_instance;
        dcblock&              my_object = an_instance;

        THEN("the first sample of a constant input passes through unchanged") {
            // x0 = 1, x1(prev) = 0, y1(prev) = 0  ->  y = 1 - 0 + R*0 = 1
            REQUIRE(static_cast<double>(my_object(1.0)) == APPROX(1.0));
        }

        THEN("a sustained DC offset decays toward zero") {
            // Re-derive the recurrence by hand for a constant input of 1.0.
            double x1 = 0.0, y1 = 0.0;
            for (int n = 0; n < 5; ++n) {
                const double expected = 1.0 - x1 + R * y1;
                x1                    = 1.0;
                y1                    = expected;
                REQUIRE(static_cast<double>(my_object(1.0)) == APPROX(expected));
            }
            // The output must already be shrinking away from the initial 1.0 toward 0.
            REQUIRE(y1 < 1.0);
            REQUIRE(y1 > 0.0);
        }
    }
}

SCENARIO("tap.dcblock~ bypass and mute behave as documented") {
    ext_main(nullptr);

    GIVEN("an instance with bypass enabled") {
        test_wrapper<dcblock> an_instance;
        dcblock&              my_object = an_instance;
        my_object.bypass                = true;

        THEN("the input passes through untouched") {
            REQUIRE(static_cast<double>(my_object(0.5)) == APPROX(0.5));
            REQUIRE(static_cast<double>(my_object(-0.25)) == APPROX(-0.25));
        }
    }

    GIVEN("an instance with mute enabled") {
        test_wrapper<dcblock> an_instance;
        dcblock&              my_object = an_instance;
        my_object.mute                  = true;

        THEN("the output is silent") {
            REQUIRE(static_cast<double>(my_object(1.0)) == APPROX(0.0));
            REQUIRE(static_cast<double>(my_object(-1.0)) == APPROX(0.0));
        }
    }
}

SCENARIO("tap.dcblock~ clear resets the filter memory") {
    ext_main(nullptr);

    constexpr double R = 0.9997;

    GIVEN("an instance that has processed some samples") {
        test_wrapper<dcblock> an_instance;
        dcblock&              my_object = an_instance;

        my_object(1.0);
        my_object(1.0);

        WHEN("clear is sent") {
            my_object.clear();

            THEN("the filter behaves as freshly constructed") {
                // After clear, x1 = y1 = 0, so the first sample of input 1.0 is 1.0 again.
                REQUIRE(static_cast<double>(my_object(1.0)) == APPROX(1.0 - 0.0 + R * 0.0));
            }
        }
    }
}
