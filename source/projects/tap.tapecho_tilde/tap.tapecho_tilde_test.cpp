/// @file
/// @brief      Unit tests for tap.tapecho~ (the multi-head tape echo).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/tapecho_test.cpp)
///             carries the DSP promises, including the bitwise null test against delay.h. Here:
///             documented defaults, the attribute clamps forwarding into the kernel (in
///             particular that regen is allowed *past* unity, unlike tap.discreet~), the head
///             layout reaching the tape through the wrapper, and clear as the eject button. The
///             kernel slews over 20 ms by default, so scenarios zero the smooth attribute first.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.tapecho_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    long samples_for_ms(double ms) {
        return static_cast<long>(ms * 0.001 * k_mock_sr);
    }

} // namespace

SCENARIO("tap.tapecho~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<tapecho> an_instance;
        tapecho&              my_object = an_instance;

        THEN("a 4 s tape, a 375 ms motor, four heads, and the stock wear and transport") {
            REQUIRE(static_cast<double>(my_object.maxspan) == 4.0);
            REQUIRE(static_cast<double>(my_object.span) == 375.0);
            REQUIRE(static_cast<int>(my_object.heads) == 4);
            REQUIRE(static_cast<double>(my_object.regen) == 0.35);
            REQUIRE(static_cast<double>(my_object.darken) == 4000.0);
            REQUIRE(static_cast<double>(my_object.drive) == 0.5);
            REQUIRE(static_cast<double>(my_object.input) == 1.0);
            REQUIRE(static_cast<double>(my_object.mix) == 35.0);
        }
        THEN("the default head layout is four evenly spaced ratios") {
            const std::vector<number>& r = my_object.ratios;
            REQUIRE(r.size() == 4);
            REQUIRE(r[0] == 0.25);
            REQUIRE(r[1] == 0.5);
            REQUIRE(r[2] == 0.75);
            REQUIRE(r[3] == 1.0);
        }
    }
}

// The wrapper-visible half of the kernel's design statement: unlike tap.discreet~, which clamps
// regen at exactly 1.0, this object accepts values past unity — that is the self-oscillating
// regime, and the drive saturator is what makes it legal.
SCENARIO("tap.tapecho~ accepts regeneration past unity, unlike tap.discreet~") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<tapecho> an_instance;
        tapecho&              my_object = an_instance;

        THEN("1.4 is kept, not clamped to 1") {
            my_object.regen = 1.4;
            REQUIRE(static_cast<double>(my_object.regen) == 1.4);
        }
        THEN("the ceiling is the kernel's driven maximum") {
            my_object.regen = 9.0;
            REQUIRE(static_cast<double>(my_object.regen) == 1.5);
        }
        THEN("a negative regen floors at 0") {
            my_object.regen = -0.3;
            REQUIRE(static_cast<double>(my_object.regen) == 0.0);
        }
    }
}

SCENARIO("tap.tapecho~ clamps the head count to the four slots") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<tapecho> an_instance;
        tapecho&              my_object = an_instance;

        THEN("more heads than slots reports back as four") {
            my_object.heads = 9;
            REQUIRE(static_cast<int>(my_object.heads) == 4);
        }
        THEN("none is a legal count") {
            my_object.heads = 0;
            REQUIRE(static_cast<int>(my_object.heads) == 0);
        }
    }
}

SCENARIO("tap.tapecho~ puts a head's return where its ratio says, through the wrapper") {
    ext_main(nullptr);

    GIVEN("one head at half a 200 ms span, wet-only, transport parked") {
        test_wrapper<tapecho> an_instance;
        tapecho&              my_object = an_instance;
        my_object.smooth                = 0.0;
        my_object.wow                   = atoms{0.0, 0.0};
        my_object.flutter               = atoms{0.0, 0.0};
        my_object.regen                 = 0.0;
        my_object.heads                 = 1;
        my_object.ratios                = atoms{0.5};
        my_object.pans                  = atoms{-1.0}; // hard left: the return lands on one bus only
        my_object.span                  = 200.0;
        my_object.mix                   = 100.0;

        WHEN("an impulse is processed") {
            const long          at = samples_for_ms(100.0); // 0.5 of a 200 ms span
            std::vector<double> left(static_cast<size_t>(at) + 8, 0.0);
            std::vector<double> right(static_cast<size_t>(at) + 8, 0.0);
            for (size_t i = 0; i < left.size(); ++i) {
                const auto y = my_object(i == 0 ? 1.0 : 0.0);
                left[i]      = y[0];
                right[i]     = y[1];
            }

            THEN("the return is the recorded impulse itself, bit for bit, at 100 ms") {
                REQUIRE(left[static_cast<size_t>(at)] == 1.0);
            }
            THEN("and the far bus is bitwise silent throughout") {
                bool silent = true;
                for (double v : right) {
                    silent = silent && (v == 0.0);
                }
                REQUIRE(silent);
            }
        }
    }
}

SCENARIO("tap.tapecho~ mix 0 is bitwise dry on both outlets") {
    ext_main(nullptr);

    GIVEN("mix 0 with the echo otherwise live") {
        test_wrapper<tapecho> an_instance;
        tapecho&              my_object = an_instance;
        my_object.smooth                = 0.0;
        my_object.mix                   = 0.0;
        my_object.regen                 = 0.9;

        THEN("both outputs are the input, bitwise") {
            bool exact = true;
            for (int i = 0; i < 512; ++i) {
                const double x = 0.001 * static_cast<double>(i % 100) - 0.05;
                const auto   y = my_object(x);
                exact          = exact && (y[0] == x) && (y[1] == x);
            }
            REQUIRE(exact);
        }
    }
}

SCENARIO("tap.tapecho~ clear is the eject button") {
    ext_main(nullptr);

    GIVEN("a short span holding an impulse") {
        test_wrapper<tapecho> an_instance;
        tapecho&              my_object = an_instance;
        my_object.smooth                = 0.0;
        my_object.wow                   = atoms{0.0, 0.0};
        my_object.flutter               = atoms{0.0, 0.0};
        my_object.heads                 = 1;
        my_object.ratios                = atoms{1.0};
        my_object.span                  = 150.0;
        my_object.mix                   = 100.0;
        my_object(1.0);

        WHEN("clear is sent before the return comes due") {
            my_object.clear();

            THEN("the tape is silent") {
                bool silent = true;
                for (long i = 0; i < samples_for_ms(300.0); ++i) {
                    const auto y = my_object(0.0);
                    silent       = silent && (y[0] == 0.0) && (y[1] == 0.0);
                }
                REQUIRE(silent);
            }
        }
    }
}
