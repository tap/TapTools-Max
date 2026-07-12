/// @file
/// @brief      Unit tests for tap.vocoder~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.vocoder_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.vocoder~ has the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<vocoder> an_instance;
        vocoder&              my_object = an_instance;

        THEN("q defaults to 20, response_interval to 20 ms, gain to 1") {
            REQUIRE(static_cast<double>(my_object.q) == 20.0);
            REQUIRE(static_cast<double>(my_object.response_interval) == 20.0);
            REQUIRE(static_cast<double>(my_object.gain) == 1.0);
        }
    }
}

SCENARIO("tap.vocoder~ produces no output without a modulator") {
    ext_main(nullptr);

    GIVEN("a default instance fed a carrier tone but a silent modulator") {
        test_wrapper<vocoder> an_instance;
        vocoder&              my_object = an_instance;

        double peak = 0.0;
        for (int n = 0; n < 4000; ++n) {
            const double carrier = std::sin(2.0 * M_PI * 1000.0 * n / 44100.0);
            const double out     = my_object(0.0, carrier); // modulator silent
            if (n > 2000) {
                peak = std::max(peak, std::fabs(out));
            }
        }

        THEN("the output stays effectively silent (envelopes never open)") {
            REQUIRE(peak < 1e-9);
        }
    }
}

SCENARIO("tap.vocoder~ passes the carrier where the modulator has energy") {
    ext_main(nullptr);

    GIVEN("a 1 kHz tone driving both the modulator and the carrier") {
        test_wrapper<vocoder> an_instance;
        vocoder&              my_object = an_instance;

        double peak = 0.0;
        for (int n = 0; n < 8000; ++n) {
            const double tone = std::sin(2.0 * M_PI * 1000.0 * n / 44100.0);
            const double out  = my_object(tone, tone);
            if (n > 4000) { // after the envelopes settle
                peak = std::max(peak, std::fabs(out));
            }
        }

        THEN("the matching band opens and a non-trivial signal passes") {
            REQUIRE(peak > 1e-3);
        }
    }
}
