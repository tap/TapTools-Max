/// @file
/// @brief      Unit tests for tap.random.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <cstdint>
#include <cstdlib>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.random.cpp"     // include the object source so we can instantiate it

using namespace c74;

// Reproduce the object's transform on a given rand() result.
static double expected_from_rand(uint32_t r) {
    r           = r * 1103515245u + 12345u;
    const int i = static_cast<int>((r >> 16) & 077777);
    return static_cast<double>(i) / 16384.0 - 1.0;
}

static double last_float(max::t_sequence* seq) {
    REQUIRE(!seq->empty());
    const auto& msg = seq->back(); // [sym("float"), value]
    return static_cast<double>(msg[1]);
}

SCENARIO("tap.random emits a float in the documented range [-1, 1)") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<randomgen> an_instance;
        randomgen&              my_object = an_instance;

        auto* out = max::object_getoutput(my_object.maxobj(), 0);

        THEN("many bangs all produce values within [-1, 1)") {
            for (int n = 0; n < 1000; ++n) {
                out->clear();
                my_object.bang();
                const double v = last_float(out);
                REQUIRE(v >= -1.0);
                REQUIRE(v < 1.0);
            }
        }
    }
}

SCENARIO("tap.random is deterministic given a fixed std::srand seed") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<randomgen> an_instance;
        randomgen&              my_object = an_instance;

        auto* out = max::object_getoutput(my_object.maxobj(), 0);

        THEN("seeding the C RNG reproduces the exact LCG-derived output") {
            // The object pulls one std::rand() per bang and runs it through a fixed LCG, so
            // seeding std::srand makes the whole sequence reproducible and computable.
            std::srand(12345);
            const uint32_t r0 = static_cast<uint32_t>(std::rand());
            const uint32_t r1 = static_cast<uint32_t>(std::rand());

            std::srand(12345);
            out->clear();
            my_object.bang();
            REQUIRE(last_float(out) == APPROX(expected_from_rand(r0)));

            out->clear();
            my_object.bang();
            REQUIRE(last_float(out) == APPROX(expected_from_rand(r1)));
        }
    }
}

SCENARIO("tap.random re-seeding reproduces the same sequence") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<randomgen> an_instance;
        randomgen&              my_object = an_instance;

        auto* out = max::object_getoutput(my_object.maxobj(), 0);

        THEN("two runs from the same seed yield identical sequences") {
            double first[5];
            std::srand(999);
            for (int n = 0; n < 5; ++n) {
                out->clear();
                my_object.bang();
                first[n] = last_float(out);
            }

            std::srand(999);
            for (int n = 0; n < 5; ++n) {
                out->clear();
                my_object.bang();
                REQUIRE(last_float(out) == APPROX(first[n]));
            }
        }
    }
}
