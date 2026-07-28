/// @file
/// @brief      Unit tests for tap.procrastinate~.
/// @details    The object draws every voice parameter randomly, so the scenarios that need a
///             predictable machine collapse each range to a single value (low == high) and bang.
///             That turns the four random voices into four known ones and makes the cascade geometry
///             — the load-bearing part of the port — measurable.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2000-2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <vector>

#include "c74_min_unittest.h"          // required unit-test header (defines main via Catch)
#include "tap.procrastinate_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    struct stereo {
        std::vector<double> left;
        std::vector<double> right;
    };

    // Pin every voice to one parameter set: unity gain, hard-left pan, no pitch shift, and a
    // `window_ms` window. With no pitch shift the phasor is static, so each voice becomes a plain
    // delay of half its window.
    void pin_all_voices(procrastinate& object, double window_ms) {
        object.gain_range(atoms{0, 0.0, 0.0});  // 0 dB
        object.pan_range(atoms{0, 0.0, 0.0});   // hard left
        object.shift_range(atoms{0, 1.0, 1.0}); // no transposition
        object.delay_range(atoms{0, window_ms, window_ms});
        object.bang(); // draw the (now singular) parameters
    }

    stereo impulse_response(procrastinate& object, size_t n) {
        stereo out{std::vector<double>(n, 0.0), std::vector<double>(n, 0.0)};
        for (size_t i = 0; i < n; ++i) {
            const auto y = object(i == 0 ? 1.0 : 0.0);
            out.left[i]  = y[0];
            out.right[i] = y[1];
        }
        return out;
    }

    size_t count_nonzero(const std::vector<double>& x) {
        size_t n = 0;
        for (double s : x) {
            if (s != 0.0) {
                ++n;
            }
        }
        return n;
    }

} // namespace

SCENARIO("tap.procrastinate~ instantiates with the documented default") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<procrastinate> an_instance;
        procrastinate&              my_object = an_instance;

        THEN("it is not muted") {
            REQUIRE(static_cast<bool>(my_object.mute) == false);
        }
    }
}

SCENARIO("tap.procrastinate~ mutes both outputs") {
    ext_main(nullptr);

    GIVEN("a muted instance") {
        test_wrapper<procrastinate> an_instance;
        procrastinate&              my_object = an_instance;
        my_object.mute                        = true;

        THEN("both channels are silent whatever goes in") {
            for (int i = 0; i < 2048; ++i) {
                const auto y = my_object(1.0);
                REQUIRE(y[0] == 0.0);
                REQUIRE(y[1] == 0.0);
            }
        }
    }
}

SCENARIO("tap.procrastinate~ cascades its four voices") {
    ext_main(nullptr);

    GIVEN("all four voices pinned to a 100 ms window with no pitch shift, panned hard left") {
        test_wrapper<procrastinate> an_instance;
        procrastinate&              my_object = an_instance;
        pin_all_voices(my_object, 100.0);

        // Each voice reads a half-window back; the padded-Welch window is zero at phase 0, so only
        // the phase-0.5 tap contributes. 100 ms is 4410 samples, so the half-window tap is at 2205 —
        // and read_delay's interpolation lands it one sample early, at 2204.
        const long per_voice = static_cast<long>(0.5 * 100.0 * k_mock_sr * 0.001) - 1;

        WHEN("an impulse is processed") {
            const stereo out = impulse_response(my_object, 4 * per_voice + 64);

            THEN("four taps come out — every voice is summed as well as cascaded") {
                REQUIRE(count_nonzero(out.left) == 4);
                for (int v = 1; v <= 4; ++v) {
                    REQUIRE(out.left[v * per_voice] != 0.0);
                }
            }
            THEN("each tap is one more voice deep, all at essentially full amplitude") {
                for (int v = 1; v <= 4; ++v) {
                    REQUIRE(std::abs(out.left[v * per_voice] - 1.0) < 1e-3);
                }
                // Each cascade stage multiplies by the window peak, so the taps decay monotonically.
                REQUIRE(out.left[per_voice] > out.left[2 * per_voice]);
                REQUIRE(out.left[2 * per_voice] > out.left[3 * per_voice]);
                REQUIRE(out.left[3 * per_voice] > out.left[4 * per_voice]);
            }
            THEN("the hard-left pan puts nothing on the right") {
                for (double s : out.right) {
                    REQUIRE(s == 0.0);
                }
            }
        }
    }

    GIVEN("the same machine panned hard right") {
        test_wrapper<procrastinate> an_instance;
        procrastinate&              my_object = an_instance;
        my_object.gain_range(atoms{0, 0.0, 0.0});
        my_object.pan_range(atoms{0, 1.0, 1.0});
        my_object.shift_range(atoms{0, 1.0, 1.0});
        my_object.delay_range(atoms{0, 100.0, 100.0});
        my_object.bang();

        const long per_voice = static_cast<long>(0.5 * 100.0 * k_mock_sr * 0.001) - 1;

        WHEN("an impulse is processed") {
            const stereo out = impulse_response(my_object, 4 * per_voice + 64);

            THEN("the equal-power pan puts nothing on the left") {
                // cos(pi/2) is not exactly zero in floating point, so measure against the level the
                // right channel carries rather than against zero.
                REQUIRE(std::abs(out.right[4 * per_voice]) > 0.9);
                REQUIRE(std::abs(out.left[4 * per_voice]) < 1e-9);
            }
        }
    }
}

