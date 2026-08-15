/// @file
/// @brief      Unit tests for tap.chime~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"  // required unit-test header (defines main via Catch)
#include "tap.chime_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.chime~ instantiates with the kernel's defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<chime> an_instance;
        chime&              my_object = an_instance;

        THEN("it is a chime rack, not a bar rack, at the kernel's default width") {
            REQUIRE(static_cast<int>(my_object.material) == kernel::material_chime);
            REQUIRE(static_cast<double>(my_object.spread) == kernel::k_default_spread);
        }
        THEN("the strike is a clapper, not a bow") {
            REQUIRE(static_cast<double>(my_object.attack) == kernel::k_default_attack_s);
            REQUIRE(static_cast<double>(my_object.decay) == kernel::k_default_decay_s);
        }
    }
}

SCENARIO("tap.chime~ clamps its parameters to the kernel's ranges") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<chime> an_instance;
        chime&              my_object = an_instance;

        THEN("material is held to the tables the kernel actually has") {
            my_object.material = 99;
            REQUIRE(static_cast<int>(my_object.material) == kernel::k_num_materials - 1);
            my_object.material = -5;
            REQUIRE(static_cast<int>(my_object.material) == 0);
        }
        THEN("spread is a 0..1 width") {
            my_object.spread = 4.0;
            REQUIRE(static_cast<double>(my_object.spread) == 1.0);
            my_object.spread = -4.0;
            REQUIRE(static_cast<double>(my_object.spread) == 0.0);
        }
        THEN("envelope times stay strictly positive, as decay_env requires") {
            my_object.attack = 0.0;
            REQUIRE(static_cast<double>(my_object.attack) > 0.0);
            my_object.decay = -1.0;
            REQUIRE(static_cast<double>(my_object.decay) > 0.0);
        }
    }
}
