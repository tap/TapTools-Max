/// @file
/// @brief      Unit tests for tap.counter~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.counter_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.counter~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<counter> an_instance;
        counter&              my_object = an_instance;

        THEN("low_bound 0, high_bound 2000000, direction up, init_value 0") {
            REQUIRE(static_cast<int>(my_object.low_bound) == 0);
            REQUIRE(static_cast<int>(my_object.high_bound) == 2000000);
            const symbol dir = my_object.direction;
            REQUIRE(std::string(dir.c_str()) == "up");
            REQUIRE(static_cast<int>(my_object.init_value) == 0);
        }
    }
}

SCENARIO("tap.counter~ advances once per rising edge from zero") {
    ext_main(nullptr);

    GIVEN("a default instance counting up") {
        test_wrapper<counter> an_instance;
        counter&              my_object = an_instance;

        THEN("each zero->non-zero transition increments the stored count") {
            // m_last starts at 0.0; first non-zero is a rising edge -> 1.
            REQUIRE(static_cast<double>(my_object(1.0)) == 1.0);
            // Staying non-zero is not a new edge -> still 1.
            REQUIRE(static_cast<double>(my_object(1.0)) == 1.0);
            // Drop to zero (no count).
            REQUIRE(static_cast<double>(my_object(0.0)) == 1.0);
            // Rising edge again -> 2.
            REQUIRE(static_cast<double>(my_object(1.0)) == 2.0);
            REQUIRE(static_cast<double>(my_object(0.0)) == 2.0);
            REQUIRE(static_cast<double>(my_object(0.5)) == 3.0);
        }
    }
}

SCENARIO("tap.counter~ counting up wraps to low_bound past high_bound") {
    ext_main(nullptr);

    GIVEN("an instance with low_bound 0 and high_bound 2") {
        test_wrapper<counter> an_instance;
        counter&              my_object = an_instance;
        my_object.low_bound             = 0;
        my_object.high_bound            = 2;

        THEN("the count wraps from above high_bound back to low_bound") {
            REQUIRE(static_cast<double>(my_object(1.0)) == 1.0); // edge -> 1
            REQUIRE(static_cast<double>(my_object(0.0)) == 1.0);
            REQUIRE(static_cast<double>(my_object(1.0)) == 2.0); // edge -> 2
            REQUIRE(static_cast<double>(my_object(0.0)) == 2.0);
            // edge -> 3, which exceeds high_bound (2) -> wraps to low_bound (0).
            REQUIRE(static_cast<double>(my_object(1.0)) == 0.0);
        }
    }
}

SCENARIO("tap.counter~ counting down wraps to high_bound below low_bound") {
    ext_main(nullptr);

    GIVEN("an instance counting down with low_bound 0, high_bound 3, init_value 1") {
        test_wrapper<counter> an_instance;
        counter&              my_object = an_instance;
        my_object.low_bound             = 0;
        my_object.high_bound            = 3;
        my_object.direction             = symbol("down");
        my_object.init_value            = 1; // setter stores 1 into m_stored

        THEN("the count decrements and wraps below low_bound to high_bound") {
            // m_stored starts at 1 (init_value setter).
            REQUIRE(static_cast<double>(my_object(1.0)) == 0.0); // edge -> 0
            REQUIRE(static_cast<double>(my_object(0.0)) == 0.0);
            // edge -> -1, below low_bound (0) -> wraps to high_bound (3).
            REQUIRE(static_cast<double>(my_object(1.0)) == 3.0);
        }
    }
}

SCENARIO("tap.counter~ set message sets the current count without an edge") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<counter> an_instance;
        counter&              my_object = an_instance;

        WHEN("set 100 is sent") {
            my_object.set(atoms{100}, 0);

            THEN("the stored value becomes 100 and a rising edge advances to 101") {
                // No edge yet: m_last is still 0, but operator() only counts on a transition.
                // First call here is a rising edge (m_last 0 -> 1) so it increments from 100.
                REQUIRE(static_cast<double>(my_object(1.0)) == 101.0);
            }
        }
    }
}

SCENARIO("tap.counter~ reset restores init_value") {
    ext_main(nullptr);

    GIVEN("an instance with init_value 7 that has counted up") {
        test_wrapper<counter> an_instance;
        counter&              my_object = an_instance;
        my_object.init_value            = 7;

        my_object(1.0); // edge -> 8
        my_object(0.0);
        my_object(1.0); // edge -> 9

        WHEN("reset is sent") {
            my_object.reset();

            THEN("the stored count returns to init_value (7)") {
                // m_last is currently non-zero (1.0 from last call), so a steady non-zero input
                // is not a new edge -> the count reads back as the reset value, 7.
                REQUIRE(static_cast<double>(my_object(1.0)) == 7.0);
            }
        }
    }
}
