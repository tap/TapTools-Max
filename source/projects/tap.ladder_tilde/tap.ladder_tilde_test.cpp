/// @file
/// @brief      Unit tests for tap.ladder~ and its ladder.h kernel.
/// @details    Kernel scenarios exercise taptools::ladder::ladder_filter directly; one scenario
///             goes through the Min mock. The harness pins this translation unit to C++17.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min_unittest.h"
#include "tap.ladder_tilde.cpp"

#include <cmath>
#include <cstddef>
#include <vector>

namespace klf = taptools::ladder;

namespace {

constexpr double c_sr = 48000.0;

klf::ladder_filter make_filter() {
    klf::ladder_filter f;
    f.prepare(c_sr);
    f.set_smooth_ms(0.0);
    return f;
}

std::vector<double> run_tone(klf::ladder_filter& f, double freq, double seconds, double amp) {
    std::vector<double> out(static_cast<size_t>(seconds * c_sr));
    for (size_t i = 0; i < out.size(); ++i)
        out[i] = f.process(amp * std::sin(2.0 * klf::k_pi * freq * i / c_sr));
    return out;
}

double goertzel(const std::vector<double>& x, size_t lo, size_t hi, double freq_hz) {
    const double w    = 2.0 * klf::k_pi * freq_hz / c_sr;
    const double coef = 2.0 * std::cos(w);
    double s1 = 0.0, s2 = 0.0;
    for (size_t i = lo; i < hi && i < x.size(); ++i) {
        const double s = x[i] + coef * s1 - s2;
        s2 = s1;
        s1 = s;
    }
    return s1 * s1 + s2 * s2 - coef * s1 * s2;
}

double rms(const std::vector<double>& x, size_t lo, size_t hi) {
    double acc = 0.0;
    size_t n   = 0;
    for (size_t i = lo; i < hi && i < x.size(); ++i, ++n)
        acc += x[i] * x[i];
    return n ? std::sqrt(acc / n) : 0.0;
}

size_t at_s(double seconds) { return static_cast<size_t>(seconds * c_sr); }

// Steady-state response (power) of a fresh filter configured by `setup` to a tone at `freq`.
template <typename Setup>
double tone_response(Setup setup, double freq, double amp = 0.05) {
    auto f = make_filter();
    setup(f);
    auto out = run_tone(f, freq, 0.5, amp);
    return goertzel(out, at_s(0.25), at_s(0.5), freq);
}

// Self-oscillation frequency: ping, let it settle, count zero crossings.
double self_osc_freq(double cutoff_hz) {
    auto f = make_filter();
    f.set_frequency(cutoff_hz);
    f.set_resonance(1.05);
    f.process(0.5);   // the ping — an all-zero state is a fixed point
    const size_t n = at_s(1.0);
    std::vector<double> out(n);
    for (size_t i = 0; i < n; ++i)
        out[i] = f.process(0.0);
    size_t zc = 0;
    for (size_t i = at_s(0.5) + 1; i < n; ++i)
        if ((out[i] >= 0.0) != (out[i - 1] >= 0.0))
            ++zc;
    return zc / (2.0 * 0.5);   // crossings per second / 2
}

}  // namespace


SCENARIO("self-oscillation tracks the tuned cutoff — the virtual-analog proof") {
    GIVEN("resonance past the oscillation threshold") {
        THEN("it oscillates within 3% of 1 kHz") {
            const double f = self_osc_freq(1000.0);
            REQUIRE(f > 970.0);
            REQUIRE(f < 1030.0);
        }
        THEN("it oscillates within 5% of 8 kHz — where the Stilson/Smith model detunes badly") {
            const double f = self_osc_freq(8000.0);
            REQUIRE(f > 7600.0);
            REQUIRE(f < 8400.0);
        }
    }
}

