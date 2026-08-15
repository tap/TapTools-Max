/// @file
/// @brief      Unit tests for tap.gardener.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.gardener.cpp"   // include the object source so we can instantiate it

SCENARIO("tap.gardener instantiates with the kernel's defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<gardener> an_instance;
        gardener&              my_object = an_instance;

        THEN("the wind's patience and gustiness match the kernel") {
            REQUIRE(static_cast<double>(my_object.idle) == kernel::k_default_idle_seconds);
            REQUIRE(static_cast<double>(my_object.gust) == kernel::k_default_gust);
        }
    }
}

SCENARIO("tap.gardener clamps the wind and lets idling be switched off") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<gardener> an_instance;
        gardener&              my_object = an_instance;

        THEN("gust is a 0..1 amount") {
            my_object.gust = 5.0;
            REQUIRE(static_cast<double>(my_object.gust) == 1.0);
            my_object.gust = -5.0;
            REQUIRE(static_cast<double>(my_object.gust) == 0.0);
        }
        THEN("idle 0 is allowed — it is how the wind is disabled entirely") {
            my_object.idle = 0.0;
            REQUIRE(static_cast<double>(my_object.idle) == 0.0);
        }
        THEN("a negative idle is not, since it would mean the wind never waits") {
            my_object.idle = -3.0;
            REQUIRE(static_cast<double>(my_object.idle) == 0.0);
        }
    }
}
