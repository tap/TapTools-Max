/// @file
/// @brief      Unit tests for the tap.autowah~ Min wrapper.
/// @details    Wrapper-level behavior only — attribute defaults, clamping, the range/direction
///             sign forwarding, and message plumbing. DSP correctness lives in the kernel repo
///             (tests/autowah_test.cpp there). The harness pins this translation unit to C++17.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include "c74_min_unittest.h"
#include "tap.autowah_tilde.cpp"

namespace kaw = taptools::autowah;

SCENARIO("the Min wrapper instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("an instance with default attributes") {
        test_wrapper<autowah> an_instance;
        autowah&              my_object = an_instance;

        THEN("attribute defaults match the spec") {
            REQUIRE(static_cast<double>(my_object.sensitivity) == 0.0);
            REQUIRE(static_cast<double>(my_object.attack) == 2.0);
            REQUIRE(static_cast<double>(my_object.decay) == 250.0);
            REQUIRE(static_cast<double>(my_object.bias) == 250.0);
            REQUIRE(static_cast<double>(my_object.range) == 3.3);
            REQUIRE(static_cast<double>(my_object.resonance) == 0.55);
            REQUIRE(static_cast<double>(my_object.drive) == 0.0);
            REQUIRE(static_cast<double>(my_object.gain) == 0.0);
            REQUIRE(static_cast<double>(my_object.mix) == 100.0);
            REQUIRE(static_cast<int>(my_object.mode) == kaw::mode_lowpass);
            REQUIRE(static_cast<int>(my_object.direction) == 0);
            REQUIRE(static_cast<double>(my_object.smooth) == kaw::k_default_smooth_ms);
            REQUIRE(static_cast<bool>(my_object.bypass) == false);
            REQUIRE(static_cast<bool>(my_object.mute) == false);
        }

        THEN("the kernel receives the defaults") {
            REQUIRE(my_object.filter().param(kaw::p_bias) == 250.0);
            REQUIRE(my_object.filter().param(kaw::p_range) == 3.3);
            REQUIRE(my_object.filter().param(kaw::p_mix) == 100.0);
        }
    }
}

SCENARIO("out-of-range attribute values are clamped") {
    ext_main(nullptr);
    test_wrapper<autowah> an_instance;
    autowah&              my_object = an_instance;

    my_object.sensitivity = 100.0;
    REQUIRE(static_cast<double>(my_object.sensitivity) == kaw::k_sens_ceil_db);
    my_object.sensitivity = -200.0;
    REQUIRE(static_cast<double>(my_object.sensitivity) == kaw::k_sens_floor_db);

    my_object.decay = 1.0;
    REQUIRE(static_cast<double>(my_object.decay) == kaw::k_decay_min_ms);
    my_object.decay = 100000.0;
    REQUIRE(static_cast<double>(my_object.decay) == kaw::k_decay_max_ms);

    my_object.range = 99.0;
    REQUIRE(static_cast<double>(my_object.range) == kaw::k_range_max_oct);
    my_object.range = -1.0; // magnitude attribute: negative clamps to 0 (use direction to sweep down)
    REQUIRE(static_cast<double>(my_object.range) == 0.0);

    my_object.mode = 99;
    REQUIRE(static_cast<int>(my_object.mode) == kaw::k_num_modes - 1);
    my_object.direction = 7;
    REQUIRE(static_cast<int>(my_object.direction) == 1);

    my_object.mix = 150.0;
    REQUIRE(static_cast<double>(my_object.mix) == 100.0);
}

SCENARIO("direction flips the sign of the range forwarded to the kernel") {
    ext_main(nullptr);
    test_wrapper<autowah> an_instance;
    autowah&              my_object = an_instance;

    my_object.range = 3.0;
    REQUIRE(my_object.filter().param(kaw::p_range) == 3.0);

    my_object.direction = 1;
    REQUIRE(my_object.filter().param(kaw::p_range) == -3.0);

    my_object.range = 2.0; // magnitude changes keep the down direction
    REQUIRE(my_object.filter().param(kaw::p_range) == -2.0);

    my_object.direction = 0;
    REQUIRE(my_object.filter().param(kaw::p_range) == 2.0);
}

SCENARIO("preset and clear messages are callable and factory slots are populated") {
    ext_main(nullptr);
    test_wrapper<autowah> an_instance;
    autowah&              my_object = an_instance;

    // factory voicings installed by the kernel (1-based in the Max surface)
    REQUIRE(my_object.filter().preset(1).v[kaw::p_bias] == 120.0);                       // 'recall 2' = bass
    REQUIRE(my_object.filter().preset(3).v[kaw::p_sensitivity] == kaw::k_sens_floor_db); // 'recall 4' = cocked

    my_object.store(c74::min::atoms{6});
    REQUIRE(my_object.filter().preset(5).v[kaw::p_bias] == 250.0);

    my_object.recall(c74::min::atoms{2, 0.0});
    REQUIRE(my_object.filter().param(kaw::p_bias) == 120.0);

    my_object.clear(c74::min::atoms{});
    REQUIRE(my_object.filter().envelope() == 0.0);
}
