/// @file
/// @brief      Unit tests for tap.svf~ and its svf.h kernel.
/// @details    Kernel scenarios exercise taptools::svf::svf_filter directly; one scenario goes
///             through the Min mock. The harness pins this translation unit to C++17.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <cmath>
#include <cstddef>
#include <vector>

#include "c74_min_unittest.h"
#include "tap.svf_tilde.cpp"

namespace ksv = taptools::svf;

namespace {

    constexpr double k_sr = 48000.0;

    ksv::svf_filter make_filter(int channels = 1) {
        ksv::svf_filter f;
        f.prepare(k_sr, channels);
        f.set_smooth_ms(0.0);
        return f;
    }

    std::vector<double> tone(double freq, double seconds, double amp) {
        std::vector<double> x(static_cast<size_t>(seconds * k_sr));
        for (size_t i = 0; i < x.size(); ++i) {
            x[i] = amp * std::sin(2.0 * ksv::k_pi * freq * i / k_sr);
        }
        return x;
    }

    double goertzel(const std::vector<double>& x, size_t lo, size_t hi, double freq_hz) {
        const double w    = 2.0 * ksv::k_pi * freq_hz / k_sr;
        const double coef = 2.0 * std::cos(w);
        double       s1 = 0.0, s2 = 0.0;
        for (size_t i = lo; i < hi && i < x.size(); ++i) {
            const double s = x[i] + coef * s1 - s2;
            s2             = s1;
            s1             = s;
        }
        return s1 * s1 + s2 * s2 - coef * s1 * s2;
    }

    double rms(const std::vector<double>& x, size_t lo, size_t hi) {
        double acc = 0.0;
        size_t n   = 0;
        for (size_t i = lo; i < hi && i < x.size(); ++i, ++n) {
            acc += x[i] * x[i];
        }
        return n ? std::sqrt(acc / n) : 0.0;
    }

    size_t at_s(double seconds) {
        return static_cast<size_t>(seconds * k_sr);
    }

    // Steady-state gain (dB) of a fresh filter configured by `setup`, probed with a tone at `freq`.
    template <typename Setup>
    double gain_db(Setup setup, double freq, double amp = 0.1) {
        auto f = make_filter();
        setup(f);
        const auto          in = tone(freq, 0.5, amp);
        std::vector<double> out(in.size());
        for (size_t i = 0; i < in.size(); ++i) {
            out[i] = f.process(in[i]);
        }
        const size_t lo = at_s(0.25), hi = at_s(0.5);
        return 10.0 * std::log10(goertzel(out, lo, hi, freq) / goertzel(in, lo, hi, freq));
    }

    // Run white-ish deterministic noise through a configured filter, return the output.
    template <typename Setup>
    std::vector<double> run_noise(Setup setup, size_t n) {
        auto f = make_filter();
        setup(f);
        std::vector<double> out(n);
        unsigned            s = 22222u;
        for (size_t i = 0; i < n; ++i) {
            s              = s * 1664525u + 1013904223u; // LCG: deterministic, no <random> needed
            const double x = (static_cast<double>(s) / 2147483648.0 - 1.0) * 0.25;
            out[i]         = f.process(x);
        }
        return out;
    }

} // namespace

SCENARIO("the cascade is maximally flat at resonance 0 — Butterworth at every order") {
    GIVEN("a 1 kHz lowpass with resonance 0") {
        for (int order : {2, 4, 8}) {
            auto setup = [order](ksv::svf_filter& f) {
                f.set_mode(ksv::mode_lowpass);
                f.set_order(order);
                f.set_frequency(1000.0);
                f.set_resonance(0.0);
            };
            THEN("the passband is flat and the cutoff sits at -3 dB (order " + std::to_string(order) + ")") {
                REQUIRE(std::abs(gain_db(setup, 100.0)) < 0.2);
                REQUIRE(std::abs(gain_db(setup, 1000.0) - (-3.01)) < 0.3);
            }
        }
    }
}

SCENARIO("stopband slope scales with order: 12/24/48 dB per octave") {
    for (int order : {2, 4, 8}) {
        auto setup = [order](ksv::svf_filter& f) {
            f.set_mode(ksv::mode_lowpass);
            f.set_order(order);
            f.set_frequency(500.0);
            f.set_resonance(0.0);
        };
        THEN("one octave deep in the stopband drops by ~6*order dB (order " + std::to_string(order) + ")") {
            const double slope = gain_db(setup, 2000.0) - gain_db(setup, 4000.0);
            REQUIRE(std::abs(slope - 6.0 * order) < 1.5);
        }
    }
}

