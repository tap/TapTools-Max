/// @file
/// @brief      Unit tests for tap.vca~ and its vca.h kernel.
/// @details    Kernel scenarios exercise taptools::vca directly; one scenario goes through the Min
///             mock for the wrapper's attribute defaults, forwarding, and clamping. The deep DSP
///             correctness tests live in the kernel repo (tests/vca_test.cpp); this pins the
///             Min-level behavior. The harness pins this translation unit to C++17.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"
#include "tap.vca_tilde.cpp"

namespace {

    constexpr double k_sr = 48000.0;

    taptools::vca make_stage(int mode = taptools::vca::mode_clean) {
        taptools::vca a;
        a.prepare(k_sr);
        a.set_mode(mode);
        return a;
    }

} // namespace

SCENARIO("the clean circuit is an exact linear multiply") {
    auto a = make_stage(taptools::vca::mode_clean);
    THEN("process(x, gain) == x * gain to the bit") {
        for (double g : {0.0, 0.5, 1.0, 2.0, -0.5}) {
            for (double x : {-0.8, 0.0, 0.42}) {
                REQUIRE(a.process(x, g) == x * g);
            }
        }
    }
}

SCENARIO("the warm circuit adds tracking saturation but leaves quiet signals near-clean") {
    auto a = make_stage(taptools::vca::mode_warm);
    THEN("a tiny signal passes at ~unity gain") {
        REQUIRE(a.shape(1e-4) == Approx(1e-4).epsilon(1e-3));
    }
    THEN("a hot signal is compressed relative to clean") {
        REQUIRE(std::abs(a.shape(1.5)) < 1.5);
    }
    THEN("the shaper is monotonic across the working range") {
        double prev = a.shape(-2.0);
        for (double v = -1.9; v <= 2.0; v += 0.1) {
            REQUIRE(a.shape(v) > prev);
            prev = a.shape(v);
        }
    }
}

SCENARIO("the Min wrapper instantiates and takes its attributes") {
    ext_main(nullptr);
    GIVEN("an instance of tap.vca~") {
        test_wrapper<vca> an_instance;
        vca&              my_object = an_instance;

        THEN("the defaults are sane") {
            REQUIRE(my_object.circuit == symbol("clean"));
            REQUIRE(static_cast<double>(my_object.gain) == 1.0);
            REQUIRE(static_cast<double>(my_object.drive) == taptools::vca::k_default_drive);
            REQUIRE(static_cast<double>(my_object.bias) == taptools::vca::k_default_bias);
            REQUIRE(static_cast<bool>(my_object.dcblock) == true);
            REQUIRE(my_object.stage().circuit() == taptools::vca::mode_clean);
        }

        WHEN("the circuit is switched to warm") {
            my_object.circuit = "warm";
            THEN("the stage follows") {
                REQUIRE(my_object.circuit == symbol("warm"));
                REQUIRE(my_object.stage().circuit() == taptools::vca::mode_warm);
            }
        }

        WHEN("drive and bias are set out of range") {
            my_object.drive = 100.0; // clamps to 12
            my_object.bias  = -9.0;  // clamps to -2
            THEN("they clamp to the documented range and reach the stage") {
                REQUIRE(static_cast<double>(my_object.drive) == 12.0);
                REQUIRE(static_cast<double>(my_object.bias) == -2.0);
                REQUIRE(my_object.stage().drive() == 12.0);
                REQUIRE(my_object.stage().bias() == -2.0);
            }
        }

        WHEN("dcblock is turned off") {
            my_object.dcblock = false;
            THEN("the stage follows") {
                REQUIRE(my_object.stage().dc_block() == false);
            }
        }
    }
}
