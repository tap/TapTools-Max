/// @file
/// @brief      Unit tests for tap.sift~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2000-2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.sift_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.sift~ instantiates with the expected defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance (float-output mode)") {
        test_wrapper<sift> an_instance;
        sift&              my_object = an_instance;

        THEN("the sift value defaults to 0.0") {
            REQUIRE(static_cast<double>(my_object.value) == 0.0);
        }
        THEN("delivery defaults to the high-priority scheduler thread") {
            REQUIRE(static_cast<bool>(my_object.high_priority) == true);
        }
    }
}