SCENARIO("highpass mirrors lowpass") {
    auto setup = [](ksv::svf_filter& f) {
        f.set_mode(ksv::mode_highpass);
        f.set_order(4);
        f.set_frequency(1000.0);
        f.set_resonance(0.0);
    };
    THEN("the high passband is flat, the cutoff is -3 dB, and lows are strongly rejected") {
        REQUIRE(std::abs(gain_db(setup, 8000.0)) < 0.2);
        REQUIRE(std::abs(gain_db(setup, 1000.0) - (-3.01)) < 0.3);
        REQUIRE(gain_db(setup, 125.0) < -60.0);
    }
}

SCENARIO("resonance sharpens a single peak with a predictable Q") {
    GIVEN("a 2nd-order lowpass with resonance 0.9") {
        auto setup = [](ksv::svf_filter& f) {
            f.set_mode(ksv::mode_lowpass);
            f.set_order(2);
            f.set_frequency(1000.0);
            f.set_resonance(0.9);
        };
        THEN("the peak at the cutoff is ~20*log10(Q) with Q = 0.7071/(1-0.9)") {
            const double expected = 20.0 * std::log10(ksv::q_from_resonance(0.9, 2));
            REQUIRE(std::abs(gain_db(setup, 1000.0) - expected) < 1.0);
        }
        THEN("the passband well below the peak stays near unity") {
            REQUIRE(std::abs(gain_db(setup, 100.0)) < 0.5);
        }
    }
}

SCENARIO("Q conversion helpers") {
    THEN("resonance 0 is the Butterworth base Q of the order") {
        REQUIRE(ksv::q_from_resonance(0.0, 2) == Approx(0.70710678).epsilon(1e-6));
        REQUIRE(ksv::q_from_resonance(0.0, 4) == Approx(1.30656296).epsilon(1e-6));
        REQUIRE(ksv::q_from_resonance(0.0, 8) == Approx(2.56291545).epsilon(1e-6));
    }
    THEN("q_from_resonance and resonance_from_q round-trip") {
        for (int order : {2, 4, 8}) {
            for (double r : {0.0, 0.3, 0.7, 0.95}) {
                REQUIRE(ksv::resonance_from_q(ksv::q_from_resonance(r, order), order) == Approx(r).margin(1e-9));
            }
        }
    }
    THEN("a Q at or below the base maps to resonance 0") {
        REQUIRE(ksv::resonance_from_q(0.5, 2) == 0.0);
    }
}

SCENARIO("morph corners are bit-identical to the discrete modes") {
    struct corner {
        double morph;
        int    mode;
    };
    for (corner c : {corner{0.0, ksv::mode_lowpass}, corner{0.25, ksv::mode_bandpass}, corner{0.5, ksv::mode_highpass},
                     corner{0.75, ksv::mode_notch}, corner{1.0, ksv::mode_lowpass}}) {
        auto discrete = run_noise(
            [&](ksv::svf_filter& f) {
                f.set_mode(c.mode);
                f.set_order(4);
                f.set_frequency(800.0);
                f.set_resonance(0.5);
            },
            4096);
        auto morphed = run_noise(
            [&](ksv::svf_filter& f) {
                f.set_mode(ksv::mode_morph);
                f.set_morph(c.morph);
                f.set_order(4);
                f.set_frequency(800.0);
                f.set_resonance(0.5);
            },
            4096);
        THEN("morph " + std::to_string(c.morph) + " matches mode " + std::to_string(c.mode)) {
            double maxdiff = 0.0;
            for (size_t i = 0; i < discrete.size(); ++i) {
                maxdiff = std::max(maxdiff, std::abs(discrete[i] - morphed[i]));
            }
            REQUIRE(maxdiff < 1e-12);
        }
    }
    THEN("mid-morph positions stay bounded") {
        auto mid = run_noise(
            [](ksv::svf_filter& f) {
                f.set_mode(ksv::mode_morph);
                f.set_morph(0.375);
                f.set_order(8);
                f.set_frequency(2000.0);
                f.set_resonance(0.8);
            },
            24000);
        REQUIRE(rms(mid, 0, mid.size()) > 0.0);
        for (double v : mid) {
            REQUIRE(std::abs(v) < 100.0);
        }
    }
}

