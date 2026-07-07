/// @file
/// @brief      Unit tests for tap.nr~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.nr_tilde.cpp"   // include the object source so we can instantiate it

namespace {
    // Run a whole signal through the object in one vector call; return the output.
    std::vector<double> run(nr& object, const std::vector<double>& in) {
        std::vector<double> input = in;
        std::vector<double> output(in.size(), 0.0);
        double*             inp[1]  = {input.data()};
        double*             outp[1] = {output.data()};
        audio_bundle        ina{inp, 1, static_cast<long>(input.size())};
        audio_bundle        outa{outp, 1, static_cast<long>(output.size())};
        object(ina, outa);
        return output;
    }
} // namespace

SCENARIO("tap.nr~ has the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<nr> an_instance;
        nr&              my_object = an_instance;

        THEN("threshold defaults to 0.01 and slope to 2") {
            REQUIRE(static_cast<double>(my_object.threshold) == 0.01);
            REQUIRE(static_cast<double>(my_object.slope) == 2.0);
        }
    }
}

SCENARIO("tap.nr~ reconstructs the input when the gate is open") {
    ext_main(nullptr);

    GIVEN("threshold 0 (gate disabled) and a 1 kHz tone") {
        test_wrapper<nr> an_instance;
        nr&              my_object = an_instance;
        my_object.threshold        = 0.0; // never gate -> pure STFT identity

        const int           N   = 1024; // default FFT size = latency in samples
        const int           len = 4096;
        std::vector<double> in(len);
        for (int i = 0; i < len; ++i) {
            in[i] = 0.5 * std::sin(2.0 * M_PI * 1000.0 * i / 44100.0);
        }

        const std::vector<double> out = run(my_object, in);

        THEN("the output equals the input delayed by one FFT frame") {
            double max_err = 0.0;
            for (int i = N + 64; i < len; ++i) {
                max_err = std::max(max_err, std::fabs(out[i] - in[i - N]));
            }
            REQUIRE(max_err < 1e-6);
        }
    }
}

SCENARIO("tap.nr~ attenuates signal below the threshold") {
    ext_main(nullptr);

    GIVEN("a tone quieter than the threshold") {
        test_wrapper<nr> an_instance;
        nr&              my_object = an_instance;
        my_object.threshold        = 0.01;
        my_object.slope            = 2.0;

        const int           len = 4096;
        std::vector<double> in(len);
        for (int i = 0; i < len; ++i) {
            in[i] = 1e-4 * std::sin(2.0 * M_PI * 1000.0 * i / 44100.0); // well below threshold
        }

        const std::vector<double> out = run(my_object, in);

        double in_peak = 0.0, out_peak = 0.0;
        for (int i = 2048; i < len; ++i) { // after the latency settles
            in_peak  = std::max(in_peak, std::fabs(in[i]));
            out_peak = std::max(out_peak, std::fabs(out[i]));
        }

        THEN("the quiet signal is strongly attenuated") {
            REQUIRE(out_peak < in_peak * 0.1);
        }
    }
}
