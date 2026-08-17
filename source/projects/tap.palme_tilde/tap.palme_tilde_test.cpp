/// @file
/// @brief      Unit tests for tap.palme~ (the Ondes Martenot's string diffuseur).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/diffuseur_test.cpp)
///             carries the sympathetic selectivity and the ring-time calibration. Here: the
///             documented defaults, the clamps, the two tunings, and the passthrough contract.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h"  // required unit-test header (defines main via Catch)
#include "tap.palme_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.palme~ instantiates with its documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<palme> an_instance;
        palme&              my_object = an_instance;

        THEN("the board and the driver are where the header says") {
            REQUIRE(static_cast<double>(my_object.root) == kernel::k_default_root_hz);
            REQUIRE(static_cast<int>(my_object.tuning) == kernel::tuning_chromatic);
            REQUIRE(static_cast<double>(my_object.decay) == kernel::k_default_decay_s);
            REQUIRE(static_cast<double>(my_object.damping) == kernel::k_default_damp_hz);
            REQUIRE(static_cast<double>(my_object.detune) == kernel::k_default_detune_c);
            REQUIRE(static_cast<double>(my_object.mix) == kernel::k_default_mix);
        }
    }
}

SCENARIO("tap.palme~ clamps its board to the legal ranges") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<palme> an_instance;
        palme&              my_object = an_instance;

        THEN("the root stays inside the range the loops were bought for") {
            my_object.root = 100000.0;
            REQUIRE(static_cast<double>(my_object.root) == kernel::k_max_pitch_hz);
            my_object.root = 0.0;
            REQUIRE(static_cast<double>(my_object.root) == kernel::k_min_pitch_hz);
        }
        THEN("damping stays inside the audible band") {
            my_object.damping = 1e9;
            REQUIRE(static_cast<double>(my_object.damping) == kernel::k_max_damp_hz);
            my_object.damping = 1.0;
            REQUIRE(static_cast<double>(my_object.damping) == kernel::k_min_damp_hz);
        }
        THEN("tuning only accepts the layouts that exist") {
            my_object.tuning = 9;
            REQUIRE(static_cast<int>(my_object.tuning) == kernel::k_num_tunings - 1);
            my_object.tuning = -3;
            REQUIRE(static_cast<int>(my_object.tuning) == 0);
        }
        THEN("detune is bounded, so the board cannot be scattered into noise") {
            my_object.detune = 400.0;
            REQUIRE(static_cast<double>(my_object.detune) == 50.0);
            my_object.detune = -1.0;
            REQUIRE(static_cast<double>(my_object.detune) == 0.0);
        }
    }
}

SCENARIO("tap.palme~ is a bitwise passthrough fully dry, and rings fully wet") {
    ext_main(nullptr);

    GIVEN("a board with the balance hard dry") {
        test_wrapper<palme> an_instance;
        palme&              my_object = an_instance;
        my_object.smooth              = 0.0;
        my_object.mix                 = 0.0;
        my_object.level               = 1.0;

        THEN("what comes out is exactly what went in") {
            bool identical = true;
            for (int i = 0; i < 2000; ++i) {
                const sample x = 0.4 * std::sin(6.2831853071795865 * 330.0 * i / 48000.0);
                identical      = identical && (my_object(x) == x);
            }
            REQUIRE(identical);
        }
    }

    GIVEN("a board driven on one of its own strings, then left alone") {
        test_wrapper<palme> an_instance;
        palme&              my_object = an_instance;
        my_object.smooth              = 0.0;
        my_object.mix                 = 100.0;
        my_object.root                = 110.0;
        my_object.decay               = 6.0;
        my_object.detune              = 0.0;

        THEN("it is still sounding after the drive stops") {
            for (int i = 0; i < 48000; ++i) {
                my_object(0.3 * std::sin(6.2831853071795865 * 110.0 * i / 48000.0));
            }
            double energy = 0.0;
            for (int i = 0; i < 24000; ++i) {
                const sample y = my_object(0.0);
                energy += static_cast<double>(y) * static_cast<double>(y);
            }
            REQUIRE(energy > 0.0);
        }
    }
}

SCENARIO("tap.palme~ clears without disturbing its settings") {
    ext_main(nullptr);

    GIVEN("a ringing board") {
        test_wrapper<palme> an_instance;
        palme&              my_object = an_instance;
        my_object.root                = 150.0;
        my_object.tuning              = kernel::tuning_harmonic;
        for (int i = 0; i < 4800; ++i) {
            my_object(0.5);
        }

        WHEN("it is cleared") {
            my_object.clear();
            THEN("the parameters are untouched") {
                REQUIRE(static_cast<double>(my_object.root) == 150.0);
                REQUIRE(static_cast<int>(my_object.tuning) == kernel::tuning_harmonic);
            }
        }
    }
}
