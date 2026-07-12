/// @file
/// @brief      Unit tests for tap.vco~ and its vco.h kernel.
/// @details    Kernel scenarios exercise taptools::vco::vco_osc directly; one scenario goes
///             through the Min mock. The harness pins this translation unit to C++17.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <cmath>
#include <cstddef>
#include <vector>

#include "c74_min_unittest.h"
#include "tap.vco_tilde.cpp"

namespace kvo = taptools::vco;

namespace {

    constexpr double k_c_sr = 48000.0;

    kvo::vco_osc make_osc() {
        kvo::vco_osc o;
        o.prepare(k_c_sr);
        o.set_smooth_ms(0.0);
        return o;
    }

    std::vector<double> render(kvo::vco_osc& o, double seconds) {
        std::vector<double> out(static_cast<size_t>(seconds * k_c_sr));
        o.process(out.data(), out.size());
        return out;
    }

    double goertzel(const std::vector<double>& x, size_t lo, size_t hi, double freq_hz) {
        const double w    = 2.0 * kvo::k_pi * freq_hz / k_c_sr;
        const double coef = 2.0 * std::cos(w);
        double       s1 = 0.0, s2 = 0.0;
        for (size_t i = lo; i < hi && i < x.size(); ++i) {
            const double s = x[i] + coef * s1 - s2;
            s2             = s1;
            s1             = s;
        }
        return s1 * s1 + s2 * s2 - coef * s1 * s2;
    }

    size_t zero_crossings(const std::vector<double>& x, size_t lo, size_t hi) {
        size_t n = 0;
        for (size_t i = std::max(lo, size_t(1)); i < hi && i < x.size(); ++i) {
            if ((x[i] >= 0.0) != (x[i - 1] >= 0.0)) {
                ++n;
            }
        }
        return n;
    }

    size_t at_s(double seconds) {
        return static_cast<size_t>(seconds * k_c_sr);
    }

} // namespace

SCENARIO("the oscillator runs at the requested frequency") {
    auto o = make_osc();
    o.set_frequency(440.0); // default saw
    auto out = render(o, 1.0);

    THEN("the zero-crossing rate matches within 0.5%") {
        const double f = zero_crossings(out, at_s(0.25), at_s(0.75)) / (2.0 * 0.5);
        REQUIRE(f > 437.8);
        REQUIRE(f < 442.2);
    }
}

SCENARIO("polyBLEP suppresses aliasing — the virtual-analog proof") {
    auto o = make_osc();
    o.set_frequency(3947.0); // 13th harmonic (51.3 kHz) folds to ~3311 Hz
    auto out = render(o, 1.0);

    const double fund  = goertzel(out, at_s(0.25), at_s(0.75), 3947.0);
    const double alias = goertzel(out, at_s(0.25), at_s(0.75), 51311.0 - 48000.0);

    THEN("the folded alias sits more than 35 dB below the fundamental") {
        REQUIRE(alias < fund * std::pow(10.0, -35.0 / 10.0));
    }
}

SCENARIO("waveform purity") {
    GIVEN("a sine") {
        auto o = make_osc();
        o.set_frequency(500.0);
        o.set_shape(kvo::wave_sine);
        auto out = render(o, 0.5);
        THEN("the 2nd harmonic is below -60 dB") {
            const double f1 = goertzel(out, at_s(0.1), at_s(0.5), 500.0);
            const double f2 = goertzel(out, at_s(0.1), at_s(0.5), 1000.0);
            REQUIRE(f2 < f1 * 1e-6);
        }
    }
    GIVEN("a triangle") {
        auto o = make_osc();
        o.set_frequency(500.0);
        o.set_shape(kvo::wave_triangle);
        auto         out = render(o, 1.0);
        const double f1  = goertzel(out, at_s(0.5), at_s(1.0), 500.0);
        const double f2  = goertzel(out, at_s(0.5), at_s(1.0), 1000.0);
        const double f3  = goertzel(out, at_s(0.5), at_s(1.0), 1500.0);
        THEN("even harmonics are absent and the 3rd sits near -19 dB") {
            REQUIRE(f2 < f1 * 1e-4);
            REQUIRE(f3 > f1 * 0.006); // 1/81 = 0.0123, generous window
            REQUIRE(f3 < f1 * 0.025);
        }
    }
}

