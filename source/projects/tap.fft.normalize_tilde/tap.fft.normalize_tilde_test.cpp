/// @file
/// @brief      Unit tests for tap.fft.normalize~.
/// @details    Instantiated without an argument, so the object runs at its 512-point default and the
///             scale factor is 1/256 — the numbers below are written against that.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2000-2026 Timothy Place.

#include <cmath>

#include "c74_min_unittest.h"          // required unit-test header (defines main via Catch)
#include "tap.fft.normalize_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_scale = 1.0 / 256.0; // 1/(fftsize/2) at the 512-point default

    struct frame {
        double real;
        double imag;
    };

    // Normalize one bin of one frame.
    frame bin(fft_normalize& object, double re, double im, double index) {
        double       r = re, i = im, x = index, o_re = 0.0, o_im = 0.0;
        double*      inp[3]  = {&r, &i, &x};
        double*      outp[2] = {&o_re, &o_im};
        audio_bundle ina{inp, 3, 1};
        audio_bundle outa{outp, 2, 1};
        object(ina, outa);
        return {o_re, o_im};
    }

} // namespace

SCENARIO("tap.fft.normalize~ scales the frame by 1/(fftsize/2)") {
    ext_main(nullptr);

    GIVEN("a default (512-point) instance") {
        test_wrapper<fft_normalize> an_instance;
        fft_normalize&              my_object = an_instance;

        THEN("an interior bin is scaled by 1/256") {
            const frame out = bin(my_object, 256.0, 0.0, 1.0);
            REQUIRE(std::abs(out.real - 1.0) < 1e-12);
        }
        THEN("the scaling is linear") {
            REQUIRE(std::abs(bin(my_object, 1.0, 0.0, 1.0).real - k_scale) < 1e-15);
            REQUIRE(std::abs(bin(my_object, -4.0, 0.0, 1.0).real + 4.0 * k_scale) < 1e-15);
        }
    }
}

SCENARIO("tap.fft.normalize~ negates the imaginary part") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<fft_normalize> an_instance;
        fft_normalize&              my_object = an_instance;

        THEN("the sign of the imaginary part is flipped as it is scaled") {
            const frame out = bin(my_object, 0.0, 256.0, 1.0);
            REQUIRE(std::abs(out.imag + 1.0) < 1e-12);
        }
        THEN("the imaginary part is never halved, even at DC") {
            const frame out = bin(my_object, 0.0, 256.0, 0.0);
            REQUIRE(std::abs(out.imag + 1.0) < 1e-12);
        }
    }
}

SCENARIO("tap.fft.normalize~ halves the DC and Nyquist bins") {
    ext_main(nullptr);

    GIVEN("a default (512-point) instance") {
        test_wrapper<fft_normalize> an_instance;
        fft_normalize&              my_object = an_instance;

        THEN("bin 0 is halved") {
            REQUIRE(std::abs(bin(my_object, 256.0, 0.0, 0.0).real - 0.5) < 1e-12);
        }
        THEN("the last bin is halved") {
            REQUIRE(std::abs(bin(my_object, 256.0, 0.0, 511.0).real - 0.5) < 1e-12);
        }
        THEN("the bins in between are not") {
            REQUIRE(std::abs(bin(my_object, 256.0, 0.0, 1.0).real - 1.0) < 1e-12);
            REQUIRE(std::abs(bin(my_object, 256.0, 0.0, 255.0).real - 1.0) < 1e-12);
            REQUIRE(std::abs(bin(my_object, 256.0, 0.0, 510.0).real - 1.0) < 1e-12);
        }
    }
}

SCENARIO("tap.fft.normalize~ rounds the bin index before comparing it") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<fft_normalize> an_instance;
        fft_normalize&              my_object = an_instance;

        // The @note in the header: the original compared the 0.49-biased index for equality, so its
        // DC/Nyquist halving never fired. Nudged indices must still resolve to the right bin.
        THEN("a fractional index rounds up onto the Nyquist bin and is halved") {
            REQUIRE(std::abs(bin(my_object, 256.0, 0.0, 510.6).real - 0.5) < 1e-12);
        }
        THEN("a fractional index rounds down onto DC and is halved") {
            REQUIRE(std::abs(bin(my_object, 256.0, 0.0, 0.4).real - 0.5) < 1e-12);
        }
        THEN("an index that rounds to an interior bin is left at full scale") {
            REQUIRE(std::abs(bin(my_object, 256.0, 0.0, 510.4).real - 1.0) < 1e-12);
        }
    }
}
