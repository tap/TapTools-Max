/// @file
/// @brief      Unit tests for tap.zerox~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2008-2026 Timothy Place.

#include "c74_min_unittest.h"  // required unit-test header (defines main via Catch)
#include "tap.zerox_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.zerox~ instantiates with the expected defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<zerox> an_instance;
        zerox&              my_object = an_instance;

        THEN("size defaults to 2000 samples") {
            REQUIRE(static_cast<int>(my_object.size) == 2000);
        }
    }
}

SCENARIO("tap.zerox~ detects zero crossings on the trigger outlet") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<zerox> an_instance;
        zerox&              my_object = an_instance;

        THEN("a sign change produces a trigger of 1, no change produces 0") {
            // The internal state starts with last_over_zero == false.
            // First positive sample: false -> true is a crossing.
            auto s0 = my_object(1.0);
            REQUIRE(static_cast<double>(s0[1]) == APPROX(1.0));

            // Still positive: true -> true, no crossing.
            auto s1 = my_object(0.5);
            REQUIRE(static_cast<double>(s1[1]) == APPROX(0.0));

            // Now negative: true -> false, a crossing.
            auto s2 = my_object(-0.5);
            REQUIRE(static_cast<double>(s2[1]) == APPROX(1.0));

            // Still non-positive: false -> false, no crossing. (0.0 is NOT over zero.)
            auto s3 = my_object(0.0);
            REQUIRE(static_cast<double>(s3[1]) == APPROX(0.0));

            // Back to positive: false -> true, a crossing.
            auto s4 = my_object(0.25);
            REQUIRE(static_cast<double>(s4[1]) == APPROX(1.0));
        }
    }
}

SCENARIO("tap.zerox~ reports the normalized crossing count at the end of a window") {
    ext_main(nullptr);

    GIVEN("an instance with a small window of 4 samples") {
        test_wrapper<zerox> an_instance;
        zerox&              my_object = an_instance;
        my_object.size                = 4;

        THEN("the count outlet stays at 0 until the window fills, then reports count/size") {
            // Window size 4. Feed an alternating +,-,+,- sequence.
            // Crossings: sample1 (false->true)=1, sample2 (true->false)=1,
            //            sample3 (false->true)=1, sample4 (true->false)=1  => 4 crossings.
            // m_final updates only when m_location reaches size (after the 4th sample).
            auto a = my_object(1.0);
            REQUIRE(static_cast<double>(a[0]) == APPROX(0.0)); // not yet reported
            auto b = my_object(-1.0);
            REQUIRE(static_cast<double>(b[0]) == APPROX(0.0));
            auto c = my_object(1.0);
            REQUIRE(static_cast<double>(c[0]) == APPROX(0.0));
            auto d = my_object(-1.0);
            // window full: 4 crossings / size 4 = 1.0
            REQUIRE(static_cast<double>(d[0]) == APPROX(1.0));
        }
    }
}

SCENARIO("tap.zerox~ clear resets the analysis state") {
    ext_main(nullptr);

    GIVEN("an instance with a 2-sample window that has reported a value") {
        test_wrapper<zerox> an_instance;
        zerox&              my_object = an_instance;
        my_object.size                = 2;

        my_object(1.0);           // crossing
        auto r = my_object(-1.0); // window full (2 samples): 2 crossings / 2 = 1.0
        REQUIRE(static_cast<double>(r[0]) == APPROX(1.0));

        WHEN("clear is sent") {
            my_object.clear();

            THEN("the reported count returns to 0 and counting restarts") {
                // After clear m_final == 0 and last_over_zero == false again.
                // One sample is not enough to fill the window, so count stays 0.
                auto s = my_object(0.5);
                REQUIRE(static_cast<double>(s[0]) == APPROX(0.0));
                // but the first positive sample is still a crossing.
                REQUIRE(static_cast<double>(s[1]) == APPROX(1.0));
            }
        }
    }
}
