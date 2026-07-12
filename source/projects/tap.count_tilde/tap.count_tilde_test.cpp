/// @file
/// @brief      Unit tests for tap.count~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "c74_min_unittest.h"  // required unit-test header (defines main via Catch)
#include "tap.count_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.count~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<count> an_instance;
        count&              my_object = an_instance;

        THEN("low_bound defaults to 0") {
            REQUIRE(static_cast<int>(my_object.low_bound) == 0);
        }
        THEN("high_bound defaults to 32000") {
            REQUIRE(static_cast<int>(my_object.high_bound) == 32000);
        }
        THEN("active, loop default to true; autoreset to false") {
            REQUIRE(static_cast<bool>(my_object.active) == true);
            REQUIRE(static_cast<bool>(my_object.loop) == true);
            REQUIRE(static_cast<bool>(my_object.autoreset) == false);
        }
    }
}

SCENARIO("tap.count~ increments once per non-zero sample") {
    ext_main(nullptr);

    GIVEN("a default instance (starts active, m_value 0)") {
        test_wrapper<count> an_instance;
        count&              my_object = an_instance;

        THEN("successive non-zero samples count up by one each time") {
            // Starts active with m_value 0; each active sample increments before output.
            REQUIRE(static_cast<double>(my_object(1.0)) == 1.0);
            REQUIRE(static_cast<double>(my_object(1.0)) == 2.0);
            REQUIRE(static_cast<double>(my_object(1.0)) == 3.0);
        }
    }
}

SCENARIO("tap.count~ pauses while the input is zero and restarts from low_bound") {
    ext_main(nullptr);

    GIVEN("an instance with low_bound 10") {
        test_wrapper<count> an_instance;
        count&              my_object = an_instance;
        my_object.low_bound           = 10;

        THEN("a zero deactivates, then the next non-zero restarts from low_bound") {
            my_object(1.0);                       // active, m_value -> 1
            my_object(1.0);                       // m_value -> 2
            const double paused = my_object(0.0); // zero -> inactive, no increment, value stays 2
            REQUIRE(paused == 2.0);
            // Next non-zero: was inactive, so reset to low_bound (10), become active, then ++ -> 11.
            REQUIRE(static_cast<double>(my_object(1.0)) == 11.0);
            REQUIRE(static_cast<double>(my_object(1.0)) == 12.0);
        }
    }
}

SCENARIO("tap.count~ loop on stops counting at high_bound") {
    ext_main(nullptr);

    GIVEN("an instance with high_bound 3 and loop on (default)") {
        test_wrapper<count> an_instance;
        count&              my_object = an_instance;
        my_object.high_bound          = 3;

        THEN("the count climbs to high_bound, deactivates, then a held input restarts from low_bound") {
            REQUIRE(static_cast<double>(my_object(1.0)) == 1.0);
            REQUIRE(static_cast<double>(my_object(1.0)) == 2.0);
            // value becomes 3, reaches high_bound; loop on -> deactivates (m_active=false).
            REQUIRE(static_cast<double>(my_object(1.0)) == 3.0);
            // Next non-zero sample: m_active is false, so this is treated as a fresh start ->
            // m_value resets to low_bound (0), becomes active, then ++ -> 1.
            REQUIRE(static_cast<double>(my_object(1.0)) == 1.0);
            REQUIRE(static_cast<double>(my_object(1.0)) == 2.0);
        }
    }
}

SCENARIO("tap.count~ loop off clamps at high_bound") {
    ext_main(nullptr);

    GIVEN("an instance with high_bound 3 and loop off") {
        test_wrapper<count> an_instance;
        count&              my_object = an_instance;
        my_object.high_bound          = 3;
        my_object.loop                = false;

        THEN("the count clamps to high_bound and stays active there") {
            REQUIRE(static_cast<double>(my_object(1.0)) == 1.0);
            REQUIRE(static_cast<double>(my_object(1.0)) == 2.0);
            // value 3 reaches high_bound; loop off -> clamp to high_bound (stays active).
            REQUIRE(static_cast<double>(my_object(1.0)) == 3.0);
            // still active but clamped: ++ to 4 then clamped back to 3.
            REQUIRE(static_cast<double>(my_object(1.0)) == 3.0);
        }
    }
}

SCENARIO("tap.count~ reset returns the count to low_bound") {
    ext_main(nullptr);

    GIVEN("an instance with low_bound 5 that has counted up") {
        test_wrapper<count> an_instance;
        count&              my_object = an_instance;
        my_object.low_bound           = 5;

        my_object(1.0); // 1
        my_object(1.0); // 2

        WHEN("reset is sent") {
            my_object.reset();

            THEN("m_value is low_bound; next active sample is low_bound + 1") {
                // reset sets m_value = low_bound (5). Object is still active (input was non-zero).
                // Next non-zero sample: active, ++ -> 6.
                REQUIRE(static_cast<double>(my_object(1.0)) == 6.0);
            }
        }
    }
}
