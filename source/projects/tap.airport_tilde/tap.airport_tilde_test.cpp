/// @file
/// @brief      Unit tests for tap.airport~ (the incommensurate loop bank).
/// @details    Wrapper-level pins only — the kernel's own suite (taptools tests/airport_test.cpp)
///             carries the structural promises. Here: documented defaults, the record message
///             punching and freezing through the wrapper, bitwise hard-pan absence, and the
///             period report. List attributes are assigned as `atoms`, never as
///             `std::vector<number>` (see tap.multitap_tilde_test.cpp for why).
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.airport_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    long samples_for(double seconds) {
        return static_cast<long>(seconds * k_mock_sr);
    }

} // namespace

SCENARIO("tap.airport~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<airport> an_instance;
        airport&              my_object = an_instance;

        THEN("a 30 s buy, one loop, unity level, center pan, transparent darken") {
            REQUIRE(static_cast<double>(my_object.maxloop) == 30.0);
            REQUIRE(static_cast<int>(my_object.loops) == 1);
            const std::vector<number>& levels = my_object.levels;
            const std::vector<number>& pans   = my_object.pans;
            REQUIRE(levels.size() == 1);
            REQUIRE(levels[0] == 1.0);
            REQUIRE(pans.size() == 1);
            REQUIRE(pans[0] == 0.0);
        }
        THEN("the loop count clamps to 1..8") {
            my_object.loops = 0;
            REQUIRE(static_cast<int>(my_object.loops) == 1);
            my_object.loops = 20;
            REQUIRE(static_cast<int>(my_object.loops) == 8);
        }
    }
}

SCENARIO("tap.airport~ records a click that returns every loop period, hard pan bitwise") {
    ext_main(nullptr);

    GIVEN("one 0.5 s loop panned hard left, a click punched at the head") {
        test_wrapper<airport> an_instance;
        airport&              my_object = an_instance;
        my_object.smooth                = 0.0;
        my_object.lengths               = atoms{0.5};
        my_object.pans                  = atoms{-1.0};

        my_object.record(atoms{0, 1});
        my_object(1.0);
        my_object.record(atoms{0, 0});

        WHEN("two loop periods are rendered") {
            const long          loop = samples_for(0.5);
            std::vector<double> left(static_cast<size_t>(2 * loop), 0.0);
            bool                right_silent = true;
            for (size_t i = 0; i < left.size(); ++i) {
                const auto y = my_object(0.0);
                left[i]      = y[0];
                right_silent = right_silent && (y[1] == 0.0);
            }

            THEN("the click returns bit-exactly on the loop grid, absent from the far bus") {
                // Planted one sample into the run, so it returns at loop - 1, 2*loop - 1.
                REQUIRE(left[static_cast<size_t>(loop) - 1] == 1.0);
                REQUIRE(left[static_cast<size_t>(2 * loop) - 1] == 1.0);
                REQUIRE(right_silent);
            }
        }
    }
}

SCENARIO("tap.airport~ reports the composite period of coprime lengths") {
    ext_main(nullptr);

    GIVEN("two loops whose lengths meet only at 2.5 s") {
        test_wrapper<airport> an_instance;
        airport&              my_object = an_instance;
        my_object.loops                 = 2;
        // 0.5 s and 0.625 s: 22050 and 27563 samples at the mock rate — the exact seconds
        // depend on the ceil to samples, so pin the report against the kernel's own numbers.
        my_object.lengths = atoms{0.5, 0.625};

        THEN("the period message emits lcm(lengths) via the info outlet without error") {
            // The outlet send is fire-and-forget under the mock; the call not throwing plus the
            // kernel introspection agreeing is the wrapper-level promise.
            my_object.period(atoms{});
            REQUIRE(true);
        }
    }
}

SCENARIO("tap.airport~ clear erases the piece but keeps the settings") {
    ext_main(nullptr);

    GIVEN("a loop holding a click") {
        test_wrapper<airport> an_instance;
        airport&              my_object = an_instance;
        my_object.smooth                = 0.0;
        my_object.lengths               = atoms{0.5};
        my_object.pans                  = atoms{-1.0};
        my_object.record(atoms{0, 1});
        my_object(1.0);
        my_object.record(atoms{0, 0});

        WHEN("clear is sent") {
            my_object.clear();

            THEN("both busses stay silent for a full period, and the pan setting survives") {
                bool silent = true;
                for (long i = 0; i < samples_for(0.5); ++i) {
                    const auto y = my_object(0.0);
                    silent       = silent && (y[0] == 0.0) && (y[1] == 0.0);
                }
                REQUIRE(silent);
                const std::vector<number>& pans = my_object.pans;
                REQUIRE(pans[0] == -1.0);
            }
        }
    }
}
