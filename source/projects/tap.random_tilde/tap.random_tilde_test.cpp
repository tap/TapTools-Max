/// @file
/// @brief      Unit tests for tap.random~.
/// @details    The generator draws from std::rand(), which is shared process-wide, so the *values*
///             are not predictable across scenarios. What is predictable — and what the port's
///             @note is about — is the sample-and-hold contract: one fresh value per zero-to-non-zero
///             edge, held in between, always inside the bounds. Collapsing the bounds to a single
///             value makes even the value deterministic.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <vector>

#include "c74_min_unittest.h"   // required unit-test header (defines main via Catch)
#include "tap.random_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.random~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<random_generator> an_instance;
        random_generator&              my_object = an_instance;

        THEN("the bounds are -1 and 1") {
            REQUIRE(static_cast<double>(my_object.low_bound) == -1.0);
            REQUIRE(static_cast<double>(my_object.high_bound) == 1.0);
        }
        THEN("a value is already held before any trigger arrives") {
            const double held = my_object(0.0);
            REQUIRE(held >= -1.0);
            REQUIRE(held <= 1.0);
        }
    }
}

SCENARIO("tap.random~ generates a fresh value on each zero-to-non-zero edge") {
    ext_main(nullptr);

    GIVEN("a default instance settled at zero input") {
        test_wrapper<random_generator> an_instance;
        random_generator&              my_object = an_instance;
        my_object(0.0);

        WHEN("the trigger rises") {
            const double first = my_object(1.0);

            THEN("the value is held while the trigger stays high") {
                for (int i = 0; i < 64; ++i) {
                    REQUIRE(my_object(1.0) == first);
                }
            }
            THEN("dropping the trigger does not change the held value") {
                REQUIRE(my_object(0.0) == first);
                REQUIRE(my_object(0.0) == first);
            }
            THEN("the next rising edge draws again") {
                my_object(0.0);
                // std::rand() is a shared sequence, so a repeat is possible but vanishingly rare;
                // drawing many times and requiring *some* change is the robust form.
                bool changed = false;
                for (int i = 0; i < 32 && !changed; ++i) {
                    my_object(0.0);
                    if (my_object(1.0) != first) {
                        changed = true;
                    }
                }
                REQUIRE(changed);
            }
        }
    }
}

SCENARIO("tap.random~ detects an edge per sample, not per block") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<random_generator> an_instance;
        random_generator&              my_object = an_instance;

        WHEN("the trigger alternates every sample inside one block") {
            // The @note in the header: the original tested for the edge once per vector, so mid-block
            // transitions were missed. Every 0 -> 1 pair here must produce its own value.
            std::vector<double> values;
            for (int i = 0; i < 64; ++i) {
                my_object(0.0);
                values.push_back(my_object(1.0));
            }

            THEN("64 edges yield a sequence that is not one held value") {
                bool all_same = true;
                for (double v : values) {
                    if (v != values.front()) {
                        all_same = false;
                    }
                }
                REQUIRE(!all_same);
            }
        }
    }
}

SCENARIO("tap.random~ keeps its values inside the configured bounds") {
    ext_main(nullptr);

    GIVEN("bounds of 0 and 10") {
        test_wrapper<random_generator> an_instance;
        random_generator&              my_object = an_instance;
        my_object.low_bound                      = 0.0;
        my_object.high_bound                     = 10.0;

        THEN("every generated value lands in range") {
            for (int i = 0; i < 4096; ++i) {
                my_object(0.0);
                const double v = my_object(1.0);
                REQUIRE(v >= 0.0);
                REQUIRE(v < 10.0); // the generator's t is in [0, 1)
            }
        }
    }

    GIVEN("a degenerate range where both bounds are the same") {
        test_wrapper<random_generator> an_instance;
        random_generator&              my_object = an_instance;
        my_object.low_bound                      = 0.5;
        my_object.high_bound                     = 0.5;

        THEN("the generator yields exactly that value") {
            for (int i = 0; i < 256; ++i) {
                my_object(0.0);
                REQUIRE(my_object(1.0) == 0.5);
            }
        }
    }

    GIVEN("an inverted range") {
        test_wrapper<random_generator> an_instance;
        random_generator&              my_object = an_instance;
        my_object.low_bound                      = 1.0;
        my_object.high_bound                     = -1.0;

        THEN("values still fall between the two bounds — the order does not matter") {
            for (int i = 0; i < 1024; ++i) {
                my_object(0.0);
                const double v = my_object(1.0);
                REQUIRE(v <= 1.0);
                REQUIRE(v > -1.0);
            }
        }
    }
}

SCENARIO("tap.random~ treats a negative trigger as non-zero") {
    ext_main(nullptr);

    GIVEN("a default instance settled at zero") {
        test_wrapper<random_generator> an_instance;
        random_generator&              my_object = an_instance;
        my_object.low_bound                      = 0.25;
        my_object.high_bound                     = 0.25;
        my_object(0.0);

        THEN("a fall to a negative value is still a trigger edge") {
            REQUIRE(my_object(-1.0) == 0.25);
        }
    }
}
