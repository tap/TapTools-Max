/// @file
/// @brief      Unit tests for tap.fft.binmodulator~.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min_unittest.h"             // required unit-test header (defines main via Catch)
#include "tap.fft.binmodulator_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()

    struct frame {
        double real;
        double imag;
    };

    // Process one single-sample "frame" for a given bin index. The LFOs advance once per frame, so a
    // frame count of 1 makes each call one LFO step — with vs == 1 the increment is freq/sr.
    frame step(fft_binmodulator& object, double re, double im, double bin) {
        double              r = re, i = im, b = bin;
        std::vector<double> or_(1, 0.0), oi(1, 0.0);
        double*             inp[3]  = {&r, &i, &b};
        double*             outp[2] = {or_.data(), oi.data()};
        audio_bundle        ina{inp, 3, 1};
        audio_bundle        outa{outp, 2, 1};
        object(ina, outa);
        return {or_[0], oi[0]};
    }

} // namespace

SCENARIO("tap.fft.binmodulator~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<fft_binmodulator> an_instance;
        fft_binmodulator&              my_object = an_instance;

        THEN("bypass is off") {
            REQUIRE(static_cast<bool>(my_object.bypass) == false);
        }
        THEN("every LFO depth starts at zero, so the frame is silenced") {
            const frame out = step(my_object, 1.0, -1.0, 0.0);
            REQUIRE(out.real == 0.0);
            REQUIRE(out.imag == 0.0);
        }
    }
}

SCENARIO("tap.fft.binmodulator~ passes the frame through when bypassed") {
    ext_main(nullptr);

    GIVEN("a bypassed instance with a fully-open LFO") {
        test_wrapper<fft_binmodulator> an_instance;
        fft_binmodulator&              my_object = an_instance;
        my_object.depth(atoms{0, 1.0});
        my_object.bypass = true;

        THEN("real and imaginary parts arrive untouched") {
            const frame out = step(my_object, 0.25, -0.75, 0.0);
            REQUIRE(out.real == 0.25);
            REQUIRE(out.imag == -0.75);
        }
    }
}

SCENARIO("tap.fft.binmodulator~ modulates a bin by its own LFO shape") {
    ext_main(nullptr);

    GIVEN("bin 0 at full depth with a stationary LFO (frequency 0)") {
        test_wrapper<fft_binmodulator> an_instance;
        fft_binmodulator&              my_object = an_instance;
        my_object.depth(atoms{0, 1.0});

        THEN("the default sine shape sits at its 0.5 midpoint at phase 0") {
            const frame out = step(my_object, 1.0, 2.0, 0.0);
            REQUIRE(std::abs(out.real - 0.5) < 1e-12);
            REQUIRE(std::abs(out.imag - 1.0) < 1e-12);
        }
        THEN("the square shape is fully open in the first half of its cycle") {
            my_object.shape(atoms{0, symbol("square")});
            const frame out = step(my_object, 1.0, 2.0, 0.0);
            REQUIRE(out.real == 1.0);
            REQUIRE(out.imag == 2.0);
        }
        THEN("the ramp shape starts fully closed") {
            my_object.shape(atoms{0, symbol("ramp")});
            const frame out = step(my_object, 1.0, 2.0, 0.0);
            REQUIRE(out.real == 0.0);
            REQUIRE(out.imag == 0.0);
        }
        THEN("a phase offset moves the sine to its peak") {
            my_object.phase(atoms{0, 0.25});
            const frame out = step(my_object, 1.0, 2.0, 0.0);
            REQUIRE(std::abs(out.real - 1.0) < 1e-12);
        }
        THEN("depth scales the modulation") {
            my_object.depth(atoms{0, 0.5});
            const frame out = step(my_object, 1.0, 2.0, 0.0);
            REQUIRE(std::abs(out.real - 0.25) < 1e-12);
        }
    }
}

SCENARIO("tap.fft.binmodulator~ advances each bin's LFO once per frame") {
    ext_main(nullptr);

    GIVEN("bin 0 with a frequency that advances a quarter cycle per single-sample frame") {
        test_wrapper<fft_binmodulator> an_instance;
        fft_binmodulator&              my_object = an_instance;
        my_object.depth(atoms{0, 1.0});
        my_object.frequency(atoms{0, 0.25 * k_mock_sr});

        THEN("the sine walks its midpoint-peak-midpoint-trough cycle") {
            REQUIRE(std::abs(step(my_object, 1.0, 0.0, 0.0).real - 1.0) < 1e-9); // phase 0.25
            REQUIRE(std::abs(step(my_object, 1.0, 0.0, 0.0).real - 0.5) < 1e-9); // phase 0.50
            REQUIRE(std::abs(step(my_object, 1.0, 0.0, 0.0).real - 0.0) < 1e-9); // phase 0.75
            REQUIRE(std::abs(step(my_object, 1.0, 0.0, 0.0).real - 0.5) < 1e-9); // phase 1.00 -> 0
        }
    }
}

SCENARIO("tap.fft.binmodulator~ keeps each bin's LFO independent") {
    ext_main(nullptr);

    GIVEN("bin 0 fully open and bin 1 fully closed") {
        test_wrapper<fft_binmodulator> an_instance;
        fft_binmodulator&              my_object = an_instance;
        my_object.depth(atoms{0, 1.0});
        my_object.shape(atoms{0, symbol("square")});
        my_object.depth(atoms{1, 0.0});

        THEN("only the bin that was opened passes") {
            REQUIRE(step(my_object, 1.0, 0.0, 0.0).real == 1.0);
            REQUIRE(step(my_object, 1.0, 0.0, 1.0).real == 0.0);
        }
    }

    GIVEN("depths set from a whole list rather than an [index value] pair") {
        test_wrapper<fft_binmodulator> an_instance;
        fft_binmodulator&              my_object = an_instance;
        my_object.depth(atoms{1.0, 0.5});
        my_object.shape(atoms{symbol("square"), symbol("square")});

        THEN("the list is distributed across the leading bins") {
            REQUIRE(step(my_object, 1.0, 0.0, 0.0).real == 1.0);
            REQUIRE(step(my_object, 1.0, 0.0, 1.0).real == 0.5);
        }
    }
}
