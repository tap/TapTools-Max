/// @file
/// @brief      Unit tests for tap.noise~.
/// @details    The white source is a deterministic LCG carried over from Jamoma's TTNoise, so the
///             white/pink/brown/blue paths can be pinned exactly rather than statistically. Only the
///             gaussian mode draws from std::mt19937, and it is tested by distribution.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <vector>

#include "c74_min_unittest.h"  // required unit-test header (defines main via Catch)
#include "tap.noise_tilde.cpp" // include the object source so we can instantiate it

namespace {

    // The LCG the white source runs, straight from the header's port of TTNoise.
    double lcg_white(long& accum) {
        accum = (accum * 3877 + 29573) % 139968;
        return 1.0 - (2.0 * static_cast<double>(accum) / 139968.0);
    }

    std::vector<double> generate(noise& object, size_t n) {
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            out[i] = object();
        }
        return out;
    }

    std::vector<double> generate_color(const char* mode, size_t n) {
        test_wrapper<noise> an_instance;
        noise&              object = an_instance;
        object.mode                = mode;
        return generate(object, n);
    }

    double mean_of(const std::vector<double>& x) {
        double acc = 0.0;
        for (double s : x) {
            acc += s;
        }
        return x.empty() ? 0.0 : acc / static_cast<double>(x.size());
    }

    double rms_of(const std::vector<double>& x) {
        double acc = 0.0;
        for (double s : x) {
            acc += s * s;
        }
        return x.empty() ? 0.0 : std::sqrt(acc / static_cast<double>(x.size()));
    }

    double stddev_of(const std::vector<double>& x) {
        const double m   = mean_of(x);
        double       acc = 0.0;
        for (double s : x) {
            acc += (s - m) * (s - m);
        }
        return x.empty() ? 0.0 : std::sqrt(acc / static_cast<double>(x.size()));
    }

    // Mean absolute first difference normalized by RMS: a level-independent measure of spectral
    // tilt. Low means energy concentrated at low frequencies, high means the opposite. This is the
    // instrument the coloring test measures with, rather than asserting filter internals.
    double spectral_tilt(const std::vector<double>& x) {
        double acc = 0.0;
        for (size_t i = 1; i < x.size(); ++i) {
            acc += std::abs(x[i] - x[i - 1]);
        }
        const double rms = rms_of(x);
        return rms > 0.0 ? (acc / static_cast<double>(x.size() - 1)) / rms : 0.0;
    }

} // namespace

SCENARIO("tap.noise~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<noise> an_instance;
        noise&              my_object = an_instance;

        THEN("attribute defaults match the reference page") {
            REQUIRE(my_object.mode == symbol{"white"});
            REQUIRE(static_cast<double>(my_object.gain) == 0.0);
            REQUIRE(static_cast<double>(my_object.mean) == 0.0);
            REQUIRE(static_cast<double>(my_object.deviation) == 1.0);
        }
    }
}

SCENARIO("tap.noise~ runs the documented white-noise LCG") {
    ext_main(nullptr);

    GIVEN("a default (white, 0 dB) instance") {
        test_wrapper<noise> an_instance;
        noise&              my_object = an_instance;

        THEN("the sequence matches the LCG sample for sample") {
            long accum = 0;
            for (int i = 0; i < 1024; ++i) {
                REQUIRE(my_object() == lcg_white(accum));
            }
        }
        THEN("it stays inside +/-1 across a full period of the LCG") {
            // The LCG's modulus is 139968 and it does reach 0, at which point the mapping
            // 1 - 2*accum/139968 yields exactly 1.0 — so the true range is (-1, 1], not (-1, 1).
            const std::vector<double> w = generate(my_object, 200000);
            for (double s : w) {
                REQUIRE(s > -1.0);
                REQUIRE(s <= 1.0);
            }
            REQUIRE(*std::max_element(w.begin(), w.end()) == 1.0);
        }
    }
}

SCENARIO("tap.noise~ scales its output by the gain attribute") {
    ext_main(nullptr);

    GIVEN("a white instance at -6 dB") {
        test_wrapper<noise> an_instance;
        noise&              my_object = an_instance;
        my_object.gain                = -6.0;

        THEN("every sample is the LCG value times the linear gain") {
            const double g     = std::pow(10.0, -6.0 * 0.05);
            long         accum = 0;
            for (int i = 0; i < 256; ++i) {
                REQUIRE(std::abs(my_object() - lcg_white(accum) * g) < 1e-15);
            }
        }
    }
}

