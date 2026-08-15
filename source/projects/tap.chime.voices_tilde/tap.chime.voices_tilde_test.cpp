/// @file
/// @brief      Unit tests for tap.chime.voices~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"         // required unit-test header (defines main via Catch)
#include "tap.chime.voices_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.chime.voices~ instantiates with the rack's defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<chimevoices> an_instance;
        chimevoices&              my_object = an_instance;

        THEN("it is the same rack tap.chime~ wraps, at the same settings") {
            REQUIRE(static_cast<int>(my_object.material) == kernel::material_chime);
            REQUIRE(static_cast<double>(my_object.spread) == kernel::k_default_spread);
            REQUIRE(static_cast<double>(my_object.attack) == kernel::k_default_attack_s);
            REQUIRE(static_cast<double>(my_object.decay) == kernel::k_default_decay_s);
        }
        THEN("there is one signal outlet per bell in the pool") {
            REQUIRE(my_object.outlets().size() == static_cast<size_t>(kernel::k_voices) + 1); // + the report outlet
        }
    }
}

SCENARIO("tap.chime.voices~ clamps what the rack clamps") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<chimevoices> an_instance;
        chimevoices&              my_object = an_instance;

        THEN("material is held to the tables the kernel actually has") {
            my_object.material = 99;
            REQUIRE(static_cast<int>(my_object.material) == kernel::k_num_materials - 1);
        }
        THEN("spread is a 0..1 width even though these outlets are always dry") {
            my_object.spread = 4.0;
            REQUIRE(static_cast<double>(my_object.spread) == 1.0);
        }
        THEN("envelope times stay strictly positive, as decay_env requires") {
            my_object.attack = 0.0;
            REQUIRE(static_cast<double>(my_object.attack) > 0.0);
        }
    }
}