SCENARIO("tap.procrastinate~ takes a per-voice window and pitch ratio") {
    ext_main(nullptr);

    GIVEN("all voices pinned, then voice 1's window widened on its own") {
        test_wrapper<procrastinate> an_instance;
        procrastinate&              my_object = an_instance;
        pin_all_voices(my_object, 100.0);
        my_object.window(atoms{0, 200.0}); // voice index 0

        const long wide   = static_cast<long>(0.5 * 200.0 * k_mock_sr * 0.001) - 1;
        const long narrow = static_cast<long>(0.5 * 100.0 * k_mock_sr * 0.001) - 1;

        WHEN("an impulse is processed") {
            const stereo out = impulse_response(my_object, wide + 3 * narrow + 64);

            THEN("the widened voice shifts its own tap and everything downstream of it") {
                REQUIRE(count_nonzero(out.left) == 4);
                REQUIRE(out.left[wide] != 0.0);
                REQUIRE(out.left[wide + narrow] != 0.0);
                REQUIRE(out.left[wide + 2 * narrow] != 0.0);
                REQUIRE(out.left[wide + 3 * narrow] != 0.0);
            }
        }
    }

    GIVEN("a pitch ratio set on one voice") {
        test_wrapper<procrastinate> an_instance;
        procrastinate&              my_object = an_instance;
        pin_all_voices(my_object, 100.0);
        my_object.shift_ratio(atoms{0, 2.0}); // voice 0 up an octave

        WHEN("a tone is processed") {
            THEN("the output stays finite — the phasor now runs") {
                for (int i = 0; i < 44100; ++i) {
                    const auto y = my_object(0.5 * std::sin(6.28318530717958647692 * 220.0 * i / k_mock_sr));
                    REQUIRE(std::isfinite(y[0]));
                    REQUIRE(std::isfinite(y[1]));
                    REQUIRE(std::abs(y[0]) < 8.0);
                }
            }
        }
    }
}

SCENARIO("tap.procrastinate~ clears its delay buffers") {
    ext_main(nullptr);

    GIVEN("a pinned machine holding an impulse") {
        test_wrapper<procrastinate> an_instance;
        procrastinate&              my_object = an_instance;
        pin_all_voices(my_object, 100.0);
        my_object(1.0);
        for (int i = 0; i < 1000; ++i) {
            my_object(0.0);
        }

        WHEN("clear is sent before the cascade completes") {
            my_object.clear();

            THEN("nothing emerges") {
                const long per_voice = static_cast<long>(0.5 * 100.0 * k_mock_sr * 0.001) - 1;
                for (long i = 0; i < 4 * per_voice + 64; ++i) {
                    const auto y = my_object(0.0);
                    REQUIRE(y[0] == 0.0);
                    REQUIRE(y[1] == 0.0);
                }
            }
        }
    }
}
