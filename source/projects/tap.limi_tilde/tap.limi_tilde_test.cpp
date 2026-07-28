/// @file
/// @brief      Unit tests for tap.limi~.
/// @details    The limiter's gain history is initialized by reset(), which Max reaches through
///             dspsetup; a freshly constructed object has an all-zero gain buffer, so these
///             scenarios send `clear` before measuring — the same thing Max does when the DSP chain
///             starts.
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <algorithm>
#include <cmath>
#include <vector>

#include "c74_min_unittest.h" // required unit-test header (defines main via Catch)
#include "tap.limi_tilde.cpp" // include the object source so we can instantiate it

namespace {

    constexpr double k_mock_sr = 44100.0; // the mock kernel's sys_getsr()
    constexpr double k_two_pi  = 6.28318530717958647692;

    struct stereo {
        std::vector<double> left;
        std::vector<double> right;
    };

    // Run a stereo block through the vector_operator.
    stereo run(limi& object, const std::vector<double>& l, const std::vector<double>& r) {
        std::vector<double> il = l, ir = r;
        stereo              out{std::vector<double>(l.size(), 0.0), std::vector<double>(l.size(), 0.0)};
        double*             inp[2]  = {il.data(), ir.data()};
        double*             outp[2] = {out.left.data(), out.right.data()};
        audio_bundle        ina{inp, 2, static_cast<long>(l.size())};
        audio_bundle        outa{outp, 2, static_cast<long>(l.size())};
        object(ina, outa);
        return out;
    }

    std::vector<double> tone(size_t n, double freq_hz, double amp) {
        std::vector<double> x(n);
        for (size_t i = 0; i < n; ++i) {
            x[i] = amp * std::sin(k_two_pi * freq_hz * static_cast<double>(i) / k_mock_sr);
        }
        return x;
    }

    // Peak magnitude, skipping the first `skip` samples (the look-ahead buffer's fill-up).
    double peak(const std::vector<double>& x, size_t skip) {
        double p = 0.0;
        for (size_t i = skip; i < x.size(); ++i) {
            p = std::max(p, std::abs(x[i]));
        }
        return p;
    }

} // namespace

SCENARIO("tap.limi~ instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<limi> an_instance;
        limi&              my_object = an_instance;

        THEN("attribute defaults match the reference page") {
            REQUIRE(static_cast<double>(my_object.threshold) == 0.0);
            REQUIRE(static_cast<double>(my_object.preamp) == 0.0);
            REQUIRE(static_cast<double>(my_object.postamp) == 0.0);
            REQUIRE(static_cast<double>(my_object.release) == 1000.0);
            REQUIRE(static_cast<int>(my_object.lookahead) == 100);
            REQUIRE(my_object.mode == symbol{"exponential"});
            REQUIRE(static_cast<bool>(my_object.bypass_dcblocker) == false);
            REQUIRE(static_cast<bool>(my_object.bypass) == false);
            REQUIRE(static_cast<bool>(my_object.mute) == false);
        }
    }
}

SCENARIO("tap.limi~ clamps the look-ahead window to its documented 1-255 range") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<limi> an_instance;
        limi&              my_object = an_instance;

        THEN("zero and negative windows are floored at one sample") {
            my_object.lookahead = 0;
            REQUIRE(static_cast<int>(my_object.lookahead) == 1);
            my_object.lookahead = -20;
            REQUIRE(static_cast<int>(my_object.lookahead) == 1);
        }
        THEN("a window past the internal buffer is capped at 255") {
            my_object.lookahead = 4096;
            REQUIRE(static_cast<int>(my_object.lookahead) == 255);
        }
        THEN("a window inside the range is taken as given") {
            my_object.lookahead = 64;
            REQUIRE(static_cast<int>(my_object.lookahead) == 64);
        }
    }
}

SCENARIO("tap.limi~ mutes and bypasses") {
    ext_main(nullptr);

    GIVEN("a muted instance") {
        test_wrapper<limi> an_instance;
        limi&              my_object = an_instance;
        my_object.mute               = true;

        THEN("both channels are silent") {
            const stereo out = run(my_object, tone(512, 1000.0, 1.0), tone(512, 1000.0, 1.0));
            for (size_t i = 0; i < out.left.size(); ++i) {
                REQUIRE(out.left[i] == 0.0);
                REQUIRE(out.right[i] == 0.0);
            }
        }
    }

    GIVEN("a bypassed instance") {
        test_wrapper<limi> an_instance;
        limi&              my_object = an_instance;
        my_object.bypass             = true;

        THEN("the input arrives untouched — no gain, no delay, no DC blocking") {
            const std::vector<double> l   = tone(512, 1000.0, 1.0);
            const std::vector<double> r   = tone(512, 500.0, 0.5);
            const stereo              out = run(my_object, l, r);
            for (size_t i = 0; i < l.size(); ++i) {
                REQUIRE(out.left[i] == l[i]);
                REQUIRE(out.right[i] == r[i]);
            }
        }
    }
}

