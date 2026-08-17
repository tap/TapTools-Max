/// @file
/// @brief      Unit tests for tap.metallique~ (the Ondes Martenot's gong diffuseur).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/diffuseur_test.cpp)
///             carries the modal maths, the boundedness argument and the transducer's law. Here:
///             the documented defaults, the clamps, and the two behaviours a patcher would be
///             surprised by if they were wrong — a fully dry cabinet is the input untouched, and
///             a driven one is not silent.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h"       // required unit-test header (defines main via Catch)
#include "tap.metallique_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.metallique~ instantiates with its documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<metallique> an_instance;
        metallique&              my_object = an_instance;

        THEN("the body and the driver are where the header says") {
            REQUIRE(static_cast<double>(my_object.pitch) == kernel::k_default_pitch_hz);
            REQUIRE(static_cast<double>(my_object.decay) == kernel::k_default_decay_s);
            REQUIRE(static_cast<double>(my_object.tilt) == kernel::k_default_tilt);
            REQUIRE(static_cast<double>(my_object.brightness) == kernel::k_default_bright);
            REQUIRE(static_cast<double>(my_object.drive) == kernel::k_default_drive);
            REQUIRE(static_cast<double>(my_object.asymmetry) == kernel::k_default_asymmetry);
            REQUIRE(static_cast<double>(my_object.saturation) == kernel::k_default_sat);
            REQUIRE(static_cast<double>(my_object.mix) == kernel::k_default_mix);
            REQUIRE(static_cast<double>(my_object.level) == kernel::k_default_level);
        }
    }
}

SCENARIO("tap.metallique~ clamps its body to the legal ranges") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<metallique> an_instance;
        metallique&              my_object = an_instance;

        THEN("pitch stays inside the audible band the modes were sized for") {
            my_object.pitch = 100000.0;
            REQUIRE(static_cast<double>(my_object.pitch) == kernel::k_max_pitch_hz);
            my_object.pitch = -5.0;
            REQUIRE(static_cast<double>(my_object.pitch) == kernel::k_min_pitch_hz);
        }
        THEN("decay stays inside the ring-time range") {
            my_object.decay = 1e6;
            REQUIRE(static_cast<double>(my_object.decay) == kernel::k_max_t60);
            my_object.decay = 0.0;
            REQUIRE(static_cast<double>(my_object.decay) == kernel::k_min_t60);
        }
        THEN("the normalized controls are fractions") {
            my_object.brightness = 4.0;
            REQUIRE(static_cast<double>(my_object.brightness) == 1.0);
            my_object.asymmetry = -1.0;
            REQUIRE(static_cast<double>(my_object.asymmetry) == 0.0);
            my_object.mix = 1000.0;
            REQUIRE(static_cast<double>(my_object.mix) == 100.0);
            my_object.tilt = 99.0;
            REQUIRE(static_cast<double>(my_object.tilt) == 3.0);
        }
        THEN("drive and saturation cannot go negative") {
            my_object.drive = -2.0;
            REQUIRE(static_cast<double>(my_object.drive) == 0.0);
            my_object.saturation = -1.0;
            REQUIRE(static_cast<double>(my_object.saturation) == 0.0);
        }
    }
}

// The two things a patcher would otherwise file as bugs.
SCENARIO("tap.metallique~ is a bitwise passthrough fully dry, and audible fully wet") {
    ext_main(nullptr);

    GIVEN("a cabinet with the balance hard dry") {
        test_wrapper<metallique> an_instance;
        metallique&              my_object = an_instance;
        my_object.smooth                   = 0.0;
        my_object.mix                      = 0.0;
        my_object.level                    = 1.0;

        THEN("what comes out is exactly what went in") {
            bool identical = true;
            for (int i = 0; i < 2000; ++i) {
                const sample x = 0.5 * std::sin(6.2831853071795865 * 220.0 * i / 48000.0);
                identical      = identical && (my_object(x) == x);
            }
            REQUIRE(identical);
        }
    }

    GIVEN("a cabinet with the balance hard wet") {
        test_wrapper<metallique> an_instance;
        metallique&              my_object = an_instance;
        my_object.smooth                   = 0.0;
        my_object.mix                      = 100.0;
        my_object.pitch                    = 220.0;
        my_object.decay                    = 4.0;

        THEN("the body speaks") {
            double energy = 0.0;
            for (int i = 0; i < 48000; ++i) {
                const sample x = 0.5 * std::sin(6.2831853071795865 * 220.0 * i / 48000.0);
                const sample y = my_object(x);
                if (i > 24000) {
                    energy += static_cast<double>(y) * static_cast<double>(y);
                }
            }
            REQUIRE(energy > 0.0);
        }
    }
}

SCENARIO("tap.metallique~ clears without disturbing its settings") {
    ext_main(nullptr);

    GIVEN("a rung cabinet") {
        test_wrapper<metallique> an_instance;
        metallique&              my_object = an_instance;
        my_object.pitch                    = 240.0;
        my_object.mix                      = 70.0;
        for (int i = 0; i < 4800; ++i) {
            my_object(0.5);
        }

        WHEN("it is cleared") {
            my_object.clear();
            THEN("the parameters are untouched") {
                REQUIRE(static_cast<double>(my_object.pitch) == 240.0);
                REQUIRE(static_cast<double>(my_object.mix) == 70.0);
            }
        }
    }
}
