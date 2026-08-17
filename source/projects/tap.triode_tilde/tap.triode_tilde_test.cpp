/// @file
/// @brief      Unit tests for tap.triode~ (one triode stage from a published tube model).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/ondes_test.cpp)
///             carries the tube model against its datasheet and the load-line solve. Here: the
///             documented defaults, the clamps, and the two properties a patcher would file as
///             bugs if they were wrong — the stage inverts, and turning drive up does not make it
///             louder.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h"   // required unit-test header (defines main via Catch)
#include "tap.triode_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.triode~ instantiates on the demodulator's published valve and operating point") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<triode> an_instance;
        triode&              my_object = an_instance;

        THEN("it is the 6C5 at the demodulator's point, as the header says") {
            REQUIRE(static_cast<int>(my_object.tube) == kernel::tube_6c5);
            REQUIRE(static_cast<int>(my_object.stage) == 0);
            REQUIRE(static_cast<double>(my_object.drive) == kernel::k_default_drive_v);
        }
    }
}

SCENARIO("tap.triode~ only accepts the valves and operating points that exist") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<triode> an_instance;
        triode&              my_object = an_instance;

        THEN("the tube index stays inside the published set") {
            my_object.tube = 9;
            REQUIRE(static_cast<int>(my_object.tube) == kernel::k_num_tubes - 1);
            my_object.tube = -2;
            REQUIRE(static_cast<int>(my_object.tube) == 0);
        }
        THEN("so does the stage") {
            my_object.stage = 9;
            REQUIRE(static_cast<int>(my_object.stage) == 2);
            my_object.stage = -1;
            REQUIRE(static_cast<int>(my_object.stage) == 0);
        }
        THEN("drive stays inside the grid swing the curve table covers") {
            my_object.drive = 1e6;
            REQUIRE(static_cast<double>(my_object.drive) == kernel::k_max_drive_v);
            my_object.drive = 0.0;
            REQUIRE(static_cast<double>(my_object.drive) == kernel::k_min_drive_v);
        }
    }
}

// The two things a patcher would otherwise file as bugs.
SCENARIO("tap.triode~ inverts, and drive does not make it louder") {
    ext_main(nullptr);

    GIVEN("a stage with its filters opened up so only the curve is in play") {
        test_wrapper<triode> an_instance;
        triode&              my_object = an_instance;
        my_object.highpass             = 0.1;
        my_object.lowpass              = 20000.0;

        THEN("a small positive input comes out negative") {
            // Settle the conditioning highpass on silence first, so the DC step is not measured.
            for (int i = 0; i < 200; ++i) {
                my_object(0.0);
            }
            const sample y = my_object(0.01);
            INFO("0.01 in gives " << y);
            REQUIRE(y < 0.0);
        }

        THEN("the small-signal gain is unity at every drive setting") {
            for (double d : {0.1, 1.0, 8.0}) {
                my_object.drive = d;
                my_object.clear();
                for (int i = 0; i < 400; ++i) {
                    my_object(0.0);
                }
                const double y = static_cast<double>(my_object(0.001));
                INFO("drive " << d << ": 0.001 in gives " << y);
                REQUIRE(std::abs(y / -0.001 - 1.0) < 0.1);
            }
        }
    }
}

SCENARIO("tap.triode~ is quiet on silence whichever valve it is running") {
    ext_main(nullptr);

    GIVEN("each published valve in turn") {
        test_wrapper<triode> an_instance;
        triode&              my_object = an_instance;

        for (int t = 0; t < kernel::k_num_tubes; ++t) {
            my_object.tube = t;
            my_object.clear();
            double worst = 0.0;
            for (int i = 0; i < 4800; ++i) {
                worst = std::max(worst, std::abs(static_cast<double>(my_object(0.0))));
            }
            INFO("tube " << t << ": worst output on silence " << worst);
            REQUIRE(worst < 1e-6);
        }
    }
}
