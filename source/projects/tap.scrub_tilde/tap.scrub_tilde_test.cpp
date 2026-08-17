/// @file
/// @brief      Unit tests for tap.scrub~ (the granular scrub pad).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/scrub_test.cpp)
///             carries the delay null, the transposition, the freeze and the seed contract. Here:
///             the documented defaults, the clamps, the maxhistory argument, and the one thing a
///             patcher would file as a bug if it were wrong — held still at pitch 0 the object is
///             a plain delay, so it must not colour the signal at all.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"  // required unit-test header (defines main via Catch)
#include "tap.scrub_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.scrub~ instantiates with its documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<scrub> an_instance;
        scrub&              my_object = an_instance;

        THEN("the playhead and the grains are where the header says") {
            REQUIRE(static_cast<double>(my_object.position) == kernel::k_default_position_ms);
            REQUIRE(static_cast<double>(my_object.pitch) == kernel::k_default_pitch_st);
            REQUIRE(static_cast<double>(my_object.drift) == kernel::k_default_drift);
            REQUIRE(static_cast<bool>(my_object.freeze) == false);
            REQUIRE(static_cast<double>(my_object.size) == kernel::k_default_size_ms);
            REQUIRE(static_cast<int>(my_object.overlap) == kernel::k_default_overlap);
            REQUIRE(static_cast<double>(my_object.spray) == kernel::k_default_spray_ms);
            REQUIRE(static_cast<double>(my_object.mix) == kernel::k_default_mix);
        }
        THEN("the captured history matches the kernel's default buy") {
            REQUIRE(static_cast<double>(my_object.maxhistory) == kernel::k_default_max_history_ms * 0.001);
        }
    }
}

SCENARIO("tap.scrub~ clamps its performance surface") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<scrub> an_instance;
        scrub&              my_object = an_instance;

        THEN("pitch stays inside two octaves either way") {
            my_object.pitch = 99.0;
            REQUIRE(static_cast<double>(my_object.pitch) == kernel::k_max_pitch_st);
            my_object.pitch = -99.0;
            REQUIRE(static_cast<double>(my_object.pitch) == -kernel::k_max_pitch_st);
        }
        THEN("drift is bounded both ways") {
            my_object.drift = 100.0;
            REQUIRE(static_cast<double>(my_object.drift) == kernel::k_max_drift);
            my_object.drift = -100.0;
            REQUIRE(static_cast<double>(my_object.drift) == -kernel::k_max_drift);
        }
        THEN("overlap stays inside the grain pool") {
            my_object.overlap = 99;
            REQUIRE(static_cast<int>(my_object.overlap) == kernel::k_max_overlap);
            my_object.overlap = 0;
            REQUIRE(static_cast<int>(my_object.overlap) == 1);
        }
        THEN("grain size stays inside the range the window sum was designed for") {
            my_object.size = 1e6;
            REQUIRE(static_cast<double>(my_object.size) == kernel::k_max_size_ms);
            my_object.size = 0.0;
            REQUIRE(static_cast<double>(my_object.size) == kernel::k_min_size_ms);
        }
        THEN("position and spray cannot go negative") {
            my_object.position = -10.0;
            REQUIRE(static_cast<double>(my_object.position) == 0.0);
            my_object.spray = -10.0;
            REQUIRE(static_cast<double>(my_object.spray) == 0.0);
        }
    }
}

// The min-api test harness has no argument-taking constructor, so the object-argument path is
// exercised in Max rather than here; what the wrapper can pin is the attribute it writes into.
SCENARIO("tap.scrub~'s captured history is settable and floored") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<scrub> an_instance;
        scrub&              my_object = an_instance;

        THEN("it can be retargeted, and cannot be asked for nothing") {
            my_object.maxhistory = 2.0;
            REQUIRE(static_cast<double>(my_object.maxhistory) == 2.0);
            my_object.maxhistory = -1.0;
            REQUIRE(static_cast<double>(my_object.maxhistory) == kernel::k_min_size_ms * 0.001);
        }
    }
}

// The claim the whole object is built on, checked at the wrapper so a plumbing mistake cannot
// hide it: held still at pitch 0 with no spray, this is a delay and nothing else.
SCENARIO("tap.scrub~ held still at unity pitch is a plain delay") {
    ext_main(nullptr);

    GIVEN("a scrub with a whole-sample position and overlap 2") {
        test_wrapper<scrub> an_instance;
        scrub&              my_object = an_instance;
        // Everything here is expressed in SAMPLES and converted, because the mock harness does
        // not necessarily report 48 kHz and the null only holds on whole samples.
        const double sr     = my_object.samplerate();
        const double pos_ms = 480.0 * 1000.0 / sr;
        my_object.smooth    = 0.0;
        my_object.overlap   = 2;
        my_object.size      = 96.0 * 1000.0 / sr; // 96 samples, divides by 2
        my_object.position  = pos_ms;
        my_object.mix       = 100.0;
        my_object.level     = 1.0;

        THEN("the output is the input, 480 samples late") {
            // The position is handed in on the signal inlet AND set as the attribute, to the same
            // value: the mock harness does not model patch cords, so whichever path the wrapper
            // takes must land on the same playhead.
            std::vector<double> in(20000), out(20000);
            for (int i = 0; i < 20000; ++i) {
                in[i] = 0.5 * std::sin(6.2831853071795865 * 220.0 * i / sr)
                        * (0.5 + 0.5 * std::sin(6.2831853071795865 * 3.0 * i / sr));
                out[i] = static_cast<double>(my_object(in[i], pos_ms, 0.0));
            }
            double worst = 0.0;
            for (int i = 2000; i < 20000; ++i) {
                worst = std::max(worst, std::abs(out[i] - in[i - 480]));
            }
            INFO("largest departure from a 480-sample delay: " << worst);
            REQUIRE(worst < 1e-12);
        }
    }
}

SCENARIO("tap.scrub~ is a bitwise passthrough fully dry") {
    ext_main(nullptr);

    GIVEN("a scrub with the balance hard dry") {
        test_wrapper<scrub> an_instance;
        scrub&              my_object = an_instance;
        my_object.smooth              = 0.0;
        my_object.mix                 = 0.0;
        my_object.level               = 1.0;

        THEN("what comes out is exactly what went in") {
            bool identical = true;
            for (int i = 0; i < 4000; ++i) {
                const sample x = 0.4 * std::sin(6.2831853071795865 * 300.0 * i / 48000.0);
                identical      = identical && (my_object(x, 0.0, 0.0) == x);
            }
            REQUIRE(identical);
        }
    }
}
