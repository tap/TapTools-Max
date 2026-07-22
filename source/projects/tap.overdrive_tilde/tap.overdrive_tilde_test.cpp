/// @file
/// @brief      Unit tests for tap.overdrive~ (Min-level behavior; DSP correctness lives in the
///             kernel repo's overdrive_test.cpp).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2004-2026 Timothy Place.

#include "c74_min_unittest.h"      // required unit-test header (defines main via Catch)
#include "tap.overdrive_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.overdrive~ instantiates with the expected defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<overdrive> an_instance;
        overdrive&              my_object = an_instance;

        THEN("the attributes have the documented defaults") {
            REQUIRE(static_cast<double>(my_object.drive) == 0.35);
            REQUIRE(static_cast<double>(my_object.body) == 0.0);
            REQUIRE(static_cast<double>(my_object.asymmetry) == 0.15);
            REQUIRE(static_cast<double>(my_object.preamp) == 0.0);
            REQUIRE(static_cast<double>(my_object.output) == 0.0);
            REQUIRE(static_cast<int>(my_object.oversample) == 4);
            REQUIRE(static_cast<double>(my_object.smooth) == tap::tools::od::k_default_smooth_ms);
            REQUIRE(static_cast<bool>(my_object.bypass) == false);
            REQUIRE(static_cast<bool>(my_object.mute) == false);
        }
    }
}

SCENARIO("tap.overdrive~ clamps attribute values to their documented ranges") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<overdrive> an_instance;
        overdrive&              my_object = an_instance;

        WHEN("normalized attributes are set out of range") {
            my_object.drive     = 2.0;
            my_object.body      = -5.0;
            my_object.asymmetry = 1.5;

            THEN("they clamp to their ranges") {
                REQUIRE(static_cast<double>(my_object.drive) == 1.0);
                REQUIRE(static_cast<double>(my_object.body) == -1.0);
                REQUIRE(static_cast<double>(my_object.asymmetry) == 1.0);
            }
        }

        WHEN("the gain attributes are set out of range") {
            my_object.preamp = 100.0;
            my_object.output = -100.0;

            THEN("they clamp to +-24 dB") {
                REQUIRE(static_cast<double>(my_object.preamp) == tap::tools::od::k_gain_range_db);
                REQUIRE(static_cast<double>(my_object.output) == -tap::tools::od::k_gain_range_db);
            }
        }

        WHEN("the oversample factor is set to a non-power-of-two") {
            my_object.oversample = 3;

            THEN("it snaps down to a legal factor") {
                REQUIRE(static_cast<int>(my_object.oversample) == 2);
                REQUIRE(my_object.od().oversample() == 2);
            }
        }

        WHEN("the smooth time is set negative") {
            my_object.smooth = -5.0;

            THEN("it clamps to zero") {
                REQUIRE(static_cast<double>(my_object.smooth) == 0.0);
            }
        }
    }
}
