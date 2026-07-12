/// @file
/// @brief      Unit tests for tap.pan~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.pan_tilde.cpp"  // include the object source so we can instantiate it

// With no signal connected to the position inlet, operator() reads the `position` attribute.
// Position -1..1 maps internally to scaled 0..1 (scaled = 0.5*(p+1)).

SCENARIO("tap.pan~ instantiates with the expected defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<pan> an_instance;
        pan&              my_object = an_instance;

        THEN("shape defaults to equal-power (0) and mode to lookup-table (0)") {
            REQUIRE(static_cast<int>(my_object.shape) == 0);
            REQUIRE(static_cast<int>(my_object.mode) == 0);
        }
        THEN("position defaults to 0.0 (center)") {
            REQUIRE(static_cast<double>(my_object.position) == APPROX(0.0));
        }
    }
}

SCENARIO("tap.pan~ equal-power curve (default shape)") {
    ext_main(nullptr);

    GIVEN("a default (equal-power) instance") {
        test_wrapper<pan> an_instance;
        pan&              my_object = an_instance;

        THEN("hard left (-1) sends the signal entirely to the left output") {
            my_object.position = -1.0;
            auto out           = my_object(1.0);
            REQUIRE(static_cast<double>(out[0]) == APPROX(1.0)); // wl = cos(0)
            REQUIRE(static_cast<double>(out[1]) == APPROX(0.0)); // wr = sin(0)
        }
        THEN("hard right (1) sends the signal entirely to the right output") {
            my_object.position = 1.0;
            auto out           = my_object(1.0);
            REQUIRE(static_cast<double>(out[0]) == APPROX(0.0));
            REQUIRE(static_cast<double>(out[1]) == APPROX(1.0));
        }
        THEN("center (0) splits equal-power (~0.707 to each side)") {
            my_object.position = 0.0;
            auto out           = my_object(1.0);
            REQUIRE(static_cast<double>(out[0]) == APPROX(std::sqrt(0.5)));
            REQUIRE(static_cast<double>(out[1]) == APPROX(std::sqrt(0.5)));
        }
    }
}

SCENARIO("tap.pan~ linear (shape=1) and square-root (shape=2) curves") {
    ext_main(nullptr);

    GIVEN("shape set to linear (index 1)") {
        test_wrapper<pan> an_instance;
        pan&              my_object = an_instance;
        my_object.shape             = 1;
        my_object.position          = 0.0; // scaled = 0.5

        THEN("center gives equal linear weights of 0.5") {
            auto out = my_object(1.0);
            REQUIRE(static_cast<double>(out[0]) == APPROX(0.5));
            REQUIRE(static_cast<double>(out[1]) == APPROX(0.5));
        }
    }

    GIVEN("shape set to square-root (index 2)") {
        test_wrapper<pan> an_instance;
        pan&              my_object = an_instance;
        my_object.shape             = 2;
        my_object.position          = -0.5; // scaled = 0.25

        THEN("the square-root curve differs from equal-power: wl=sqrt(.75), wr=sqrt(.25)") {
            REQUIRE(static_cast<int>(my_object.shape) == 2);
            auto out = my_object(1.0);
            REQUIRE(static_cast<double>(out[0]) == APPROX(std::sqrt(0.75)));
            REQUIRE(static_cast<double>(out[1]) == APPROX(std::sqrt(0.25)));
        }
    }
}
