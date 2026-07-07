/// @file
/// @brief      Unit tests for tap.crossfade~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include "c74_min_unittest.h"      // required unit-test header (defines main via Catch)
#include "tap.crossfade_tilde.cpp" // include the object source so we can instantiate it

// With no signal connected to the position inlet, operator() reads the `position` attribute, so
// these tests set `position` directly and pass a dummy third argument.

SCENARIO("tap.crossfade~ instantiates with the expected defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<crossfade> an_instance;
        crossfade&              my_object = an_instance;

        THEN("shape defaults to equal-power (0) and mode to lookup-table (0)") {
            REQUIRE(static_cast<int>(my_object.shape) == 0);
            REQUIRE(static_cast<int>(my_object.mode) == 0);
        }
        THEN("position defaults to 0.5") {
            REQUIRE(static_cast<double>(my_object.position) == APPROX(0.5));
        }
    }
}

SCENARIO("tap.crossfade~ equal-power curve (default shape)") {
    ext_main(nullptr);

    GIVEN("a default (equal-power) instance") {
        test_wrapper<crossfade> an_instance;
        crossfade&              my_object = an_instance;

        THEN("position 0 passes all of input A (wa=1, wb=0)") {
            my_object.position = 0.0;
            REQUIRE(static_cast<double>(my_object(1.0, 0.5)) == APPROX(1.0));
        }
        THEN("position 1 passes all of input B (wa=0, wb=1)") {
            my_object.position = 1.0;
            REQUIRE(static_cast<double>(my_object(1.0, 0.5)) == APPROX(0.5));
        }
        THEN("position 0.5 uses equal-power weights cos/sin(pi/4)") {
            my_object.position = 0.5;
            // a=b=1 -> cos(pi/4) + sin(pi/4) = sqrt(2)
            REQUIRE(static_cast<double>(my_object(1.0, 1.0)) == APPROX(std::sqrt(2.0)));
        }
    }
}

SCENARIO("tap.crossfade~ linear curve via shape=1") {
    ext_main(nullptr);

    GIVEN("an instance with shape set to linear (index 1)") {
        test_wrapper<crossfade> an_instance;
        crossfade&              my_object = an_instance;
        my_object.shape                   = 1;

        THEN("the integer shape index round-trips") {
            REQUIRE(static_cast<int>(my_object.shape) == 1);
        }
        THEN("position 0.5 gives equal linear weights of 0.5") {
            my_object.position = 0.5;
            REQUIRE(static_cast<double>(my_object(1.0, 0.0)) == APPROX(0.5)); // wa = 0.5
            REQUIRE(static_cast<double>(my_object(0.0, 1.0)) == APPROX(0.5)); // wb = 0.5
        }
        THEN("position 0.25 gives wa=0.75, wb=0.25") {
            my_object.position = 0.25;
            REQUIRE(static_cast<double>(my_object(1.0, 1.0)) == APPROX(1.0)); // 0.75 + 0.25
            REQUIRE(static_cast<double>(my_object(1.0, 0.0)) == APPROX(0.75));
        }
    }
}
