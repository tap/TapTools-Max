/// @file
/// @brief      Unit tests for tap.sieve.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2000-2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.sieve.cpp"      // include the object source so we can instantiate it

using namespace c74;

SCENARIO("tap.sieve instantiates with the expected defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<sieve> an_instance;
        sieve&              my_object = an_instance;

        THEN("the value to match defaults to 0") {
            REQUIRE(static_cast<int>(my_object.value) == 0);
        }
    }
}

SCENARIO("tap.sieve passes a matching value and blocks a non-matching one") {
    ext_main(nullptr);

    GIVEN("an instance set to match the value 5") {
        test_wrapper<sieve> an_instance;
        sieve&              my_object = an_instance;
        my_object.value               = 5;

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("a matching int 5 is received in the left inlet") {
            output->clear();
            my_object.number(atoms{5}, 0);

            THEN("it is passed to the outlet") {
                REQUIRE(output->size() == 1);
                REQUIRE(static_cast<long>((*output)[0][1]) == 5);
            }
        }

        WHEN("a non-matching int 6 is received in the left inlet") {
            output->clear();
            my_object.number(atoms{6}, 0);

            THEN("nothing is output") {
                REQUIRE(output->empty());
            }
        }

        WHEN("a value is sent to the right inlet") {
            output->clear();
            my_object.number(atoms{42}, 1);

            THEN("the match value is updated and nothing is output") {
                REQUIRE(static_cast<int>(my_object.value) == 42);
                REQUIRE(output->empty());
            }

            AND_THEN("a subsequent matching input passes through") {
                output->clear();
                my_object.number(atoms{42}, 0);
                REQUIRE(output->size() == 1);
                REQUIRE(static_cast<long>((*output)[0][1]) == 42);
            }
        }
    }
}