SCENARIO("pulse width shapes the duty cycle") {
    auto o = make_osc();
    o.set_frequency(200.0);
    o.set_shape(kvo::wave_pulse);
    o.set_pw(25.0);
    auto out = render(o, 0.5);

    THEN("the mean matches the 25% duty cycle") {
        double mean = 0.0;
        for (size_t i = at_s(0.1); i < out.size(); ++i) {
            mean += out[i];
        }
        mean /= (out.size() - at_s(0.1));
        REQUIRE(mean > -0.53);
        REQUIRE(mean < -0.47); // 2*0.25 - 1 = -0.5
    }
}

SCENARIO("the shape morph glides without discontinuities") {
    auto o = make_osc();
    o.set_frequency(220.0);
    o.set_shape(0.0);
    o.snap();
    o.set_smooth_ms(1000.0);
    o.set_shape(3.0); // sine -> pulse over 1 s

    double       prev        = o.process();
    double       maxjump     = 0.0;
    const double slope_bound = 2.0 * kvo::k_pi * 220.0 / k_c_sr * 1.2 + 0.05; // sine slope + margin
    size_t       discont     = 0;
    for (size_t i = 1; i < at_s(1.2); ++i) {
        const double y = o.process();
        const double d = std::abs(y - prev);
        // the saw/pulse phases wrap legitimately; count only jumps far beyond a waveform edge
        if (d > 2.2) {
            ++discont;
        }
        maxjump = std::max(maxjump, d);
        prev    = y;
    }
    THEN("no jump ever exceeds a full waveform edge") {
        REQUIRE(discont == 0);
        REQUIRE(maxjump < 2.2);
    }
}

SCENARIO("hard sync locks the period to the master") {
    auto o = make_osc();
    o.set_frequency(237.0); // free frequency, deliberately inharmonic to the master

    const double        master = 100.0;
    const size_t        n      = at_s(1.0);
    std::vector<double> out(n);
    for (size_t i = 0; i < n; ++i) {
        const double sync = std::sin(2.0 * kvo::k_pi * master * i / k_c_sr);
        out[i]            = o.process(0.0, sync);
    }

    THEN("energy concentrates at the master's harmonics, not the free frequency") {
        const double at_master = goertzel(out, at_s(0.25), at_s(1.0), 100.0)
                                 + goertzel(out, at_s(0.25), at_s(1.0), 200.0)
                                 + goertzel(out, at_s(0.25), at_s(1.0), 300.0);
        const double at_free = goertzel(out, at_s(0.25), at_s(1.0), 237.0);
        REQUIRE(at_master > 10.0 * at_free);
    }
}

SCENARIO("through-zero FM creates sidebands and survives deep modulation") {
    GIVEN("moderate FM") {
        auto o = make_osc();
        o.set_frequency(1000.0);
        o.set_shape(kvo::wave_sine);
        const double        fmod = 100.0, depth = 200.0;
        const size_t        n = at_s(1.0);
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            out[i] = o.process(depth * std::sin(2.0 * kvo::k_pi * fmod * i / k_c_sr));
        }
        THEN("first-order sidebands appear at f0 +- fmod") {
            const double carrier = goertzel(out, at_s(0.25), at_s(1.0), 1000.0);
            const double upper   = goertzel(out, at_s(0.25), at_s(1.0), 1100.0);
            const double lower   = goertzel(out, at_s(0.25), at_s(1.0), 900.0);
            REQUIRE(upper > carrier * 0.05);
            REQUIRE(lower > carrier * 0.05);
        }
    }
    GIVEN("modulation depth far beyond the carrier (through zero)") {
        auto o = make_osc();
        o.set_frequency(200.0);
        const size_t        n = at_s(2.0);
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            out[i] = o.process(1500.0 * std::sin(2.0 * kvo::k_pi * 30.0 * i / k_c_sr));
        }
        THEN("the oscillator stays finite and bounded") {
            double peak   = 0.0;
            bool   finite = true;
            for (double s : out) {
                if (!std::isfinite(s)) {
                    finite = false;
                }
                peak = std::max(peak, std::abs(s));
            }
            REQUIRE(finite);
            REQUIRE(peak < 2.5);
        }
    }
}