SCENARIO("the notch removes the center frequency") {
    auto setup = [](ksv::svf_filter& f) {
        f.set_mode(ksv::mode_notch);
        f.set_order(2);
        f.set_frequency(1000.0);
        f.set_resonance(0.5);
    };
    THEN("the center is at least 50 dB down while the far passband is untouched") {
        REQUIRE(gain_db(setup, 1000.0) < -50.0);
        REQUIRE(std::abs(gain_db(setup, 60.0)) < 0.3);
    }
}

SCENARIO("the allpass passes every frequency at unity") {
    auto setup = [](ksv::svf_filter& f) {
        f.set_mode(ksv::mode_allpass);
        f.set_order(2);
        f.set_frequency(1000.0);
        f.set_resonance(0.3);
    };
    for (double freq : {100.0, 1000.0, 4000.0, 12000.0}) {
        THEN("unity at " + std::to_string(freq) + " Hz") {
            REQUIRE(std::abs(gain_db(setup, freq)) < 0.2);
        }
    }
}

SCENARIO("parametric-EQ modes hit their gain targets — Simper's bell and shelf tables") {
    GIVEN("a +12 dB bell at 1 kHz") {
        auto setup = [](ksv::svf_filter& f) {
            f.set_mode(ksv::mode_bell);
            f.set_frequency(1000.0);
            f.set_resonance(0.5);
            f.set_gain(12.0);
        };
        THEN("the center is boosted by 12 dB and the far skirts are flat") {
            REQUIRE(std::abs(gain_db(setup, 1000.0) - 12.0) < 0.3);
            REQUIRE(std::abs(gain_db(setup, 40.0)) < 0.4);
            REQUIRE(std::abs(gain_db(setup, 16000.0)) < 0.4);
        }
    }
    GIVEN("a -12 dB bell at 1 kHz") {
        auto setup = [](ksv::svf_filter& f) {
            f.set_mode(ksv::mode_bell);
            f.set_frequency(1000.0);
            f.set_resonance(0.5);
            f.set_gain(-12.0);
        };
        THEN("the cut is symmetric to the boost") {
            REQUIRE(std::abs(gain_db(setup, 1000.0) + 12.0) < 0.3);
        }
    }
    GIVEN("a +9 dB low shelf at 500 Hz") {
        auto setup = [](ksv::svf_filter& f) {
            f.set_mode(ksv::mode_lowshelf);
            f.set_frequency(500.0);
            f.set_resonance(0.0);
            f.set_gain(9.0);
        };
        THEN("lows are lifted by 9 dB, highs are untouched") {
            REQUIRE(std::abs(gain_db(setup, 30.0) - 9.0) < 0.4);
            REQUIRE(std::abs(gain_db(setup, 12000.0)) < 0.4);
        }
    }
    GIVEN("a -9 dB high shelf at 2 kHz") {
        auto setup = [](ksv::svf_filter& f) {
            f.set_mode(ksv::mode_highshelf);
            f.set_frequency(2000.0);
            f.set_resonance(0.0);
            f.set_gain(-9.0);
        };
        THEN("highs drop by 9 dB, lows are untouched") {
            REQUIRE(std::abs(gain_db(setup, 16000.0) + 9.0) < 0.5);
            REQUIRE(std::abs(gain_db(setup, 60.0)) < 0.4);
        }
    }
}

SCENARIO("the driven circuit self-oscillates at the cutoff at resonance 1") {
    GIVEN("a pinged, fully resonant driven 2nd-order filter at 1 kHz") {
        auto f = make_filter();
        f.set_mode(ksv::mode_lowpass);
        f.set_order(2);
        f.set_circuit(ksv::circuit_driven);
        f.set_frequency(1000.0);
        f.set_resonance(1.0);
        f.process(0.5); // the ping — an all-zero state is a fixed point
        const size_t        n = at_s(1.0);
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            out[i] = f.process(0.0);
        }
        THEN("it keeps ringing, bounded by the saturator") {
            REQUIRE(rms(out, at_s(0.75), n) > 0.05);
            for (double v : out) {
                REQUIRE(std::abs(v) < 4.0);
            }
        }
        THEN("the oscillation sits within 3% of the tuned cutoff") {
            size_t zc = 0;
            for (size_t i = at_s(0.5) + 1; i < n; ++i) {
                if ((out[i] >= 0.0) != (out[i - 1] >= 0.0)) {
                    ++zc;
                }
            }
            const double hz = zc / (2.0 * 0.5);
            REQUIRE(hz > 970.0);
            REQUIRE(hz < 1030.0);
        }
    }
}

