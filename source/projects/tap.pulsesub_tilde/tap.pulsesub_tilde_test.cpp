/// @file
/// @brief      Unit tests for tap.pulsesub~.
/// @details    The envelope step sizes are computed from the sample rate, which Max supplies through
///             dspsetup — so the scenarios that measure the envelope send `dspsetup` first, exactly
///             as Max does when the DSP chain starts.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2004-2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <vector>

#include "c74_min_unittest.h"     // required unit-test header (defines main via Catch)
#include "tap.pulsesub_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    // Drive the object with a constant 1.0 so the output *is* the envelope.
    std::vector<double> envelope(pulsesub& object, size_t n) {
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            out[i] = object(1.0);
        }
        return out;
    }

    double max_over(const std::vector<double>& x, size_t lo, size_t hi) {
        double m = 0.0;
        for (size_t i = lo; i < hi && i < x.size(); ++i) {
            m = std::max(m, x[i]);
        }
        return m;
    }

} // namespace

SCENARIO("tap.pulsesub~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<pulsesub> an_instance;
        pulsesub&              my_object = an_instance;

        THEN("attribute defaults match the reference page") {
            REQUIRE(static_cast<double>(my_object.frequency) == 1.0);
            REQUIRE(static_cast<double>(my_object.length) == 0.25);
            REQUIRE(static_cast<double>(my_object.attack) == 50.0);
            REQUIRE(static_cast<double>(my_object.decay) == 100.0);
            REQUIRE(static_cast<double>(my_object.sustain) == -6.0);
            REQUIRE(static_cast<double>(my_object.release) == 500.0);
            REQUIRE(my_object.mode == symbol{"linear"});
        }
    }
}

SCENARIO("tap.pulsesub~ clamps its envelope times to 1-60000 ms") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<pulsesub> an_instance;
        pulsesub&              my_object = an_instance;

        THEN("zero-length stages are floored at 1 ms") {
            my_object.attack = 0.0;
            REQUIRE(static_cast<double>(my_object.attack) == 1.0);
            my_object.decay = -10.0;
            REQUIRE(static_cast<double>(my_object.decay) == 1.0);
            my_object.release = 0.0;
            REQUIRE(static_cast<double>(my_object.release) == 1.0);
        }
        THEN("stages longer than a minute are capped at 60000 ms") {
            my_object.attack  = 120000.0;
            my_object.decay   = 120000.0;
            my_object.release = 120000.0;
            REQUIRE(static_cast<double>(my_object.attack) == 60000.0);
            REQUIRE(static_cast<double>(my_object.decay) == 60000.0);
            REQUIRE(static_cast<double>(my_object.release) == 60000.0);
        }
    }
}

SCENARIO("tap.pulsesub~ keeps the gate shut for the duty cycle it is not open") {
    ext_main(nullptr);

    GIVEN("the default 1 Hz pulse with a 0.25 duty cycle") {
        test_wrapper<pulsesub> an_instance;
        pulsesub&              my_object = an_instance;
        my_object.dspsetup();

        WHEN("one second is processed") {
            const std::vector<double> env = envelope(my_object, static_cast<size_t>(k_mock_sr));

            // The phasor ramp triggers the ADSR only while (ramp + length - 0.5) > 0.5, i.e. while
            // ramp > 0.75 — the last quarter of every cycle.
            const size_t gate_opens = static_cast<size_t>(0.75 * k_mock_sr);

            THEN("nothing passes for the first three quarters of the cycle") {
                // The phasor accumulates 1/sr per sample, so the sample the ramp first exceeds 0.75
                // on is only pinned to within one; the assertions leave that one sample out.
                for (size_t i = 0; i < gate_opens - 1; ++i) {
                    REQUIRE(env[i] == 0.0);
                }
            }
            THEN("the envelope opens once the ramp crosses the duty-cycle threshold") {
                REQUIRE(env[gate_opens + 1] > 0.0);
            }
        }
    }

    GIVEN("a 0.75 duty cycle") {
        test_wrapper<pulsesub> an_instance;
        pulsesub&              my_object = an_instance;
        my_object.length                 = 0.75;
        my_object.dspsetup();

        WHEN("one second is processed") {
            const std::vector<double> env        = envelope(my_object, static_cast<size_t>(k_mock_sr));
            const size_t              gate_opens = static_cast<size_t>(0.25 * k_mock_sr);

            THEN("the gate opens three quarters of the way earlier") {
                REQUIRE(env[gate_opens - 2] == 0.0);
                REQUIRE(env[gate_opens + 2] > 0.0);
            }
        }
    }
}