SCENARIO("self-oscillation is bounded and steady") {
    auto f = make_filter();
    f.set_frequency(700.0);
    f.set_resonance(klf::k_res_max);
    f.process(0.5);
    const size_t n = at_s(5.0);
    std::vector<double> out(n);
    for (size_t i = 0; i < n; ++i)
        out[i] = f.process(0.0);

    THEN("the tanh stages limit the amplitude — no blow-up, no decay") {
        double peak = 0.0;
        bool   finite = true;
        for (double s : out) {
            if (!std::isfinite(s))
                finite = false;
            peak = std::max(peak, std::abs(s));
        }
        REQUIRE(finite);
        REQUIRE(peak < 2.0);
        const double a = rms(out, at_s(4.0), at_s(4.5));
        const double b = rms(out, at_s(4.5), at_s(5.0));
        REQUIRE(a > 0.01);
        REQUIRE(b > a * 0.7);
        REQUIRE(b < a * 1.4);
    }
    THEN("clear() stops it") {
        f.clear();
        double acc = 0.0;
        for (int i = 0; i < 4800; ++i)
            acc += std::abs(f.process(0.0));
        REQUIRE(acc == 0.0);
    }
}

SCENARIO("the lowpass rolls off at 24 dB per octave") {
    auto lp = [](klf::ladder_filter& f) { f.set_frequency(500.0); };
    const double below = tone_response(lp, 125.0);    // fc/4
    const double above = tone_response(lp, 2000.0);   // 4*fc

    THEN("two octaves above cutoff sits far below the passband") {
        REQUIRE(above < 0.001 * below);   // > 30 dB
    }
}

SCENARIO("resonance peaks the response at the cutoff") {
    auto at_res = [](double r) {
        return tone_response([r](klf::ladder_filter& f) {
            f.set_frequency(1000.0);
            f.set_resonance(r);
        }, 1000.0, 0.02);
    };
    const double r0 = at_res(0.0);
    const double r5 = at_res(0.5);
    const double r9 = at_res(0.9);

    THEN("the peak grows monotonically with resonance") {
        REQUIRE(r5 > r0 * 1.5);
        REQUIRE(r9 > r5 * 1.5);
    }
}

SCENARIO("drive adds harmonic saturation") {
    auto driven = [](double drive_db) {
        auto f = make_filter();
        f.set_frequency(5000.0);   // fundamental and 3rd harmonic both in the passband
        f.set_drive(drive_db);
        auto out = run_tone(f, 200.0, 0.5, 0.3);
        const double fund = goertzel(out, at_s(0.25), at_s(0.5), 200.0);
        const double h3   = goertzel(out, at_s(0.25), at_s(0.5), 600.0);
        return h3 / (fund + 1e-30);
    };
    THEN("the relative 3rd harmonic rises by more than 10 dB at +18 dB drive") {
        REQUIRE(driven(18.0) > 10.0 * driven(0.0));
    }
}

SCENARIO("pole-mixed modes have the right shapes") {
    GIVEN("hp24 at 1 kHz") {
        auto hp = [](klf::ladder_filter& f) {
            f.set_frequency(1000.0);
            f.set_mode(klf::mode_hp24);
        };
        THEN("highs pass, lows die") {
            const double hi = tone_response(hp, 8000.0);
            const double lo = tone_response(hp, 125.0);
            REQUIRE(lo < 0.001 * hi);
        }
    }
    GIVEN("bp12 at 1 kHz") {
        auto bp = [](klf::ladder_filter& f) {
            f.set_frequency(1000.0);
            f.set_mode(klf::mode_bp12);
        };
        THEN("the center passes and both extremes are attenuated") {
            const double mid = tone_response(bp, 1000.0);
            const double lo  = tone_response(bp, 125.0);
            const double hi  = tone_response(bp, 8000.0);
            REQUIRE(mid > 15.0 * lo);
            REQUIRE(mid > 15.0 * hi);
        }
    }
    GIVEN("lp12 vs lp24 at 4x cutoff") {
        auto resp = [](int mode) {
            return tone_response([mode](klf::ladder_filter& f) {
                f.set_frequency(500.0);
                f.set_mode(mode);
            }, 2000.0);
        };
        THEN("the 12 dB slope attenuates less than the 24 dB slope") {
            REQUIRE(resp(klf::mode_lp12) > 4.0 * resp(klf::mode_lp24));
        }
    }
}

SCENARIO("comp restores the passband at high resonance") {
    auto passband = [](double comp) {
        return tone_response([comp](klf::ladder_filter& f) {
            f.set_frequency(1000.0);
            f.set_resonance(0.9);
            f.set_comp(comp);
        }, 100.0, 0.02);
    };
    THEN("comp 1 is at least 3 dB louder than comp 0") {
        REQUIRE(passband(1.0) > 2.0 * passband(0.0));
    }
}

