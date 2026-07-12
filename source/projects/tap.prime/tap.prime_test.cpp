/// @file
/// @brief      Unit tests for tap.prime.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 1999-2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.prime.cpp"      // include the object source so we can instantiate it

using namespace c74;

// Helper: read the single most-recent int value sent out an outlet's log.
static long last_int(max::t_sequence* seq) {
    REQUIRE(!seq->empty());
    const auto& msg = seq->back(); // [sym("int"), value]
    return static_cast<long>(msg[1]);
}

SCENARIO("tap.prime steps through the prime sequence on bang") {
    ext_main(nullptr);

    GIVEN("a default instance (initial value 0)") {
        test_wrapper<prime> an_instance;
        prime&              my_object = an_instance;

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        THEN("successive bangs produce 2, 3, 5, 7, 11, 13") {
            const long expected[] = {2, 3, 5, 7, 11, 13};
            for (long e : expected) {
                output->clear();
                my_object.bang();
                REQUIRE(last_int(output) == e);
            }
        }
    }
}

SCENARIO("tap.prime returns the next prime strictly greater than an int input") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<prime> an_instance;
        prime&              my_object = an_instance;

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        THEN("the next prime above various inputs is correct") {
            // strictly greater: next_prime(value) > value
            struct {
                long in;
                long out;
            } cases[] = {
                {0, 2}, {1, 2}, {2, 3}, {3, 5}, {4, 5}, {5, 7}, {7, 11}, {10, 11}, {13, 17}, {100, 101},
            };
            for (auto& c : cases) {
                output->clear();
                my_object.number(atoms{c.in}, 0);
                INFO("input " << c.in);
                REQUIRE(last_int(output) == c.out);
            }
        }
    }
}

SCENARIO("tap.prime right inlet sets the value to step from without output") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<prime> an_instance;
        prime&              my_object = an_instance;

        auto* output = max::object_getoutput(my_object.maxobj(), 0);

        WHEN("an int is sent to the right inlet") {
            output->clear();
            my_object.number(atoms{100}, 1);

            THEN("nothing is output") {
                REQUIRE(output->empty());
            }

            AND_THEN("a subsequent bang steps from that value") {
                output->clear();
                my_object.bang(); // next prime after 100 is 101
                REQUIRE(last_int(output) == 101);
            }
        }
    }
}
