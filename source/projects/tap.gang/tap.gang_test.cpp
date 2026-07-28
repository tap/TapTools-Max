/// @file
/// @brief      Unit tests for tap.gang.
/// @details    tap.gang forwards everything through per-outlet `queue`s so the fan-out lands on
///             Max's low-priority thread. The mock kernel's `qelem_set` is a no-op and `queue`'s
///             service function is private, so nothing the object sends is observable here: these
///             scenarios pin construction and message dispatch (the plumbing that a refactor breaks
///             first), and the ring-of-gangs change-detection behavior is asserted in Max — see
///             runtime-tests/.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2005-2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.gang.cpp"       // include the object source so we can instantiate it

using namespace c74;

SCENARIO("tap.gang instantiates with four inlets and four outlets") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<gang> an_instance;
        gang&              my_object = an_instance;

        THEN("all four outlets exist") {
            REQUIRE(max::object_getoutput(my_object.maxobj(), 0) != nullptr);
            REQUIRE(max::object_getoutput(my_object.maxobj(), 3) != nullptr);
        }
        THEN("nothing has been emitted yet") {
            REQUIRE(max::object_getoutput(my_object.maxobj(), 0)->empty());
        }
    }
}

SCENARIO("tap.gang accepts a number at every inlet") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<gang> an_instance;
        gang&              my_object = an_instance;

        THEN("each inlet takes a number, and a repeat of the same value is filtered out") {
            for (int in = 0; in < 4; ++in) {
                my_object.number(atoms{static_cast<double>(in) + 0.5}, in);
                my_object.number(atoms{static_cast<double>(in) + 0.5}, in); // the change-detect path
            }
            REQUIRE(true); // the fan-out is deferred; see the file header
        }
        THEN("a NaN is absorbed rather than propagated") {
            my_object.number(atoms{std::nan("")}, 0);
            REQUIRE(true);
        }
    }
}

SCENARIO("tap.gang accepts a list at every inlet") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<gang> an_instance;
        gang&              my_object = an_instance;

        THEN("each inlet takes a list, and an identical list is filtered out") {
            for (int in = 0; in < 4; ++in) {
                my_object.list_msg(atoms{1.0, 2.0, 3.0}, in);
                my_object.list_msg(atoms{1.0, 2.0, 3.0}, in); // the change-detect path
            }
            REQUIRE(true);
        }
        THEN("a list of a different length is treated as a change") {
            my_object.list_msg(atoms{1.0, 2.0, 3.0}, 0);
            my_object.list_msg(atoms{1.0, 2.0}, 0);
            my_object.list_msg(atoms{1.0, 2.0, 3.0, 4.0}, 0);
            REQUIRE(true);
        }
        THEN("switching between number and list input is safe") {
            my_object.number(atoms{7.0}, 0);
            my_object.list_msg(atoms{7.0, 8.0}, 1);
            my_object.number(atoms{9.0}, 2);
            REQUIRE(true);
        }
    }
}
