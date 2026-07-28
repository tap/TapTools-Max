/// @file
/// @brief      Unit tests for tap.fourpole~.
/// @details    The ladder coefficients are derived from the sample rate, which Max supplies through
///             dspsetup — a freshly constructed object has m_f == 0 and passes nothing. So every
///             scenario that measures sends `dspsetup` first, exactly as Max does when the DSP chain
///             starts.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2015-2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"     // required unit-test header (defines main via Catch)
#include "tap.fourpole_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()
    constexpr double k_two_pi  = 6.28318530717958647692;

    // Drive both channels with the same tone and report the steady-state RMS of the left output.
    double tone_rms(fourpole& object, double freq_hz, double amp = 0.5, double seconds = 0.25) {
        const size_t n     = static_cast<size_t>(seconds * k_mock_sr);
        const size_t start = n / 2; // skip the transient
        double       acc   = 0.0;
        size_t       count = 0;
        for (size_t i = 0; i < n; ++i) {
            const double x = amp * std::sin(k_two_pi * freq_hz * static_cast<double>(i) / k_mock_sr);
            const auto   y = object(x, x);
            if (i >= start) {
                acc += y[0] * y[0];
                ++count;
            }
        }
        return count ? std::sqrt(acc / static_cast<double>(count)) : 0.0;
    }

    // Steady-state response of a fresh, dsp-initialized filter at the default 1 kHz cutoff.
    double resp_at(double q_value, double freq_hz) {
        test_wrapper<fourpole> an_instance;
        fourpole&              object = an_instance;
        object.q                      = q_value;
        object.dspsetup();
        return tone_rms(object, freq_hz, 0.1);
    }

} // namespace

SCENARIO("tap.fourpole~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<fourpole> an_instance;
        fourpole&              my_object = an_instance;

        THEN("the cutoff is 1 kHz and resonance is off") {
            REQUIRE(static_cast<double>(my_object.frequency) == 1000.0);
            REQUIRE(static_cast<double>(my_object.q) == 0.0);
        }
    }
}

SCENARIO("tap.fourpole~ clamps resonance to the documented range") {
    ext_main(nullptr);

    // The setter clamps the coefficient it derives but echoes the raw value back to Max, so the
    // clamp is observable in the filter's response rather than in the attribute.
    GIVEN("one instance at q 1.0 and another at q 5.0") {
        test_wrapper<fourpole> a1;
        fourpole&              at_max = a1;
        at_max.q                      = 1.0;
        at_max.dspsetup();
        test_wrapper<fourpole> a2;
        fourpole&              over_max = a2;
        over_max.q                      = 5.0;
        over_max.dspsetup();

        THEN("both filter identically — anything above 1 is the same as 1") {
            REQUIRE(tone_rms(at_max, 1000.0, 0.1) == tone_rms(over_max, 1000.0, 0.1));
        }
    }

    GIVEN("one instance at q 0.0 and another at q -3.0") {
        test_wrapper<fourpole> a1;
        fourpole&              at_min = a1;
        at_min.dspsetup();
        test_wrapper<fourpole> a2;
        fourpole&              under_min = a2;
        under_min.q                      = -3.0;
        under_min.dspsetup();

        THEN("both filter identically — anything below 0 is the same as 0") {
            REQUIRE(tone_rms(at_min, 1000.0, 0.1) == tone_rms(under_min, 1000.0, 0.1));
        }
    }
}

SCENARIO("tap.fourpole~ is a lowpass") {
    ext_main(nullptr);

    GIVEN("the default 1 kHz cutoff") {
        test_wrapper<fourpole> a1;
        fourpole&              below = a1;
        below.dspsetup();
        test_wrapper<fourpole> a2;
        fourpole&              above = a2;
        above.dspsetup();

        const double passband = tone_rms(below, 100.0);
        const double stopband = tone_rms(above, 8000.0);

        THEN("three octaves above cutoff is far quieter than three octaves below") {
            REQUIRE(passband > 0.0);
            REQUIRE(stopband < 0.02 * passband); // > 34 dB of rejection
        }
    }
}

SCENARIO("tap.fourpole~ moves its corner with the frequency attribute") {
    ext_main(nullptr);

    GIVEN("the same 4 kHz tone at two cutoff settings") {
        test_wrapper<fourpole> a1;
        fourpole&              closed = a1;
        closed.frequency              = 500.0;
        closed.dspsetup();
        test_wrapper<fourpole> a2;
        fourpole&              open = a2;
        open.frequency              = 12000.0;
        open.dspsetup();

        THEN("opening the cutoff lets much more of the tone through") {
            REQUIRE(tone_rms(open, 4000.0) > 10.0 * tone_rms(closed, 4000.0));
        }
    }
}

SCENARIO("tap.fourpole~ resonance reshapes the response — and does not peak at the nominal cutoff") {
    ext_main(nullptr);

    GIVEN("the default 1 kHz cutoff measured with and without resonance") {
        THEN("resonance boosts the region just below the corner") {
            REQUIRE(resp_at(0.9, 500.0) > 1.4 * resp_at(0.0, 500.0));
        }
        THEN("resonance thins the deep passband, as a ladder does") {
            REQUIRE(resp_at(0.9, 100.0) < 0.4 * resp_at(0.0, 100.0));
        }
        // Honest limit, pinned rather than papered over: this is the Stilson/Smith one-pole-cascade
        // model, whose resonant peak sits well below the frequency it is nominally tuned to. At the
        // nominal cutoff itself, resonance does essentially nothing — the peak measured above is at
        // 500 Hz for a 1 kHz setting. Retuning the model would change this and should change the
        // numbers here with it.
        THEN("at the nominal cutoff the response barely moves with resonance") {
            const double flat     = resp_at(0.0, 1000.0);
            const double resonant = resp_at(0.9, 1000.0);
            REQUIRE(std::abs(resonant - flat) < 0.02 * flat);
        }
    }
}

SCENARIO("tap.fourpole~ processes its two channels independently and identically") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<fourpole> an_instance;
        fourpole&              my_object = an_instance;
        my_object.dspsetup();

        THEN("equal inputs give equal outputs") {
            for (int i = 0; i < 1024; ++i) {
                const double x = std::sin(k_two_pi * 300.0 * i / k_mock_sr);
                const auto   y = my_object(x, x);
                REQUIRE(y[0] == y[1]);
            }
        }
        THEN("silence on one channel does not leak from the other") {
            for (int i = 0; i < 1024; ++i) {
                const double x = std::sin(k_two_pi * 300.0 * i / k_mock_sr);
                const auto   y = my_object(x, 0.0);
                REQUIRE(y[1] == 0.0);
            }
        }
    }
}

SCENARIO("tap.fourpole~ clears its state on demand") {
    ext_main(nullptr);

    GIVEN("a filter that has been ringing") {
        test_wrapper<fourpole> an_instance;
        fourpole&              my_object = an_instance;
        my_object.q                      = 0.5;
        my_object.dspsetup();
        for (int i = 0; i < 4410; ++i) {
            my_object(0.5, -0.5);
        }

        WHEN("clear is sent") {
            my_object.clear();

            THEN("the filter is silent for silent input") {
                for (int i = 0; i < 1024; ++i) {
                    const auto y = my_object(0.0, 0.0);
                    REQUIRE(y[0] == 0.0);
                    REQUIRE(y[1] == 0.0);
                }
            }
        }
    }
}
