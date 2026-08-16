/// @file
/// @brief      Unit tests for tap.touche~ (the Ondes Martenot intensity key).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/touche_test.cpp)
///             carries the curve's fidelity to the published measurement. Here: the documented
///             defaults, the clamps, and the two behaviours a patcher would be surprised by if
///             they were wrong — a key at rest is exactly silent, and the bottom of the throw is
///             silent too (which looks like a bug until you know it is the instrument's).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h"   // required unit-test header (defines main via Catch)
#include "tap.touche_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.touche~ instantiates at rest") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<touche> an_instance;
        touche&              my_object = an_instance;

        THEN("the key starts up, silent, driven by displacement") {
            REQUIRE(static_cast<double>(my_object.position) == 0.0);
            REQUIRE(static_cast<int>(my_object.mode) == 0);
        }
    }
}

SCENARIO("tap.touche~ clamps position to the travel") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<touche> an_instance;
        touche&              my_object = an_instance;

        THEN("position is a fraction of the throw") {
            my_object.position = 4.0;
            REQUIRE(static_cast<double>(my_object.position) == 1.0);
            my_object.position = -1.0;
            REQUIRE(static_cast<double>(my_object.position) == 0.0);
        }
        THEN("mode only accepts the two measured columns") {
            my_object.mode = 7;
            REQUIRE(static_cast<int>(my_object.mode) == 0);
            my_object.mode = 1;
            REQUIRE(static_cast<int>(my_object.mode) == 1);
        }
    }
}

// The two things a patcher would otherwise file as bugs.
SCENARIO("tap.touche~ is exactly silent at rest, and through the dead zone") {
    ext_main(nullptr);

    GIVEN("a key at rest") {
        test_wrapper<touche> an_instance;
        touche&              my_object = an_instance;
        my_object.smooth               = 0.0;
        my_object.position             = 0.0;

        THEN("nothing comes out at all") {
            bool silent = true;
            for (int i = 0; i < 4000; ++i) {
                silent = silent && (my_object(0.7, 0.0) == 0.0);
            }
            REQUIRE(silent);
        }
    }

    GIVEN("the key pressed part-way, but not as far as the powder bag") {
        test_wrapper<touche> an_instance;
        touche&              my_object = an_instance;
        my_object.smooth               = 0.0;

        THEN("everything below 4.3 mm is still silent — the key is only bending") {
            for (double travel : {0.0, 1.0, 2.5, 4.0}) {
                my_object.mm = travel;
                INFO(travel << " mm");
                REQUIRE(my_object(0.7, 0.0) == 0.0);
            }
        }
        THEN("and just past it, sound arrives") {
            my_object.mm = 4.5;
            REQUIRE(std::abs(my_object(0.7, 0.0)) > 0.0);
        }
    }
}

SCENARIO("tap.touche~ reaches unity at full press") {
    ext_main(nullptr);

    GIVEN("the key fully pressed") {
        test_wrapper<touche> an_instance;
        touche&              my_object = an_instance;
        my_object.smooth               = 0.0;
        my_object.position             = 1.0;

        THEN("the input passes at unity") {
            REQUIRE(std::abs(my_object(0.5, 0.0) - 0.5) < 1e-9);
        }
    }
}
