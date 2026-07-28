/// @file
/// @brief      Unit tests for tap.comb~.
/// @details    The feedback lowpass coefficient is derived from the sample rate, which Max supplies
///             through dspsetup — a freshly constructed object still carries the 0.1 placeholder. So
///             every scenario that measures sends `dspsetup` first, exactly as Max does when the DSP
///             chain starts, and after any delay change so the coefficient matches.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.comb_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    // update_lowpass(): hertz_to_radians(hz)/pi == hz * 2 / sr, clamped to [0, 1].
    constexpr double k_lp_coef = 20000.0 * 2.0 / k_mock_sr;

    // Run a block through the vector_operator. Only channel 0 is read unless a signal is connected
    // to the delay inlet, which the mock cannot report, so one channel is enough.
    std::vector<double> run(comb& object, const std::vector<double>& in) {
        std::vector<double> input = in;
        std::vector<double> output(in.size(), 0.0);
        double*             inp[1]  = {input.data()};
        double*             outp[1] = {output.data()};
        audio_bundle        ina{inp, 1, static_cast<long>(input.size())};
        audio_bundle        outa{outp, 1, static_cast<long>(output.size())};
        object(ina, outa);
        return output;
    }

    std::vector<double> impulse(size_t n) {
        std::vector<double> x(n, 0.0);
        x[0] = 1.0;
        return x;
    }

} // namespace

SCENARIO("tap.comb~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<comb> an_instance;
        comb&              my_object = an_instance;

        THEN("attribute defaults match the reference page") {
            REQUIRE(static_cast<double>(my_object.buffersize) == 200.0);
            REQUIRE(static_cast<double>(my_object.feedback) == 0.9);
            REQUIRE(static_cast<double>(my_object.delay) == 50.0);
            REQUIRE(static_cast<double>(my_object.decay) == 0.0);
            REQUIRE(static_cast<double>(my_object.lowpass) == 20000.0);
            REQUIRE(static_cast<bool>(my_object.autoclip) == false);
        }
    }
}

SCENARIO("tap.comb~ is a plain pass-through at zero feedback") {
    ext_main(nullptr);

    GIVEN("feedback 0") {
        test_wrapper<comb> an_instance;
        comb&              my_object = an_instance;
        my_object.feedback           = 0.0;
        my_object.dspsetup();

        WHEN("a ramp is processed") {
            std::vector<double> in(128);
            for (size_t i = 0; i < in.size(); ++i) {
                in[i] = static_cast<double>(i) / 128.0;
            }
            const std::vector<double> out = run(my_object, in);

            THEN("the output is the input — nothing is fed back") {
                for (size_t i = 0; i < in.size(); ++i) {
                    REQUIRE(out[i] == in[i]);
                }
            }
        }
    }
}

SCENARIO("tap.comb~ produces decaying echoes at the delay interval") {
    ext_main(nullptr);

    GIVEN("a 10 ms delay with 0.9 feedback") {
        test_wrapper<comb> an_instance;
        comb&              my_object = an_instance;
        my_object.delay              = 10.0;
        my_object.dspsetup();

        const long tap = static_cast<long>(10.0 * k_mock_sr * 0.001); // 441 samples

        WHEN("an impulse is processed") {
            const std::vector<double> out = run(my_object, impulse(2048));

            THEN("the impulse appears at the output immediately") {
                REQUIRE(out[0] == 1.0);
            }
            THEN("the first echo is the feedback coefficient through the feedback lowpass") {
                REQUIRE(std::abs(out[tap] - 0.9 * k_lp_coef) < 1e-12);
            }
            THEN("each successive echo is one more trip round the loop, so they decay") {
                REQUIRE(std::abs(out[2 * tap] - std::pow(0.9 * k_lp_coef, 2.0)) < 1e-9);
                REQUIRE(out[2 * tap] < out[tap]);
                REQUIRE(out[4 * tap] > 0.0);
                REQUIRE(out[4 * tap] < out[2 * tap]);
            }
            THEN("nothing rings between the taps") {
                REQUIRE(out[tap / 2] == 0.0);
            }
        }
    }
}

SCENARIO("tap.comb~ clears its delay line on demand") {
    ext_main(nullptr);

    GIVEN("a comb with an impulse already in the buffer") {
        test_wrapper<comb> an_instance;
        comb&              my_object = an_instance;
        my_object.delay              = 10.0;
        my_object.dspsetup();
        run(my_object, impulse(64));

        WHEN("clear is sent") {
            my_object.clear();

            THEN("no echo of the impulse survives") {
                const std::vector<double> out = run(my_object, std::vector<double>(2048, 0.0));
                for (double s : out) {
                    REQUIRE(s == 0.0);
                }
            }
        }
    }
}

SCENARIO("tap.comb~ couples delay and decay without blowing up") {
    ext_main(nullptr);

    GIVEN("a decay time of 1 second") {
        test_wrapper<comb> an_instance;
        comb&              my_object = an_instance;
        my_object.delay              = 10.0;
        my_object.decay              = 1.0;
        my_object.dspsetup();

        THEN("the feedback coefficient becomes the decay-time equivalent, not the 0.9 default") {
            // -60 dB over 1 s with a 10 ms tap: 10^((0.01/1 * -60)/20) == 0.9332543...
            const double expected = std::pow(10.0, ((0.01 / 1.0) * -60.0) / 20.0);
            REQUIRE(std::abs(static_cast<double>(my_object.feedback) - 0.9) < 1e-12); // the attribute is not rewritten
            const std::vector<double> out = run(my_object, impulse(2048));
            const long                tap = static_cast<long>(10.0 * k_mock_sr * 0.001);
            REQUIRE(std::abs(out[tap] - expected * k_lp_coef) < 1e-12); // but the *coefficient* did change
            REQUIRE(out[tap] > 0.9 * k_lp_coef);                        // louder than the default feedback
        }

        WHEN("the delay is changed while a decay time is set") {
            my_object.delay = 20.0;

            THEN("the filter stays stable — the decay coupling is guarded") {
                const std::vector<double> out  = run(my_object, impulse(8192));
                double                    peak = 0.0;
                for (double s : out) {
                    REQUIRE(std::isfinite(s));
                    peak = std::max(peak, std::abs(s));
                }
                REQUIRE(peak <= 1.0);
            }
        }
    }
}

SCENARIO("tap.comb~ autoclip bounds the feedback path") {
    ext_main(nullptr);

    GIVEN("unity feedback and autoclip enabled") {
        test_wrapper<comb> an_instance;
        comb&              my_object = an_instance;
        my_object.delay              = 5.0;
        my_object.feedback           = 1.0;
        my_object.autoclip           = true;
        my_object.dspsetup();

        WHEN("a sustained full-scale signal is processed") {
            const std::vector<double> out = run(my_object, std::vector<double>(44100, 1.0));

            THEN("the output stays finite and bounded by input + clip ceiling") {
                for (double s : out) {
                    REQUIRE(std::isfinite(s));
                    REQUIRE(std::abs(s) <= 2.0);
                }
            }
        }
    }
}
