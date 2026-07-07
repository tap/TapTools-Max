/// @file
/// @brief      Unit tests for tap.split~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include "c74_min_unittest.h"  // required unit-test header (defines main via Catch)
#include "tap.split_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.split~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<split> an_instance;
        split&              my_object = an_instance;

        THEN("low defaults to 0.0 and high to 1.0") {
            REQUIRE(static_cast<double>(my_object.low) == 0.0);
            REQUIRE(static_cast<double>(my_object.high) == 1.0);
        }
    }
}

SCENARIO("tap.split~ routes in-range values to the first outlet") {
    ext_main(nullptr);

    GIVEN("a default instance (range [0, 1])") {
        test_wrapper<split> an_instance;
        split&              my_object = an_instance;

        THEN("a value inside the range comes out the in-range outlet, cmp = 1") {
            // No signal connections to the limit inlets, so it uses the low/high attributes.
            auto out = my_object(0.5, 0.0, 0.0);
            REQUIRE(static_cast<double>(out[0]) == 0.5); // in-range outlet
            REQUIRE(static_cast<double>(out[1]) == 0.0); // out-of-range outlet silent
            REQUIRE(static_cast<double>(out[2]) == 1.0); // comparison result
        }

        THEN("the inclusive boundaries are in-range") {
            auto lo = my_object(0.0, 0.0, 0.0);
            REQUIRE(static_cast<double>(lo[0]) == 0.0);
            REQUIRE(static_cast<double>(lo[2]) == 1.0);

            auto hi = my_object(1.0, 0.0, 0.0);
            REQUIRE(static_cast<double>(hi[0]) == 1.0);
            REQUIRE(static_cast<double>(hi[2]) == 1.0);
        }
    }
}

SCENARIO("tap.split~ routes out-of-range values to the second outlet") {
    ext_main(nullptr);

    GIVEN("a default instance (range [0, 1])") {
        test_wrapper<split> an_instance;
        split&              my_object = an_instance;

        THEN("a value above the range comes out the out-of-range outlet, cmp = 0") {
            auto out = my_object(2.0, 0.0, 0.0);
            REQUIRE(static_cast<double>(out[0]) == 0.0); // in-range outlet silent
            REQUIRE(static_cast<double>(out[1]) == 2.0); // out-of-range outlet carries the value
            REQUIRE(static_cast<double>(out[2]) == 0.0); // comparison result
        }

        THEN("a value below the range comes out the out-of-range outlet, cmp = 0") {
            auto out = my_object(-0.5, 0.0, 0.0);
            REQUIRE(static_cast<double>(out[0]) == 0.0);
            REQUIRE(static_cast<double>(out[1]) == -0.5);
            REQUIRE(static_cast<double>(out[2]) == 0.0);
        }
    }
}

SCENARIO("tap.split~ honors custom low/high limits") {
    ext_main(nullptr);

    GIVEN("an instance with range [-1, 1]") {
        test_wrapper<split> an_instance;
        split&              my_object = an_instance;
        my_object.low                 = -1.0;
        my_object.high                = 1.0;

        THEN("-0.5 is now in range") {
            auto out = my_object(-0.5, 0.0, 0.0);
            REQUIRE(static_cast<double>(out[0]) == -0.5);
            REQUIRE(static_cast<double>(out[2]) == 1.0);
        }

        THEN("1.5 is still out of range") {
            auto out = my_object(1.5, 0.0, 0.0);
            REQUIRE(static_cast<double>(out[1]) == 1.5);
            REQUIRE(static_cast<double>(out[2]) == 0.0);
        }
    }
}

SCENARIO("tap.split~ number to the limit inlets updates the range") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<split> an_instance;
        split&              my_object = an_instance;

        WHEN("a float to inlet 1 sets the low limit and inlet 2 sets the high limit") {
            my_object.m_number(atoms{0.25}, 1); // low  = 0.25
            my_object.m_number(atoms{0.75}, 2); // high = 0.75

            THEN("the attributes reflect the new range") {
                REQUIRE(static_cast<double>(my_object.low) == 0.25);
                REQUIRE(static_cast<double>(my_object.high) == 0.75);
            }

            AND_THEN("routing follows the new range") {
                auto in = my_object(0.5, 0.0, 0.0);
                REQUIRE(static_cast<double>(in[2]) == 1.0);
                auto out = my_object(0.1, 0.0, 0.0);
                REQUIRE(static_cast<double>(out[2]) == 0.0);
            }
        }
    }
}