SCENARIO("oversampling: consistent passband, less aliasing") {
    GIVEN("a low tone in the passband") {
        auto resp_at_os = [](int os) {
            return tone_response([os](klf::ladder_filter& f) {
                f.set_oversample(os);
                f.set_frequency(2000.0);
                f.set_resonance(0.3);
            }, 200.0);
        };
        THEN("1x and 2x match within ~1.5 dB") {
            const double r1 = resp_at_os(1);
            const double r2 = resp_at_os(2);
            REQUIRE(r2 > r1 * 0.7);
            REQUIRE(r2 < r1 * 1.4);
        }
    }
    GIVEN("a hard-driven high tone (5 kHz, harmonics past Nyquist)") {
        auto alias_energy = [](int os) {
            auto f = make_filter();
            f.set_oversample(os);
            f.set_frequency(8000.0);
            f.set_drive(18.0);
            auto out = run_tone(f, 5000.0, 0.5, 0.5);
            // the 7th harmonic (35 kHz) aliases to 13 kHz at 48k
            return goertzel(out, at_s(0.25), at_s(0.5), 13000.0);
        };
        THEN("4x has less energy at the alias frequency than 1x") {
            REQUIRE(alias_energy(4) < 0.5 * alias_energy(1));
        }
    }
}

SCENARIO("parameter morphs and signal-rate cutoff are click-free") {
    auto f = make_filter();
    f.set_frequency(300.0);
    f.set_resonance(0.8);
    f.store_preset(0);
    f.set_frequency(6000.0);
    f.set_resonance(0.2);
    f.set_drive(12.0);

    auto sig = [](size_t i) { return 0.3 * std::sin(2.0 * klf::k_pi * 220.0 * i / c_sr); };
    size_t i = 0;
    for (; i < at_s(0.2); ++i)
        f.process(sig(i));

    GIVEN("a 100 ms preset recall under a running tone") {
        REQUIRE(f.recall_preset(0, 0.1));
        double prev = f.process(sig(i++));
        double maxjump = 0.0;
        for (; i < at_s(0.6); ++i) {
            const double y = f.process(sig(i));
            maxjump = std::max(maxjump, std::abs(y - prev));
            prev = y;
        }
        THEN("no sample-to-sample click; parameters land") {
            REQUIRE(maxjump < 0.25);
            REQUIRE(!f.morphing());
            REQUIRE(std::abs(f.snap_current().v[klf::p_frequency] - 300.0) < 1e-9);
        }
    }
    GIVEN("a per-sample signal-rate cutoff sweep") {
        auto f2 = make_filter();
        f2.set_resonance(0.9);
        double prev = 0.0, maxjump = 0.0;
        const size_t n = at_s(0.3);
        for (size_t j = 0; j < n; ++j) {
            const double hz = 500.0 + 5500.0 * j / n;
            const double y  = f2.process(sig(j), hz);
            maxjump = std::max(maxjump, std::abs(y - prev));
            prev = y;
        }
        THEN("the sweep stays continuous") {
            REQUIRE(maxjump < 0.3);
        }
    }
}

SCENARIO("the Min wrapper instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("an instance of tap.ladder~") {
        test_wrapper<ladder> an_instance;
        ladder&              my_object = an_instance;

        THEN("attribute defaults match the spec") {
            REQUIRE(static_cast<double>(my_object.frequency) == 1000.0);
            REQUIRE(static_cast<double>(my_object.resonance) == 0.0);
            REQUIRE(static_cast<double>(my_object.drive) == 0.0);
            REQUIRE(static_cast<double>(my_object.comp) == 0.0);
            REQUIRE(static_cast<int>(my_object.mode) == klf::mode_lp24);
            REQUIRE(static_cast<int>(my_object.oversample) == 2);
        }
        THEN("out-of-range values are clamped") {
            my_object.resonance = 2.0;
            REQUIRE(static_cast<double>(my_object.resonance) == klf::k_res_max);
            my_object.mode = 99;
            REQUIRE(static_cast<int>(my_object.mode) == klf::k_num_modes - 1);
            my_object.oversample = 3;
            REQUIRE(static_cast<int>(my_object.oversample) == 2);
        }
        THEN("preset and clear messages are callable") {
            my_object.store(atoms { 1 });
            my_object.recall(atoms { 1 });
            my_object.recall(atoms { 1, 250.0 });
            my_object.clear(atoms {});
            REQUIRE(true);
        }
    }
}
