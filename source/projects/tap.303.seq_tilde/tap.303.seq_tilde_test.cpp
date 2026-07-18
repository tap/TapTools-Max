/// @file
/// @brief      Unit tests for tap.303.seq~ (Min-level: attribute defaults, clamping, plumbing).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.303.seq_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.303.seq~ instantiates with the plan's defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<seq303> an_instance;
        seq303&              my_object = an_instance;

        THEN("the pattern is 16 steps, straight, untransposed, cycle-quantized") {
            REQUIRE(static_cast<int>(my_object.length) == 16);
            REQUIRE(static_cast<double>(my_object.swing) == 0.0);
            REQUIRE(static_cast<double>(my_object.transpose) == 0.0);
            REQUIRE(my_object.quantize == symbol("cycle"));
        }
        THEN("all steps rest") {
            auto& p = my_object.row().clock().data();
            for (int k = 0; k < p.length; ++k)
                REQUIRE(p.steps[k].gate == false);
        }
    }
}

SCENARIO("tap.303.seq~ plumbs the pattern surface into the kernel") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<seq303> an_instance;
        seq303&              my_object = an_instance;

        WHEN("a step message sets pitch, accent, and slide") {
            my_object.step(c74::min::atoms{3, 45.0, 1, 1}); // 1-based
            THEN("step 3 is gated with the flags") {
                auto& st = my_object.row().clock().data().steps[2];
                REQUIRE(st.gate == true);
                REQUIRE(st.pitch == 45.0);
                REQUIRE(st.accent == true);
                REQUIRE(st.slide == true);
            }
        }
        WHEN("rest silences it again") {
            my_object.step(c74::min::atoms{3, 45.0});
            my_object.rest(c74::min::atoms{3});
            THEN("the gate clears but the pitch stays") {
                auto& st = my_object.row().clock().data().steps[2];
                REQUIRE(st.gate == false);
                REQUIRE(st.pitch == 45.0);
            }
        }
        WHEN("the list surface writes a line") {
            my_object.pitches(c74::min::atoms{33.0, 45.0, 36.0});
            my_object.gates(c74::min::atoms{1, 1, 0});
            my_object.accents(c74::min::atoms{1, 0, 0});
            my_object.slides(c74::min::atoms{0, 1, 0});
            THEN("the kernel pattern holds it") {
                auto& p = my_object.row().clock().data();
                REQUIRE(p.steps[0].pitch == 33.0);
                REQUIRE(p.steps[0].accent == true);
                REQUIRE(p.steps[1].slide == true);
                REQUIRE(p.steps[2].gate == false);
            }
        }
        WHEN("transpose is set out of range") {
            my_object.transpose = 90.0;
            THEN("it clamps to 48 and reaches the row") {
                REQUIRE(static_cast<double>(my_object.transpose) == 48.0);
                REQUIRE(my_object.row().transpose() == 48.0);
            }
        }
        WHEN("quantize is set to now and a stored slot is recalled") {
            my_object.step(c74::min::atoms{1, 40.0});
            my_object.store(c74::min::atoms{2});
            my_object.rest(c74::min::atoms{1});
            my_object.quantize = c74::min::symbol("now");
            my_object.recall(c74::min::atoms{2});
            THEN("the slot applies immediately") {
                REQUIRE(my_object.row().clock().data().steps[0].gate == true);
            }
        }
    }
}
