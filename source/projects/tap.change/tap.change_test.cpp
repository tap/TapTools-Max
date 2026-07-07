/// @file
/// @brief      Unit tests for tap.change.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.change.cpp"     // include the object source so we can instantiate it

using namespace c74;

SCENARIO("tap.change passes the very first input through (default state)") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<change> an_instance;
        change&              my_object = an_instance;

        auto* main_out = max::object_getoutput(my_object.maxobj(), 0);
        auto* bang_out = max::object_getoutput(my_object.maxobj(), 1);

        WHEN("the first int arrives") {
            main_out->clear();
            bang_out->clear();
            my_object.m_int(atoms{5}, 0);

            THEN("it is passed out the main outlet, with no bang") {
                REQUIRE(main_out->size() == 1);
                REQUIRE(static_cast<long>((*main_out)[0][1]) == 5);
                REQUIRE(bang_out->empty());
            }
        }
    }
}

SCENARIO("tap.change filters out a repeated value") {
    ext_main(nullptr);

    GIVEN("a default instance that has already seen the value 5") {
        test_wrapper<change> an_instance;
        change&              my_object = an_instance;

        auto* main_out = max::object_getoutput(my_object.maxobj(), 0);
        auto* bang_out = max::object_getoutput(my_object.maxobj(), 1);

        my_object.m_int(atoms{5}, 0); // primes the stored value

        WHEN("the same value 5 arrives again") {
            main_out->clear();
            bang_out->clear();
            my_object.m_int(atoms{5}, 0);

            THEN("a bang is emitted instead of passing the value through") {
                REQUIRE(main_out->empty());
                REQUIRE(bang_out->size() == 1);
            }
        }

        WHEN("a different value 6 arrives") {
            main_out->clear();
            bang_out->clear();
            my_object.m_int(atoms{6}, 0);

            THEN("it is passed through the main outlet") {
                REQUIRE(main_out->size() == 1);
                REQUIRE(static_cast<long>((*main_out)[0][1]) == 6);
                REQUIRE(bang_out->empty());
            }
        }
    }
}

SCENARIO("tap.change treats a type difference as a change") {
    ext_main(nullptr);

    GIVEN("a default instance that has seen the int 5") {
        test_wrapper<change> an_instance;
        change&              my_object = an_instance;

        auto* main_out = max::object_getoutput(my_object.maxobj(), 0);
        auto* bang_out = max::object_getoutput(my_object.maxobj(), 1);

        my_object.m_int(atoms{5}, 0);

        WHEN("the float 5.0 arrives (same numeric value, different type)") {
            main_out->clear();
            bang_out->clear();
            my_object.m_float(atoms{5.0}, 0);

            THEN("it is treated as novel and passed through") {
                REQUIRE(main_out->size() == 1);
                REQUIRE(bang_out->empty());
            }
        }
    }
}

SCENARIO("tap.change right inlet sets the stored value silently") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<change> an_instance;
        change&              my_object = an_instance;

        auto* main_out = max::object_getoutput(my_object.maxobj(), 0);
        auto* bang_out = max::object_getoutput(my_object.maxobj(), 1);

        WHEN("a value is set via the right inlet (inlet 1)") {
            main_out->clear();
            bang_out->clear();
            my_object.m_int(atoms{9}, 1);

            THEN("nothing is output") {
                REQUIRE(main_out->empty());
                REQUIRE(bang_out->empty());
            }

            AND_THEN("a matching value in the left inlet is now filtered to a bang") {
                main_out->clear();
                bang_out->clear();
                my_object.m_int(atoms{9}, 0);
                REQUIRE(main_out->empty());
                REQUIRE(bang_out->size() == 1);
            }
        }
    }
}
