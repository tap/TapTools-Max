/// @file
/// @brief      Unit tests for tap.shift~ (modernized engine).
/// @details    Frequency assertions use normalized frequency (radians/sample) so they hold at any
///             mock sample rate. The harness pins this translation unit to C++17.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min_unittest.h"
#include "tap.shift_tilde.cpp"

#include <cmath>
#include <vector>

namespace {

constexpr double c_tau = 6.28318530717958647692;
constexpr long   c_period = 96;   // input sine period in samples

// Goertzel power at a normalized frequency (radians/sample).
double goertzel_w(const std::vector<double>& x, size_t lo, size_t hi, double w) {
    const double coef = 2.0 * std::cos(w);
    double s1 = 0.0, s2 = 0.0;
    for (size_t i = lo; i < hi && i < x.size(); ++i) {
        const double s = x[i] + coef * s1 - s2;
        s2 = s1;
        s1 = s;
    }
    return s1 * s1 + s2 * s2 - coef * s1 * s2;
}

std::vector<double> run_sine(shift& object, size_t n, double amp = 0.5) {
    std::vector<double> out(n);
    for (size_t i = 0; i < n; ++i)
        out[i] = object(amp * std::sin(c_tau * static_cast<double>(i) / c_period));
    return out;
}

}  // namespace


SCENARIO("tap.shift~ transposes by the given ratio") {
    ext_main(nullptr);
    const double w = c_tau / c_period;

    GIVEN("ratio 2 (up an octave)") {
        test_wrapper<shift> an_instance;
        shift&              my_object = an_instance;
        my_object.smooth = 0.0;
        my_object.ratio  = 2.0;

        const size_t n = static_cast<size_t>(my_object.samplerate());   // 1 second
        auto out = run_sine(my_object, n);
        const size_t lo = n / 2;

        THEN("energy lands at twice the input frequency") {
            REQUIRE(goertzel_w(out, lo, n, 2.0 * w) > 5.0 * goertzel_w(out, lo, n, w));
        }
    }
    GIVEN("ratio 0.5 (down an octave)") {
        test_wrapper<shift> an_instance;
        shift&              my_object = an_instance;
        my_object.smooth = 0.0;
        my_object.ratio  = 0.5;

        const size_t n = static_cast<size_t>(my_object.samplerate());
        auto out = run_sine(my_object, n);
        const size_t lo = n / 2;

        THEN("energy lands at half the input frequency") {
            REQUIRE(goertzel_w(out, lo, n, 0.5 * w) > 5.0 * goertzel_w(out, lo, n, w));
        }
    }
    GIVEN("ratio 1 (no shift)") {
        test_wrapper<shift> an_instance;
        shift&              my_object = an_instance;
        my_object.smooth = 0.0;

        const size_t n = static_cast<size_t>(my_object.samplerate());
        auto out = run_sine(my_object, n);
        const size_t lo = n / 2;

        THEN("the input frequency passes with its level intact") {
            REQUIRE(goertzel_w(out, lo, n, w) > 100.0 * goertzel_w(out, lo, n, 1.5 * w));
            double rms = 0.0;
            for (size_t i = lo; i < n; ++i)
                rms += out[i] * out[i];
            rms = std::sqrt(rms / (n - lo));
            REQUIRE(rms > 0.9 * 0.5 / std::sqrt(2.0));
            REQUIRE(rms < 1.1 * 0.5 / std::sqrt(2.0));
        }
    }
}

SCENARIO("the complementary envelopes hold the level constant") {
    ext_main(nullptr);

    GIVEN("a DC input and a non-unity ratio") {
        test_wrapper<shift> an_instance;
        shift&              my_object = an_instance;
        my_object.smooth = 0.0;
        my_object.ratio  = 1.3;

        // fill the whole 200 ms buffer with DC first
        const size_t warm = static_cast<size_t>(0.3 * my_object.samplerate());
        for (size_t i = 0; i < warm; ++i)
            my_object(0.5);

        THEN("the output is exactly the DC level — no grain-rate amplitude ripple") {
            double max_err = 0.0;
            for (size_t i = 0; i < warm; ++i)
                max_err = std::max(max_err, std::abs(my_object(0.5) - 0.5));
            REQUIRE(max_err < 1e-9);
        }
    }
}

SCENARIO("parameter changes glide without clicks") {
    ext_main(nullptr);

    GIVEN("a running sine and a drastic window_size change") {
        test_wrapper<shift> an_instance;
        shift&              my_object = an_instance;
        my_object.ratio = 1.5;   // taps in motion; smooth stays at its 20 ms default

        const size_t settle = static_cast<size_t>(0.4 * my_object.samplerate());
        double       prev   = 0.0;
        for (size_t i = 0; i < settle; ++i)
            prev = my_object(0.5 * std::sin(c_tau * static_cast<double>(i) / c_period));

        my_object.window_size = 20.0;   // used to click: the taps jumped
        my_object.ratio       = 0.7;

        double maxjump = 0.0;
        for (size_t i = settle; i < settle + settle; ++i) {
            const double y = my_object(0.5 * std::sin(c_tau * static_cast<double>(i) / c_period));
            maxjump = std::max(maxjump, std::abs(y - prev));
            prev    = y;
        }
        THEN("no sample-to-sample discontinuity") {
            REQUIRE(maxjump < 0.2);
        }
    }
}

SCENARIO("defaults and clamping match the documented surface") {
    ext_main(nullptr);

    GIVEN("a default instance") {
        test_wrapper<shift> an_instance;
        shift&              my_object = an_instance;

        THEN("defaults are ratio 1, window 87 ms, smooth 20 ms") {
            REQUIRE(static_cast<double>(my_object.ratio) == 1.0);
            REQUIRE(static_cast<double>(my_object.window_size) == 87.0);
            REQUIRE(static_cast<double>(my_object.smooth) == 20.0);
        }
        THEN("window_size clamps below the buffer size") {
            my_object.window_size = 500.0;
            REQUIRE(static_cast<double>(my_object.window_size) < 200.0);
            my_object.window_size = 0.0;
            REQUIRE(static_cast<double>(my_object.window_size) == 1.0);
        }
        THEN("clear is callable") {
            my_object.clear(atoms {});
            REQUIRE(true);
        }
    }
}