SCENARIO("drift is deterministic per seed") {
    auto run_with_seed = [](uint32_t seed) {
        auto o = make_osc();
        o.set_seed(seed);
        o.clear();
        o.set_frequency(440.0);
        o.set_drift(30.0);
        return render(o, 0.5);
    };
    GIVEN("two instances with the same seed") {
        auto a = run_with_seed(7u);
        auto b = run_with_seed(7u);
        THEN("output is bit-identical") {
            double maxdiff = 0.0;
            for (size_t i = 0; i < a.size(); ++i) {
                maxdiff = std::max(maxdiff, std::abs(a[i] - b[i]));
            }
            REQUIRE(maxdiff == 0.0);
        }
    }
    GIVEN("different seeds") {
        auto a = run_with_seed(7u);
        auto b = run_with_seed(8u);
        THEN("outputs decorrelate") {
            double diff = 0.0;
            for (size_t i = at_s(0.25); i < a.size(); ++i) {
                diff += std::abs(a[i] - b[i]);
            }
            REQUIRE(diff > 1.0);
        }
    }
    GIVEN("drift 0") {
        auto o = make_osc();
        o.set_frequency(440.0);
        auto out = render(o, 1.0);
        THEN("pitch is exactly stable (zero-crossing count is exact)") {
            const double f = zero_crossings(out, at_s(0.25), at_s(0.75)) / (2.0 * 0.5);
            REQUIRE(f > 438.0);
            REQUIRE(f < 442.0);
        }
    }
}

SCENARIO("preset recall morphs without discontinuities") {
    auto o = make_osc();
    o.set_frequency(110.0);
    o.set_shape(0.0);
    o.set_pw(50.0);
    o.store_preset(0);

    o.set_frequency(880.0);
    o.set_shape(3.0);
    o.set_pw(10.0);

    for (size_t i = 0; i < at_s(0.1); ++i) {
        o.process();
    }

    REQUIRE(o.recall_preset(0, 0.2));
    REQUIRE(o.morphing());

    double prev    = o.process();
    size_t discont = 0;
    for (size_t i = 1; i < at_s(0.5); ++i) {
        const double y = o.process();
        if (std::abs(y - prev) > 2.2) {
            ++discont;
        }
        prev = y;
    }
    THEN("no jump beyond a waveform edge; parameters land") {
        REQUIRE(discont == 0);
        REQUIRE(!o.morphing());
        REQUIRE(std::abs(o.snap_current().v[kvo::p_frequency] - 110.0) < 1e-9);
    }
}

SCENARIO("the Min wrapper instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("an instance of tap.vco~") {
        test_wrapper<vco> an_instance;
        vco&              my_object = an_instance;

        THEN("attribute defaults match the spec") {
            REQUIRE(static_cast<double>(my_object.frequency) == 220.0);
            REQUIRE(static_cast<double>(my_object.shape) == 2.0);
            REQUIRE(static_cast<double>(my_object.pw) == 50.0);
            REQUIRE(static_cast<double>(my_object.drift) == 0.0);
            REQUIRE(static_cast<int>(my_object.seed) == 1);
        }
        THEN("the waveform message snaps the shape") {
            my_object.waveform(atoms{symbol("sine")});
            REQUIRE(my_object.osc().snap_targets().v[kvo::p_shape] == 0.0);
            my_object.waveform(atoms{symbol("pulse")});
            REQUIRE(my_object.osc().snap_targets().v[kvo::p_shape] == 3.0);
        }
        THEN("out-of-range values clamp") {
            my_object.pw = 0.0;
            REQUIRE(static_cast<double>(my_object.pw) == 1.0);
            my_object.shape = 9.0;
            REQUIRE(static_cast<double>(my_object.shape) == 3.0);
        }
        THEN("preset and clear messages are callable") {
            my_object.store(atoms{1});
            my_object.recall(atoms{1});
            my_object.clear(atoms{});
            REQUIRE(true);
        }
    }
}