SCENARIO("tap.limi~ holds the output at the threshold") {
    ext_main(nullptr);

    GIVEN("a -6 dB threshold, the DC blocker out of the way, and a full-scale tone") {
        test_wrapper<limi> an_instance;
        limi&              my_object = an_instance;
        my_object.bypass_dcblocker   = true;
        my_object.threshold          = -6.0;
        my_object.clear(); // what Max's dspsetup does: seed the gain history at unity

        const double ceiling = std::pow(10.0, -6.0 * 0.05); // 0.5012

        WHEN("a 1 kHz tone at full scale is limited") {
            const stereo out = run(my_object, tone(4410, 1000.0, 1.0), tone(4410, 1000.0, 1.0));

            THEN("nothing exceeds the threshold") {
                REQUIRE(peak(out.left, 256) <= ceiling + 1e-12);
                REQUIRE(peak(out.right, 256) <= ceiling + 1e-12);
            }
            THEN("the signal is not simply thrown away — it sits just under the ceiling") {
                REQUIRE(peak(out.left, 256) > 0.9 * ceiling);
            }
        }
    }

    GIVEN("a 0 dB threshold and a half-scale tone that never reaches it") {
        test_wrapper<limi> an_instance;
        limi&              my_object = an_instance;
        my_object.bypass_dcblocker   = true;
        my_object.clear();

        WHEN("the tone is processed") {
            const stereo out = run(my_object, tone(4410, 1000.0, 0.5), tone(4410, 1000.0, 0.5));

            THEN("it passes at its own level — the limiter is transparent below threshold") {
                REQUIRE(peak(out.left, 256) > 0.49);
                REQUIRE(peak(out.left, 256) <= 0.5 + 1e-12);
            }
        }
    }
}

SCENARIO("tap.limi~ applies pre-gain before limiting and post-gain after") {
    ext_main(nullptr);

    GIVEN("a 0 dB threshold with 12 dB of pre-gain") {
        test_wrapper<limi> an_instance;
        limi&              my_object = an_instance;
        my_object.bypass_dcblocker   = true;
        my_object.preamp             = 12.0;
        my_object.clear();

        WHEN("a quarter-scale tone is driven into the limiter") {
            const stereo out = run(my_object, tone(4410, 1000.0, 0.25), tone(4410, 1000.0, 0.25));

            THEN("the pre-gain lifts it to full scale but the threshold still holds") {
                REQUIRE(peak(out.left, 256) > 0.9);
                REQUIRE(peak(out.left, 256) <= 1.0 + 1e-12);
            }
        }
    }
}

SCENARIO("tap.limi~ blocks DC by default") {
    ext_main(nullptr);

    GIVEN("a default instance fed a constant") {
        test_wrapper<limi> an_instance;
        limi&              my_object = an_instance;
        my_object.clear();

        WHEN("a half-scale DC offset is processed") {
            const stereo out = run(my_object, std::vector<double>(44100, 0.5), std::vector<double>(44100, 0.5));

            THEN("the offset decays away — the integrated DC blocker is in the path") {
                REQUIRE(std::abs(out.left.back()) < 0.01);
            }
        }
    }

    GIVEN("the DC blocker bypassed") {
        test_wrapper<limi> an_instance;
        limi&              my_object = an_instance;
        my_object.bypass_dcblocker   = true;
        my_object.clear();

        WHEN("a half-scale DC offset is processed") {
            const stereo out = run(my_object, std::vector<double>(44100, 0.5), std::vector<double>(44100, 0.5));

            THEN("the offset survives") {
                REQUIRE(std::abs(out.left.back() - 0.5) < 1e-9);
            }
        }
    }
}

SCENARIO("tap.limi~ accepts both documented recovery curves") {
    ext_main(nullptr);

    GIVEN("the linear recovery mode") {
        test_wrapper<limi> an_instance;
        limi&              my_object = an_instance;
        my_object.bypass_dcblocker   = true;
        my_object.mode               = "linear";
        my_object.clear();

        THEN("the threshold still holds and the output stays finite") {
            const stereo out = run(my_object, tone(4410, 1000.0, 1.0), tone(4410, 1000.0, 1.0));
            for (double s : out.left) {
                REQUIRE(std::isfinite(s));
            }
            REQUIRE(peak(out.left, 256) <= 1.0 + 1e-12);
        }
    }
}
