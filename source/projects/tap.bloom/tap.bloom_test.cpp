/// @file
/// @brief      Unit tests for tap.bloom.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.bloom.cpp"      // include the object source so we can instantiate it

SCENARIO("tap.bloom instantiates with the kernel's defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<bloom> an_instance;
        bloom&              my_object = an_instance;

        THEN("the ring's parameters match the kernel") {
            REQUIRE(static_cast<double>(my_object.loop) == kernel::k_default_loop_seconds);
            REQUIRE(static_cast<double>(my_object.decay) == kernel::k_default_decay);
            REQUIRE(static_cast<double>(my_object.soften) == kernel::k_default_soften);
            REQUIRE(static_cast<double>(my_object.floor) == kernel::k_default_floor);
        }
    }
}

SCENARIO("tap.bloom clamps its parameters to the kernel's ranges") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<bloom> an_instance;
        bloom&              my_object = an_instance;

        THEN("the loop is held between the kernel's bounds") {
            my_object.loop = 0.01;
            REQUIRE(static_cast<double>(my_object.loop) == kernel::k_min_loop_seconds);
            my_object.loop = 9999.0;
            REQUIRE(static_cast<double>(my_object.loop) == kernel::k_max_loop_seconds);
        }
        THEN("decay and soften are per-pass multipliers, so they stay in 0..1") {
            my_object.decay = 3.0;
            REQUIRE(static_cast<double>(my_object.decay) == 1.0);
            my_object.soften = -1.0;
            REQUIRE(static_cast<double>(my_object.soften) == 0.0);
        }
        THEN("the floor never reaches zero, so a bloom always retires") {
            my_object.floor = 0.0;
            REQUIRE(static_cast<double>(my_object.floor) > 0.0);
        }
        THEN("the scheduler tick stays in a range Max can actually service") {
            my_object.interval = 0.0;
            REQUIRE(static_cast<double>(my_object.interval) == 0.1);
            my_object.interval = 1e6;
            REQUIRE(static_cast<double>(my_object.interval) == 100.0);
        }
    }
}
