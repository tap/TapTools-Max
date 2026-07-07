/// @file
/// @brief      Unit tests for tap.filter~.
/// @details    The mock kernel reports a samplerate of 44100 Hz (see c74_mock.cpp / sys_getsr),
///             so the object's samplerate() returns 44100 and the expected coefficient numbers
///             below are derived from the RBJ Audio EQ Cookbook formulas at Fs = 44100.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 1999-2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h"   // required unit-test header (defines main via Catch)
#include "tap.filter_tilde.cpp" // include the object source so we can instantiate it

// Run a coefficient set as a Transposed Direct Form II biquad over a block of input, returning the
// last output sample (after the filter has settled). Mirrors the runtime in operator().
static double run_tdf2(const filter::coeffs& c, const std::vector<double>& in) {
    double z1 = 0.0, z2 = 0.0, y = 0.0;
    for (double x : in) {
        y  = c.b0 * x + z1;
        z1 = c.b1 * x - c.a1 * y + z2;
        z2 = c.b2 * x - c.a2 * y;
    }
    return y;
}

SCENARIO("tap.filter~ instantiates with the expected defaults") {
    ext_main(nullptr); // configure the class (required once per test executable)

    GIVEN("a default instance") {
        test_wrapper<filter> an_instance;
        filter&              my_object = an_instance;

        THEN("the mode defaults to lowpass") {
            REQUIRE(my_object.mode == symbol{"lowpass"});
        }
        THEN("the frequency defaults to 1000 Hz") {
            REQUIRE(static_cast<double>(my_object.frequency) == 1000.0);
        }
        THEN("the q defaults to 0.707") {
            REQUIRE(static_cast<double>(my_object.q) == Approx(0.707));
        }
        THEN("the gain defaults to 0 dB") {
            REQUIRE(static_cast<double>(my_object.gain) == 0.0);
        }
    }
}

SCENARIO("tap.filter~ computes the RBJ lowpass coefficients") {
    ext_main(nullptr);

    GIVEN("a default lowpass at 1000 Hz, Q = 0.707, Fs = 44100") {
        test_wrapper<filter> an_instance;
        filter&              my_object = an_instance;

        auto c = my_object.compute(1000.0);

        THEN("the normalized coefficients match the cookbook") {
            REQUIRE(c.b0 == Approx(0.004603935028493071));
            REQUIRE(c.b1 == Approx(0.009207870056986141));
            REQUIRE(c.b2 == Approx(0.004603935028493071));
            REQUIRE(c.a1 == Approx(-1.799071616595651));
            REQUIRE(c.a2 == Approx(0.8174873567096231));
        }
    }
}

SCENARIO("tap.filter~ lowpass passes DC and attenuates high frequencies") {
    ext_main(nullptr);

    GIVEN("a default lowpass (1000 Hz, Q = 0.707)") {
        test_wrapper<filter> an_instance;
        filter&              my_object = an_instance;

        auto c = my_object.compute(1000.0);

        WHEN("a DC (unity) signal is filtered") {
            std::vector<double> dc(4096, 1.0);
            double              settled = run_tdf2(c, dc);
            THEN("the output settles to ~unity (the lowpass passes DC)") {
                REQUIRE(settled == Approx(1.0).epsilon(0.001));
            }
        }

        WHEN("a high-frequency (Nyquist-quarter) signal is filtered") {
            // alternating-ish high tone near 11 kHz at Fs = 44100; expect strong attenuation.
            std::vector<double> hi(8192);
            const double        w = 2.0 * M_PI * 11025.0 / 44100.0;
            for (size_t i = 0; i < hi.size(); ++i) {
                hi[i] = std::sin(w * static_cast<double>(i));
            }

            double peak_out = 0.0;
            double z1 = 0.0, z2 = 0.0;
            for (size_t i = 0; i < hi.size(); ++i) {
                double x = hi[i];
                double y = c.b0 * x + z1;
                z1       = c.b1 * x - c.a1 * y + z2;
                z2       = c.b2 * x - c.a2 * y;
                if (i > 2048 && std::abs(y) > peak_out) { // measure after settling
                    peak_out = std::abs(y);
                }
            }
            THEN("the high-frequency content is strongly attenuated (< 0.1 of input peak)") {
                REQUIRE(peak_out < 0.1);
            }
        }
    }
}

SCENARIO("tap.filter~ recomputes coefficients when the mode changes") {
    ext_main(nullptr);

    GIVEN("a lowpass and a highpass at the same frequency/Q") {
        test_wrapper<filter> an_instance;
        filter&              my_object = an_instance;

        auto lp = my_object.compute(1000.0);

        WHEN("the mode is switched to highpass") {
            my_object.mode = symbol("highpass");
            auto hp        = my_object.compute(1000.0);

            THEN("the feedforward coefficients differ from the lowpass set") {
                REQUIRE(hp.b0 != Approx(lp.b0));
                REQUIRE(hp.b1 != Approx(lp.b1));
            }
            THEN("the highpass blocks DC (its feedforward coefficients sum to ~0)") {
                REQUIRE((hp.b0 + hp.b1 + hp.b2) == Approx(0.0).margin(1e-9));
            }
            THEN("the feedback (denominator) coefficients are unchanged across the two modes") {
                REQUIRE(hp.a1 == Approx(lp.a1));
                REQUIRE(hp.a2 == Approx(lp.a2));
            }
        }
    }
}
