/// @file
/// @brief      Unit tests for tap.autothru~.
/// @details    The object's whole behavior is a branch on inlet 2's signal-connection state.
///             `inlet::has_signal_connection()` is set by Max's dspsetup via min_dsp64_io() and its
///             backing flag is protected, so the mock can only present the *unconnected* case —
///             which is exactly the documented default: inlet 1 passes through. The connected case
///             needs a real patch cord, so it belongs in runtime-tests/.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include "c74_min_unittest.h"     // required unit-test header (defines main via Catch)
#include "tap.autothru_tilde.cpp" // include the object source so we can instantiate it

SCENARIO("tap.autothru~ passes inlet 1 through while inlet 2 is unconnected") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<autothru> an_instance;
        autothru&              my_object = an_instance;

        THEN("the first inlet arrives at the outlet unchanged") {
            REQUIRE(my_object(0.25, 0.0) == 0.25);
            REQUIRE(my_object(-1.0, 0.0) == -1.0);
            REQUIRE(my_object(0.0, 0.0) == 0.0);
        }
        THEN("the second inlet is ignored — it only takes over once a signal is connected") {
            REQUIRE(my_object(0.25, 0.75) == 0.25);
            REQUIRE(my_object(0.0, 1.0) == 0.0);
        }
        THEN("the pass-through is stateless: the same input always gives the same output") {
            for (int i = 0; i < 64; ++i) {
                REQUIRE(my_object(0.5, -0.5) == 0.5);
            }
        }
    }
}
