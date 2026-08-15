/// @file
/// @brief      Unit tests for tap.period.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.period.cpp"     // include the object source so we can instantiate it

SCENARIO("tap.period defaults its rate to the global one") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<lcmperiod> an_instance;
        lcmperiod&              my_object = an_instance;

        THEN("the sample rate follows the host") { // the mock kernel's sys_getsr() is 44100
            REQUIRE(static_cast<double>(my_object.sr) == 44100.0);
        }
        THEN("a nonsensical rate falls back rather than dividing by it") {
            my_object.sr = 0.0;
            REQUIRE(static_cast<double>(my_object.sr) > 0.0);
            my_object.sr = -48000.0;
            REQUIRE(static_cast<double>(my_object.sr) > 0.0);
        }
    }
}

SCENARIO("tap.period is the same arithmetic the bank reports for its own lanes") {
    ext_main(nullptr);

    GIVEN("an instance at 48 kHz") {
        test_wrapper<lcmperiod> an_instance;
        lcmperiod&              my_object = an_instance;
        my_object.sr                      = 48000.0;

        THEN("two lengths whose sample counts share a factor realign at their lcm") {
            // 0.5 s = 24000 samples, 0.625 s = 30000; gcd 6000, so lcm 120000 = 2.5 s.
            const long pair[2] = {24000, 30000};
            REQUIRE(kernel::composite_period_seconds(pair, 2, 48000.0) == 2.5);
        }
        THEN("a length is quantized to samples exactly as tap.reel~ quantizes it") {
            // The floor and the rounding are the shared kernel function, not a copy here.
            REQUIRE(kernel::loop_samples_for(0.5, 48000.0) == 24000);
            REQUIRE(kernel::loop_samples_for(0.01, 48000.0)
                    == kernel::loop_samples_for(kernel::k_min_loop_seconds, 48000.0));
        }
        THEN("airport-scale lengths in awkward ratios leave the 64-bit range, which is the point") {
            std::vector<long> lengths;
            for (const double s : {17.8, 19.1, 21.3, 23.9, 26.2, 28.7, 30.9}) {
                lengths.push_back(kernel::loop_samples_for(s, 48000.0));
            }
            const double p =
                kernel::composite_period_seconds(lengths.data(), static_cast<int>(lengths.size()), 48000.0);
            REQUIRE(std::isinf(p));
        }
    }
}