SCENARIO("tap.noise~ tilts the spectrum by color") {
    ext_main(nullptr);

    GIVEN("20000 samples of each color from a fresh generator") {
        const std::vector<double> brown = generate_color("brown", 20000);
        const std::vector<double> pink  = generate_color("pink", 20000);
        const std::vector<double> white = generate_color("white", 20000);
        const std::vector<double> blue  = generate_color("blue", 20000);

        THEN("the colors are ordered brown, pink, white, blue from darkest to brightest") {
            REQUIRE(spectral_tilt(brown) < spectral_tilt(pink));
            REQUIRE(spectral_tilt(pink) < spectral_tilt(white));
            REQUIRE(spectral_tilt(white) < spectral_tilt(blue));
        }
        THEN("brown is bounded by its clipped integrator and 0.25 trim") {
            for (double s : brown) {
                REQUIRE(std::abs(s) <= 0.25);
            }
        }
        THEN("blue is bounded by its clip") {
            for (double s : blue) {
                REQUIRE(std::abs(s) <= 1.0);
            }
        }
        THEN("pink stays finite — its filter bank sums past unity, so it needs headroom") {
            for (double s : pink) {
                REQUIRE(std::isfinite(s));
                REQUIRE(std::abs(s) < 8.0);
            }
            REQUIRE(rms_of(pink) > rms_of(white)); // the 1/f tilt piles up low-frequency energy
        }
    }
}

SCENARIO("tap.noise~ clears the coloring filters") {
    ext_main(nullptr);

    GIVEN("a brown instance that has integrated for a while") {
        test_wrapper<noise> an_instance;
        noise&              my_object = an_instance;
        my_object.mode                = "brown";
        generate(my_object, 20000);

        WHEN("clear is sent") {
            my_object.clear();

            THEN("the next sample restarts from a zero filter state") {
                // Only the filter memory is cleared; the white LCG keeps running, so the first
                // sample after a clear is the scaled white value on its own: |white| * 0.1 * 0.25.
                REQUIRE(std::abs(my_object()) <= 0.025);
            }
        }
    }
}

SCENARIO("tap.noise~ draws gaussian noise from mean and deviation") {
    ext_main(nullptr);

    GIVEN("a gaussian instance with a shifted mean and a narrow deviation") {
        test_wrapper<noise> an_instance;
        noise&              my_object = an_instance;
        my_object.mode                = "gauss";
        my_object.mean                = 0.5;
        my_object.deviation           = 0.1;

        THEN("the sample distribution follows the requested parameters") {
            const std::vector<double> g = generate(my_object, 50000);
            REQUIRE(std::abs(mean_of(g) - 0.5) < 0.01);
            REQUIRE(std::abs(stddev_of(g) - 0.1) < 0.01);
        }
        THEN("gain does not apply to the gaussian path, as documented") {
            my_object.gain              = -60.0;
            const std::vector<double> g = generate(my_object, 50000);
            REQUIRE(std::abs(mean_of(g) - 0.5) < 0.01);
        }
    }
}

SCENARIO("tap.noise~ clamps the gaussian deviation to its documented range") {
    ext_main(nullptr);

    GIVEN("a gaussian instance") {
        test_wrapper<noise> an_instance;
        noise&              my_object = an_instance;
        my_object.mode                = "gauss";

        THEN("a deviation above 1 widens the distribution no further than 1") {
            my_object.deviation = 4.0;
            REQUIRE(static_cast<double>(my_object.deviation) == 4.0); // the raw value is echoed to Max
            REQUIRE(stddev_of(generate(my_object, 20000)) < 1.1);     // but the draw is clamped
        }
        THEN("a negative deviation collapses the distribution onto its mean") {
            my_object.deviation = -1.0;
            my_object.mean      = 0.25;
            for (double s : generate(my_object, 256)) {
                REQUIRE(std::abs(s - 0.25) < 1e-12);
            }
        }
    }
}
