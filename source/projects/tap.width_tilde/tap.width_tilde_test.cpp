/// @file
/// @brief      Unit tests for tap.width~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include "c74_min_unittest.h"  // required unit-test header (defines main via Catch)
#include "tap.width_tilde.cpp" // include the object source so we can instantiate it

// The mock kernel's sys_getsr() returns 44100, so samplerate() is 44100 here.
static constexpr double k_sr = 44100.0;

SCENARIO("tap.width~ outputs zero until a pulse completes") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<width> an_instance;
        width&              my_object = an_instance;

        THEN("while the input is high the held width stays at its initial zero") {
            REQUIRE(static_cast<double>(my_object(1.0)) == 0.0);
            REQUIRE(static_cast<double>(my_object(1.0)) == 0.0);
            REQUIRE(static_cast<double>(my_object(1.0)) == 0.0);
        }
    }
}

SCENARIO("tap.width~ reports the pulse width in ms when the pulse ends") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<width> an_instance;
        width&              my_object = an_instance;

        WHEN("the input is high for 4 samples then falls to zero") {
            for (int i = 0; i < 4; ++i) {
                my_object(1.0); // counter -> 4
            }

            const double w = static_cast<double>(my_object(0.0)); // pulse ends

            THEN("the reported width is (1000 * 4) / samplerate ms") {
                REQUIRE(w == APPROX((1000.0 * 4.0) / k_sr));
            }
        }
    }
}

SCENARIO("tap.width~ holds the last measured width across the gap until the next pulse ends") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<width> an_instance;
        width&              my_object = an_instance;

        // First pulse: 4 samples high then low.
        for (int i = 0; i < 4; ++i) {
            my_object(1.0);
        }
        const double first = static_cast<double>(my_object(0.0));
        REQUIRE(first == APPROX((1000.0 * 4.0) / k_sr));

        THEN("further zeros hold the value (counter is 0, no new measurement)") {
            REQUIRE(static_cast<double>(my_object(0.0)) == APPROX(first));
            REQUIRE(static_cast<double>(my_object(0.0)) == APPROX(first));
        }

        WHEN("a second, longer pulse of 10 samples completes") {
            for (int i = 0; i < 10; ++i) {
                REQUIRE(static_cast<double>(my_object(1.0)) == APPROX(first)); // still holding old width
            }
            const double second = static_cast<double>(my_object(0.0));

            THEN("the width updates to the new measurement") {
                REQUIRE(second == APPROX((1000.0 * 10.0) / k_sr));
            }
        }
    }
}

SCENARIO("tap.width~ treats exactly zero and negative input as 'low'") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<width> an_instance;
        width&              my_object = an_instance;

        WHEN("the pulse is ended by a negative sample") {
            my_object(1.0);
            my_object(1.0);                                        // counter -> 2
            const double w = static_cast<double>(my_object(-0.5)); // x <= 0, counter != 0 -> measure

            THEN("the width reflects the 2 high samples") {
                REQUIRE(w == APPROX((1000.0 * 2.0) / k_sr));
            }
        }
    }
}
