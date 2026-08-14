/// @file
/// @brief      Unit tests for tap.garden~ (the generative event loop).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/garden_test.cpp)
///             carries the DSP promises (staircase, scale contract, seed triad). Here: documented
///             defaults, the note message planting through the wrapper, the scale attribute's
///             symbol-or-index contract, silence with the gardener disabled, and clear as uproot.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"   // required unit-test header (defines main via Catch)
#include "tap.garden_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    long samples_for(double seconds) {
        return static_cast<long>(seconds * k_mock_sr);
    }

    double peak(garden& my_object, long n) {
        double p = 0.0;
        for (long i = 0; i < n; ++i) {
            const auto y = my_object();
            p            = std::max({p, std::abs(static_cast<double>(y[0])), std::abs(static_cast<double>(y[1]))});
        }
        return p;
    }

} // namespace

SCENARIO("tap.garden~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<garden> an_instance;
        garden&              my_object = an_instance;

        THEN("an 8 s loop, decay 0.85, soften 0.9, floor 0.03, the pentatonic, a patient gardener, "
             "a chime rack at spread 0.7 in a moderate wind") {
            REQUIRE(static_cast<double>(my_object.loop) == 8.0);
            REQUIRE(static_cast<double>(my_object.decay) == 0.85);
            REQUIRE(static_cast<double>(my_object.soften) == 0.9);
            REQUIRE(static_cast<double>(my_object.floor) == 0.03);
            REQUIRE(static_cast<double>(my_object.idle) == 30.0);
            REQUIRE(my_object.scale == symbol{"majorpentatonic"});
            REQUIRE(my_object.material == symbol{"chime"});
            REQUIRE(static_cast<double>(my_object.spread) == 0.7);
            REQUIRE(static_cast<double>(my_object.gust) == 0.5);
        }
    }
}

SCENARIO("tap.garden~ accepts the scale as a symbol or an index") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<garden> an_instance;
        garden&              my_object = an_instance;

        THEN("a symbol sets and reports itself") {
            my_object.scale = "minor";
            REQUIRE(my_object.scale == symbol{"minor"});
        }
        THEN("an index reports back as its symbol") {
            my_object.scale = atoms{0};
            REQUIRE(my_object.scale == symbol{"chromatic"});
        }
        THEN("an unknown symbol falls back to the default field") {
            my_object.scale = "phrygian";
            REQUIRE(my_object.scale == symbol{"majorpentatonic"});
        }
    }
}

SCENARIO("tap.garden~ accepts the material as a symbol or an index") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<garden> an_instance;
        garden&              my_object = an_instance;

        THEN("a symbol sets and reports itself") {
            my_object.material = "bar";
            REQUIRE(my_object.material == symbol{"bar"});
        }
        THEN("an index reports back as its symbol") {
            my_object.material = atoms{1};
            REQUIRE(my_object.material == symbol{"bar"});
        }
        THEN("an unknown symbol falls back to the chime rack") {
            my_object.material = "glass";
            REQUIRE(my_object.material == symbol{"chime"});
        }
    }
}

SCENARIO("tap.garden~ at spread 0 sounds the same garden on both outlets, bitwise") {
    ext_main(nullptr);

    GIVEN("spread 0 and a planted note") {
        test_wrapper<garden> an_instance;
        garden&              my_object = an_instance;
        my_object.idle                 = 0.0;
        my_object.spread               = 0.0;
        my_object.note(atoms{69, 0.8});

        THEN("left and right agree to the bit") {
            bool same = true;
            for (long i = 0; i < samples_for(0.5); ++i) {
                const auto y = my_object();
                same         = same && (y[0] == y[1]);
            }
            REQUIRE(same);
        }
    }
}

SCENARIO("tap.garden~ is silent until planted when the gardener is disabled") {
    ext_main(nullptr);

    GIVEN("idle 0 — no self-seeding") {
        test_wrapper<garden> an_instance;
        garden&              my_object = an_instance;
        my_object.idle                 = 0.0;

        THEN("a full loop of silence") {
            REQUIRE(peak(my_object, samples_for(1.0)) == 0.0);
        }

        WHEN("a note is planted") {
            my_object.note(atoms{69, 0.8});

            THEN("the bell sounds") {
                REQUIRE(peak(my_object, samples_for(0.5)) > 0.1);
            }
        }
    }
}

SCENARIO("tap.garden~ clear uproots the garden but keeps the settings") {
    ext_main(nullptr);

    GIVEN("a planted, ringing garden with fast bells") {
        test_wrapper<garden> an_instance;
        garden&              my_object = an_instance;
        my_object.idle                 = 0.0;
        my_object.bell                 = atoms{0.001, 0.05, 1.0};
        my_object.loop                 = 0.5;
        my_object.note(atoms{69, 0.8});
        REQUIRE(peak(my_object, samples_for(0.25)) > 0.1);

        WHEN("clear is sent") {
            my_object.clear();

            THEN("nothing returns on the next pass, and the loop setting survives") {
                REQUIRE(peak(my_object, samples_for(1.0)) < 1e-9);
                REQUIRE(static_cast<double>(my_object.loop) == 0.5);
            }
        }
    }
}
