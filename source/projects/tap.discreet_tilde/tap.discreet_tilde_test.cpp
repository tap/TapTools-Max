/// @file
/// @brief      Unit tests for tap.discreet~ (the Discreet Music two-machine tape loop).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/discreet_test.cpp)
///             carries the DSP promises. Here: documented defaults, attribute clamps forwarding
///             into the kernel, the bit-exact first return through the wrapper with the transport
///             parked, and clear as the eject button. The kernel slews over 20 ms by default, so
///             scenarios zero the smooth attribute or settle before measuring.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"     // required unit-test header (defines main via Catch)
#include "tap.discreet_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    long samples_for(double seconds) {
        return static_cast<long>(seconds * k_mock_sr);
    }

} // namespace

SCENARIO("tap.discreet~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<discreet> an_instance;
        discreet&              my_object = an_instance;

        THEN("a 30 s tape, a 5 s loop, regen 0, the stock wear and transport, mix 50") {
            REQUIRE(static_cast<double>(my_object.maxloop) == 30.0);
            REQUIRE(static_cast<double>(my_object.loop) == 5.0);
            REQUIRE(static_cast<double>(my_object.regen) == 0.0);
            REQUIRE(static_cast<double>(my_object.darken) == 3000.0);
            REQUIRE(static_cast<double>(my_object.drive) == 0.5);
            REQUIRE(static_cast<double>(my_object.input) == 1.0);
            REQUIRE(static_cast<double>(my_object.mix) == 50.0);
            const std::vector<number>& wow = my_object.wow;
            REQUIRE(wow.size() == 2);
            REQUIRE(wow[0] == 1.0);
            REQUIRE(wow[1] == 0.8);
        }
    }
}

SCENARIO("tap.discreet~ clamps regen to unity — and unity is legal") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<discreet> an_instance;
        discreet&              my_object = an_instance;

        THEN("a regen past 1 reports back as exactly 1 (the sustaining regime, not an error)") {
            my_object.regen = 1.5;
            REQUIRE(static_cast<double>(my_object.regen) == 1.0);
        }
        THEN("a negative regen floors at 0") {
            my_object.regen = -0.3;
            REQUIRE(static_cast<double>(my_object.regen) == 0.0);
        }
    }
}

SCENARIO("tap.discreet~ returns an impulse bit-exactly one loop later with the transport parked") {
    ext_main(nullptr);

    GIVEN("a 0.5 s loop, wet-only, wow and flutter off, instant setters") {
        test_wrapper<discreet> an_instance;
        discreet&              my_object = an_instance;
        my_object.smooth                 = 0.0;
        my_object.wow                    = atoms{0.0, 0.0};
        my_object.flutter                = atoms{0.0, 0.0};
        my_object.loop                   = 0.5;
        my_object.mix                    = 100.0;
        my_object.regen                  = 0.0;

        WHEN("an impulse is processed") {
            const long          at = samples_for(0.5);
            std::vector<double> y(static_cast<size_t>(at) + 8, 0.0);
            for (size_t i = 0; i < y.size(); ++i) {
                y[i] = my_object(i == 0 ? 1.0 : 0.0);
            }

            THEN("the first return is the recorded impulse itself, bit for bit") {
                REQUIRE(y[static_cast<size_t>(at)] == 1.0);
            }
        }
    }
}

SCENARIO("tap.discreet~ mix 0 is bitwise dry") {
    ext_main(nullptr);

    GIVEN("mix 0 with the loop otherwise live") {
        test_wrapper<discreet> an_instance;
        discreet&              my_object = an_instance;
        my_object.smooth                 = 0.0;
        my_object.mix                    = 0.0;
        my_object.regen                  = 0.9;

        THEN("the output is the input, bitwise") {
            bool exact = true;
            for (int i = 0; i < 512; ++i) {
                const double x = 0.001 * static_cast<double>(i % 100) - 0.05;
                exact          = exact && (my_object(x) == x);
            }
            REQUIRE(exact);
        }
    }
}

SCENARIO("tap.discreet~ clear is the eject button") {
    ext_main(nullptr);

    GIVEN("a short loop holding an impulse") {
        test_wrapper<discreet> an_instance;
        discreet&              my_object = an_instance;
        my_object.smooth                 = 0.0;
        my_object.wow                    = atoms{0.0, 0.0};
        my_object.flutter                = atoms{0.0, 0.0};
        my_object.loop                   = 0.2;
        my_object.mix                    = 100.0;
        my_object(1.0);

        WHEN("clear is sent before the return comes due") {
            my_object.clear();

            THEN("the tape is silent") {
                bool silent = true;
                for (long i = 0; i < samples_for(0.4); ++i) {
                    silent = silent && (my_object(0.0) == 0.0);
                }
                REQUIRE(silent);
            }
        }
    }
}
