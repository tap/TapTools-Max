/// @file
/// @brief      Unit tests for tap.delay~ (the kernel-backed rebuild).
/// @details    The kernel slews every parameter over 20 ms by default, so each scenario settles the
///             ramps (~882 samples at the mock 44.1 kHz) before measuring. The signal-rate
///             time-override path branches on `inlet::has_signal_connection()`, which the mock
///             harness can only present as *unconnected* (see tap.autothru~'s test note) — so the
///             "a time signal always wins, including at 0.0" contract needs a real patch cord and
///             belongs in runtime-tests/. What is pinned here is the message-rate surface.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 1999-2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"  // required unit-test header (defines main via Catch)
#include "tap.delay_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    // Run silence through the object so the kernel's parameter ramps reach their targets.
    void settle(delay& object, size_t n = 2048) {
        for (size_t i = 0; i < n; ++i) {
            object(0.0, 0.0);
        }
    }

    // Feed an impulse followed by silence and return the whole output.
    std::vector<double> impulse_response(delay& object, size_t n) {
        std::vector<double> out(n, 0.0);
        for (size_t i = 0; i < n; ++i) {
            out[i] = object(i == 0 ? 1.0 : 0.0, 0.0);
        }
        return out;
    }

} // namespace

SCENARIO("tap.delay~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;

        THEN("buffer 1000 ms, delay 0, feedback 0, mix 100 (wet-only), Hermite interpolation") {
            REQUIRE(static_cast<double>(my_object.buffersize) == 1000.0);
            REQUIRE(static_cast<double>(my_object.delaytime) == 0.0);
            REQUIRE(static_cast<double>(my_object.feedback) == 0.0);
            REQUIRE(static_cast<double>(my_object.mix) == 100.0);
            REQUIRE(static_cast<int>(my_object.interp) == 1);
        }
    }
}

SCENARIO("tap.delay~ delays by the attribute value") {
    ext_main(nullptr);

    GIVEN("a 10 ms delay, ramps settled") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;
        my_object.delaytime           = 10.0;
        settle(my_object);

        WHEN("an impulse is processed") {
            const std::vector<double> out = impulse_response(my_object, 2048);

            THEN("the tap comes out at unity, 441 samples later (a whole-sample delay reads exactly)") {
                REQUIRE(out[static_cast<size_t>(10.0 * k_mock_sr * 0.001)] == 1.0);
            }
        }
    }

    GIVEN("the same delay in legacy truncation mode") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;
        my_object.interp              = 0;
        my_object.delaytime           = 10.0;
        settle(my_object);

        WHEN("an impulse is processed") {
            const std::vector<double> out = impulse_response(my_object, 2048);

            THEN("the tap lands at long(ms * sr / 1000) samples, exactly as the old object") {
                REQUIRE(out[441] == 1.0);
            }
        }
    }
}

SCENARIO("tap.delay~ recirculates through the feedback path") {
    ext_main(nullptr);

    GIVEN("a 10 ms delay with 0.5 feedback, ramps settled") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;
        my_object.delaytime           = 10.0;
        my_object.feedback            = 0.5;
        settle(my_object);

        WHEN("an impulse is processed") {
            const std::vector<double> out = impulse_response(my_object, 4096);

            THEN("the echoes decay by ~the feedback coefficient (the loop DC blocker shaves a hair)") {
                REQUIRE(out[441] == 1.0);
                REQUIRE(std::abs(out[882] - 0.5) < 0.01);
                REQUIRE(std::abs(out[1323] - 0.25) < 0.01);
            }
        }
    }
}

SCENARIO("tap.delay~ honours the equal-power mix endpoints") {
    ext_main(nullptr);

    GIVEN("mix 0, ramps settled") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;
        my_object.delaytime           = 10.0;
        my_object.mix                 = 0.0;
        settle(my_object);

        THEN("the input passes through bitwise dry") {
            REQUIRE(my_object(0.5, 0.0) == 0.5);
            REQUIRE(my_object(-0.25, 0.0) == -0.25);
        }
    }
}

SCENARIO("tap.delay~ clamps its attributes to the documented ranges") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;

        THEN("a negative delay is floored at zero") {
            my_object.delaytime = -50.0;
            REQUIRE(static_cast<double>(my_object.delaytime) == 0.0);
        }
        THEN("feedback is capped at 0.99") {
            my_object.feedback = 2.0;
            REQUIRE(static_cast<double>(my_object.feedback) == 0.99);
            my_object.feedback = -1.0;
            REQUIRE(static_cast<double>(my_object.feedback) == 0.0);
        }
        THEN("mix is clamped to 0..100") {
            my_object.mix = 150.0;
            REQUIRE(static_cast<double>(my_object.mix) == 100.0);
            my_object.mix = -10.0;
            REQUIRE(static_cast<double>(my_object.mix) == 0.0);
        }
        THEN("interp collapses to 0 or 1") {
            my_object.interp = 7;
            REQUIRE(static_cast<int>(my_object.interp) == 1);
            my_object.interp = 0;
            REQUIRE(static_cast<int>(my_object.interp) == 0);
        }
    }

    GIVEN("a delay past the buffer size, ramps settled") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;
        my_object.interp              = 0;
        my_object.delaytime           = 5000.0; // buffer is 1000 ms
        settle(my_object, 4096);

        WHEN("an impulse is processed") {
            const std::vector<double> out = impulse_response(my_object, 46000);

            THEN("the tap is bounded by the buffer instead of wrapping") {
                REQUIRE(out[static_cast<size_t>(1000.0 * k_mock_sr * 0.001)] == 1.0);
            }
        }
    }
}

SCENARIO("tap.delay~ clears its buffer on demand") {
    ext_main(nullptr);

    GIVEN("a 10 ms delay holding an impulse") {
        test_wrapper<delay> an_instance;
        delay&              my_object = an_instance;
        my_object.delaytime           = 10.0;
        settle(my_object);
        my_object(1.0, 0.0); // write the impulse
        for (int i = 0; i < 100; ++i) {
            my_object(0.0, 0.0);
        }

        WHEN("clear is sent before the tap comes due") {
            my_object.clear();

            THEN("the buffered impulse never emerges") {
                for (int i = 0; i < 2048; ++i) {
                    REQUIRE(my_object(0.0, 0.0) == 0.0);
                }
            }
        }
    }
}
