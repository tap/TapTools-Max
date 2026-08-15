/// @file
/// @brief      Unit tests for tap.scale.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.scale.cpp"      // include the object source so we can instantiate it

SCENARIO("tap.scale instantiates with the expected defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<scalesnap> an_instance;
        scalesnap&              my_object = an_instance;

        THEN("it defaults to C major pentatonic — the scale where nothing sounds wrong") {
            REQUIRE(static_cast<int>(my_object.root) == 0);
            REQUIRE(static_cast<int>(my_object.scalemode) == kernel::scale_major_pentatonic);
        }
    }
}

SCENARIO("tap.scale wraps the root and clamps the scale") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<scalesnap> an_instance;
        scalesnap&              my_object = an_instance;

        THEN("a root outside 0..11 wraps into the pitch classes") {
            my_object.root = 14;
            REQUIRE(static_cast<int>(my_object.root) == 2);
        }
        THEN("an out-of-range scale is clamped rather than left dangling") {
            my_object.scalemode = 99;
            REQUIRE(static_cast<int>(my_object.scalemode) == kernel::k_num_scales - 1);
        }
    }
}
