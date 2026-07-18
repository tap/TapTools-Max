/// @file
/// @brief      Unit tests for tap.808.seq~ (Min-level: attribute defaults, clamping, plumbing).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.808.seq_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.808.seq~ instantiates with the plan's defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<seq808> an_instance;
        seq808&              my_object = an_instance;

        THEN("the pattern is 16 steps, straight, single-sample impulses, cycle-quantized") {
            REQUIRE(static_cast<int>(my_object.length) == 16);
            REQUIRE(static_cast<double>(my_object.swing) == 0.0);
            REQUIRE(static_cast<double>(my_object.pulse) == 0.0);
            REQUIRE(my_object.quantize == symbol("cycle"));
        }
        THEN("the trigger levels are the pinned bus constants") {
            REQUIRE(static_cast<double>(my_object.plain) == taptools::seq::k_trig_plain);
            REQUIRE(static_cast<double>(my_object.accented) == taptools::seq::k_trig_accented);
        }
    }
}

SCENARIO("tap.808.seq~ clamps and plumbs into the kernel") {
    ext_main(nullptr);

    GIVEN("an instance") {
        test_wrapper<seq808> an_instance;
        seq808&              my_object = an_instance;

        WHEN("length is set out of range") {
            my_object.length = 999;
            THEN("it clamps to the kernel maximum, in the kernel too") {
                REQUIRE(static_cast<int>(my_object.length) == taptools::seq::k_max_steps);
                REQUIRE(my_object.row().clock().data().length == taptools::seq::k_max_steps);
            }
        }
        WHEN("swing is set past 1") {
            my_object.swing = 3.0;
            THEN("it clamps to 1 and reaches the engine") {
                REQUIRE(static_cast<double>(my_object.swing) == 1.0);
                REQUIRE(my_object.row().clock().swing() == 1.0);
            }
        }
        WHEN("a hits list arrives") {
            my_object.hits(c74::min::atoms{1, 0, 1, 0});
            THEN("sounding steps carry the plain level and rests are zero") {
                auto& p = my_object.row().clock().data();
                REQUIRE(p.steps[0].velocity == taptools::seq::k_trig_plain);
                REQUIRE(p.steps[1].velocity == 0.0);
                REQUIRE(p.steps[2].velocity == taptools::seq::k_trig_plain);
            }
        }
        WHEN("an accents list marks a sounding step") {
            my_object.hits(c74::min::atoms{1, 1});
            my_object.accents(c74::min::atoms{0, 1});
            THEN("the accented step carries the accented level") {
                auto& p = my_object.row().clock().data();
                REQUIRE(p.steps[0].velocity == taptools::seq::k_trig_plain);
                REQUIRE(p.steps[1].velocity == taptools::seq::k_trig_accented);
            }
        }
        WHEN("a step message sets a velocity directly") {
            my_object.step(c74::min::atoms{3, 0.8}); // 1-based
            THEN("step 3 holds it") {
                REQUIRE(my_object.row().clock().data().steps[2].velocity == 0.8);
            }
        }
        WHEN("quantize is set to now and a stored slot is recalled") {
            my_object.step(c74::min::atoms{1, 0.6});
            my_object.store(c74::min::atoms{1});
            my_object.step(c74::min::atoms{1, 1.0});
            my_object.quantize = c74::min::symbol("now");
            my_object.recall(c74::min::atoms{1});
            THEN("the slot applies immediately") {
                REQUIRE(my_object.row().clock().data().steps[0].velocity == 0.6);
            }
        }
        WHEN("clear arrives") {
            my_object.step(c74::min::atoms{1, 1.0});
            my_object.clear();
            THEN("the pattern is all rests") {
                REQUIRE(my_object.row().clock().data().steps[0].velocity == 0.0);
            }
        }
    }
}
