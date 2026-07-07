/// @file
/// @brief      Unit tests for tap.radians~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.radians_tilde.cpp" // include the object source so we can instantiate it

// The mock kernel reports a sample rate of 44100, which is what samplerate() returns
// for a freshly constructed sample_operator in the test harness.
static constexpr double k_sr    = 44100.0;
static constexpr double k_pi    = 3.14159265358979323846;
static constexpr double k_twopi = 6.28318530717958647692;

SCENARIO("tap.radians~ defaults to hertz->radians mode") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<radians> an_instance;
        radians&              my_object = an_instance;

        THEN("mode defaults to 0 (hz->rad)") {
            REQUIRE(static_cast<int>(my_object.mode) == 0);
        }
        THEN("a hertz value is converted to radians: hz * pi / (sr/2)") {
            REQUIRE(static_cast<double>(my_object(1000.0)) == APPROX(1000.0 * (k_pi / (k_sr * 0.5))));
        }
        THEN("zero hertz converts to zero radians") {
            REQUIRE(static_cast<double>(my_object(0.0)) == APPROX(0.0));
        }
    }
}

SCENARIO("tap.radians~ mode 1 converts radians->hertz") {
    ext_main(nullptr);

    GIVEN("an instance in mode 1") {
        test_wrapper<radians> an_instance;
        radians&              my_object = an_instance;
        my_object.mode                  = 1;

        THEN("radians convert to hertz: rad * sr / 2pi") {
            REQUIRE(static_cast<double>(my_object(k_twopi)) == APPROX(k_sr));
            REQUIRE(static_cast<double>(my_object(k_pi)) == APPROX(k_sr / 2.0));
        }
        THEN("hz->rad then rad->hz round-trips back to the original frequency") {
            // Apply the documented hz->rad formula, then verify this object inverts it.
            const double rad = 440.0 * (k_pi / (k_sr * 0.5));
            REQUIRE(static_cast<double>(my_object(rad)) == APPROX(440.0));
        }
    }
}

SCENARIO("tap.radians~ mode 2 converts degrees->radians") {
    ext_main(nullptr);

    GIVEN("an instance in mode 2") {
        test_wrapper<radians> an_instance;
        radians&              my_object = an_instance;
        my_object.mode                  = 2;

        THEN("180 degrees == pi radians") {
            REQUIRE(static_cast<double>(my_object(180.0)) == APPROX(k_pi));
        }
        THEN("90 degrees == pi/2 radians") {
            REQUIRE(static_cast<double>(my_object(90.0)) == APPROX(k_pi / 2.0));
        }
        THEN("0 degrees == 0 radians") {
            REQUIRE(static_cast<double>(my_object(0.0)) == APPROX(0.0));
        }
    }
}

SCENARIO("tap.radians~ mode 3 converts radians->degrees") {
    ext_main(nullptr);

    GIVEN("an instance in mode 3") {
        test_wrapper<radians> an_instance;
        radians&              my_object = an_instance;
        my_object.mode                  = 3;

        THEN("pi radians == 180 degrees") {
            REQUIRE(static_cast<double>(my_object(k_pi)) == APPROX(180.0));
        }
        THEN("pi/2 radians == 90 degrees") {
            REQUIRE(static_cast<double>(my_object(k_pi / 2.0)) == APPROX(90.0));
        }
    }
}