SCENARIO("tap.pulsesub~ runs a full attack-decay-sustain through the open gate") {
    ext_main(nullptr);

    GIVEN("the default 1 Hz pulse with a 0.25 duty cycle") {
        test_wrapper<pulsesub> an_instance;
        pulsesub&              my_object = an_instance;
        my_object.dspsetup();

        WHEN("one second is processed") {
            const std::vector<double> env        = envelope(my_object, static_cast<size_t>(k_mock_sr));
            const size_t              gate_opens = static_cast<size_t>(0.75 * k_mock_sr);

            THEN("the 50 ms attack reaches full scale, and does it in 50 ms") {
                REQUIRE(max_over(env, gate_opens, env.size()) == 1.0);
                const size_t peak_at = static_cast<size_t>(std::max_element(env.begin(), env.end()) - env.begin());
                REQUIRE(peak_at > gate_opens);
                REQUIRE(peak_at <= gate_opens + static_cast<size_t>(0.05 * k_mock_sr) + 4);
            }
            THEN("the 100 ms decay lands on the -6 dB sustain level and holds there") {
                const double sustain = std::pow(10.0, -6.0 * 0.05);
                REQUIRE(std::abs(env.back() - sustain) < 1e-12);
                const size_t decay_done = gate_opens + static_cast<size_t>(0.16 * k_mock_sr);
                REQUIRE(std::abs(env[decay_done] - sustain) < 1e-12);
            }
        }
    }
}

SCENARIO("tap.pulsesub~ releases when the gate closes again") {
    ext_main(nullptr);

    GIVEN("a 10 ms release so the tail is short compared to the cycle") {
        test_wrapper<pulsesub> an_instance;
        pulsesub&              my_object = an_instance;
        my_object.release                = 10.0;
        my_object.dspsetup();

        WHEN("two cycles are processed") {
            const std::vector<double> env = envelope(my_object, static_cast<size_t>(2.0 * k_mock_sr));

            THEN("the envelope is back to zero shortly after the cycle turns over") {
                const size_t after_close = static_cast<size_t>(k_mock_sr) + static_cast<size_t>(0.02 * k_mock_sr);
                REQUIRE(env[after_close] == 0.0);
            }
            THEN("and it opens again on the next cycle") {
                REQUIRE(max_over(env, static_cast<size_t>(1.8 * k_mock_sr), env.size()) > 0.5);
            }
        }
    }
}

SCENARIO("tap.pulsesub~ multiplies the input rather than replacing it") {
    ext_main(nullptr);

    GIVEN("an instance driven past its attack") {
        test_wrapper<pulsesub> an_instance;
        pulsesub&              my_object = an_instance;
        my_object.dspsetup();
        for (size_t i = 0; i < static_cast<size_t>(0.85 * k_mock_sr); ++i) {
            my_object(1.0);
        }

        THEN("silence in is silence out, whatever the envelope is doing") {
            REQUIRE(my_object(0.0) == 0.0);
        }
        THEN("the sign and scale of the input survive") {
            REQUIRE(my_object(-0.5) < 0.0);
            REQUIRE(std::abs(my_object(0.25)) < 0.25 + 1e-12);
        }
    }
}

SCENARIO("tap.pulsesub~ offers an exponential curve as well as a linear one") {
    ext_main(nullptr);

    GIVEN("the exponential mode") {
        test_wrapper<pulsesub> an_instance;
        pulsesub&              my_object = an_instance;
        my_object.mode                   = "exponential";
        my_object.dspsetup();

        WHEN("one second is processed") {
            const std::vector<double> env        = envelope(my_object, static_cast<size_t>(k_mock_sr));
            const size_t              gate_opens = static_cast<size_t>(0.75 * k_mock_sr);

            THEN("the gate still holds shut for the closed part of the cycle") {
                for (size_t i = 0; i < gate_opens - 1; ++i) {
                    REQUIRE(env[i] == 0.0);
                }
            }
            THEN("it reaches full scale and settles on the sustain level") {
                REQUIRE(max_over(env, gate_opens, env.size()) == 1.0);
                REQUIRE(std::abs(env.back() - std::pow(10.0, -6.0 * 0.05)) < 1e-12);
            }
            THEN("the exponential attack is behind the linear one at the same elapsed time") {
                test_wrapper<pulsesub> linear_instance;
                pulsesub&              linear = linear_instance;
                linear.dspsetup();
                const std::vector<double> lin = envelope(linear, static_cast<size_t>(k_mock_sr));

                const size_t quarter_through = gate_opens + static_cast<size_t>(0.0125 * k_mock_sr);
                REQUIRE(env[quarter_through] < lin[quarter_through]);
            }
        }
    }
}