SCENARIO("stability under abuse") {
    GIVEN("a driven 8th-order filter, max drive and resonance, cutoff thrashed per sample") {
        auto f = make_filter();
        f.set_mode(ksv::mode_lowpass);
        f.set_order(8);
        f.set_circuit(ksv::circuit_driven);
        f.set_resonance(1.0);
        f.set_drive(24.0);
        unsigned s      = 777u;
        bool     finite = true;
        for (size_t i = 0; i < at_s(1.0); ++i) {
            s               = s * 1664525u + 1013904223u;
            const double cf = 100.0 + (static_cast<double>(s >> 8) / 16777216.0) * 15000.0;
            const double x  = std::sin(2.0 * ksv::k_pi * 220.0 * i / k_sr);
            const double y  = f.process(x, cf);
            if (!std::isfinite(y) || std::abs(y) > 100.0) {
                finite = false;
            }
        }
        THEN("the output never blows up") {
            REQUIRE(finite);
        }
    }
    GIVEN("a clean filter at resonance 1 (the damping floor)") {
        auto out = run_noise(
            [](ksv::svf_filter& f) {
                f.set_mode(ksv::mode_lowpass);
                f.set_order(2);
                f.set_frequency(3000.0);
                f.set_resonance(1.0);
            },
            48000);
        THEN("the output stays finite") {
            bool ok = true;
            for (double v : out) {
                if (!std::isfinite(v) || std::abs(v) > 1000.0) {
                    ok = false;
                }
            }
            REQUIRE(ok);
        }
    }
}

SCENARIO("the kernel is multichannel: shared coefficients, independent state") {
    GIVEN("a 2-channel kernel with an impulse into channel 0 only") {
        auto f = make_filter(2);
        f.set_mode(ksv::mode_lowpass);
        f.set_frequency(1000.0);
        f.set_resonance(0.7);
        double e0 = 0.0, e1 = 0.0;
        for (int i = 0; i < 4096; ++i) {
            f.tick();
            const double y0 = f.process(0, i == 0 ? 1.0 : 0.0);
            const double y1 = f.process(1, 0.0);
            e0 += y0 * y0;
            e1 += y1 * y1;
        }
        THEN("channel 0 rings and channel 1 stays perfectly silent") {
            REQUIRE(e0 > 0.01);
            REQUIRE(e1 == 0.0);
        }
    }
}

SCENARIO("clear() silences the filter") {
    auto f = make_filter();
    f.set_frequency(500.0);
    f.set_resonance(0.9);
    for (int i = 0; i < 1000; ++i) {
        f.process(1.0);
    }
    f.clear();
    double energy = 0.0;
    for (int i = 0; i < 1000; ++i) {
        const double y = f.process(0.0);
        energy += y * y;
    }
    THEN("no state survives") {
        REQUIRE(energy == 0.0);
    }
}

SCENARIO("the Min wrapper instantiates and takes its attributes") {
    ext_main(nullptr);
    GIVEN("an instance of tap.svf~") {
        test_wrapper<svf> an_instance;
        svf&              my_object = an_instance;
        THEN("the defaults are sane") {
            REQUIRE(my_object.type == symbol("lowpass"));
            REQUIRE(static_cast<double>(my_object.frequency) == 1000.0);
            REQUIRE(static_cast<int>(my_object.order) == 2);
            REQUIRE(my_object.circuit == symbol("clean"));
        }
        WHEN("attributes are set") {
            my_object.type      = "morph";
            my_object.morph     = 0.5;
            my_object.order     = 5; // snaps down to 4
            my_object.circuit   = "driven";
            my_object.resonance = 0.8;
            THEN("they are stored and quantized as documented") {
                REQUIRE(my_object.type == symbol("morph"));
                REQUIRE(static_cast<int>(my_object.order) == 4);
                REQUIRE(my_object.filter().mode() == ksv::mode_morph);
                REQUIRE(my_object.filter().order() == 4);
                REQUIRE(my_object.filter().circuit() == ksv::circuit_driven);
            }
        }
        WHEN("the q message sets the resonance from a Q value") {
            my_object.q(ksv::q_from_resonance(0.6, 2));
            THEN("the resonance attribute follows") {
                REQUIRE(static_cast<double>(my_object.resonance) == Approx(0.6).margin(1e-9));
            }
        }
    }
}
