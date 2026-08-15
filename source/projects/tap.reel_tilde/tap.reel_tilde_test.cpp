/// @file
/// @brief      Unit tests for tap.reel~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.reel_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.reel~ instantiates with the expected defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<reel> an_instance;
        reel&              my_object = an_instance;

        THEN("the reel is transparent: unity level, centred, shade bypassed") {
            REQUIRE(static_cast<double>(my_object.level) == 1.0);
            REQUIRE(static_cast<double>(my_object.pan) == 0.0);
            REQUIRE(static_cast<double>(my_object.darken) == tap::tools::tape::k_darken_ceil_hz);
        }
        THEN("the tape buy defaults to the kernel's worst case") {
            REQUIRE(static_cast<double>(my_object.maxloop) == kernel::k_default_max_seconds);
        }
    }
}

SCENARIO("tap.reel~ clamps what the kernel clamps") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<reel> an_instance;
        reel&              my_object = an_instance;

        THEN("pan is held to the equal-power range") {
            my_object.pan = 4.0;
            REQUIRE(static_cast<double>(my_object.pan) == 1.0);
            my_object.pan = -4.0;
            REQUIRE(static_cast<double>(my_object.pan) == -1.0);
        }
        THEN("darken is held to the audible band") {
            my_object.darken = 1.0;
            REQUIRE(static_cast<double>(my_object.darken) == tap::tools::tape::k_darken_floor_hz);
            my_object.darken = 1e9;
            REQUIRE(static_cast<double>(my_object.darken) == tap::tools::tape::k_darken_ceil_hz);
        }
        THEN("a length below the floor comes back as the floor, quantized to the sample grid") {
            my_object.length = 0.01;
            REQUIRE(static_cast<double>(my_object.length) >= kernel::k_min_loop_seconds);
        }
        THEN("smoothing cannot go negative") {
            my_object.smooth = -5.0;
            REQUIRE(static_cast<double>(my_object.smooth) == 0.0);
        }
    }
}
