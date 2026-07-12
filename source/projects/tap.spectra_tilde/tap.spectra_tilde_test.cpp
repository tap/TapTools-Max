/// @file
/// @brief      Unit tests for tap.spectra~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"    // required unit-test header (defines main via Catch)
#include "tap.spectra_tilde.cpp" // include the object source so we can instantiate it

namespace {
    std::vector<double> run(spectra& object, const std::vector<double>& in) {
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

SCENARIO("tap.spectra~ defaults to the identity remap") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<spectra> an_instance;
        spectra&              my_object = an_instance;

        THEN("remap defaults to 1.0") {
            REQUIRE(static_cast<double>(my_object.remap) == 1.0);
        }
    }
}

SCENARIO("tap.spectra~ reconstructs the input at the identity remap") {
    ext_main(nullptr);

    GIVEN("remap 1.0 and a 1 kHz tone") {
        test_wrapper<spectra> an_instance;
        spectra&              my_object = an_instance;
        my_object.remap                 = 1.0;

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

SCENARIO("tap.spectra~ remaps the spectrum when remap != 1") {
    ext_main(nullptr);

    GIVEN("remap 2.0 driving a 1 kHz tone (its energy should move off the 1 kHz bin)") {
        test_wrapper<spectra> an_instance;
        spectra&              my_object = an_instance;
        my_object.remap                 = 2.0;

        const int           len  = 8192;
        const double        freq = 1000.0;
        std::vector<double> in(len);
        for (int i = 0; i < len; ++i) {
            in[i] = 0.5 * std::sin(2.0 * M_PI * freq * i / 44100.0);
        }

        const std::vector<double> out = run(my_object, in);

        // Correlate the settled output against the original 1 kHz tone. With the spectrum
        // remapped (bin k <- 2k), the 1 kHz component is moved, so correlation should be weak.
        double corr = 0.0, ref = 0.0;
        for (int i = 4096; i < len; ++i) {
            const double s = 0.5 * std::sin(2.0 * M_PI * freq * i / 44100.0);
            corr += out[i] * s;
            ref += s * s;
        }
        const double normalized = std::fabs(corr) / ref;

        THEN("little of the original 1 kHz tone remains") {
            REQUIRE(normalized < 0.25);
        }
    }
}
