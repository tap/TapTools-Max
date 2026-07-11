/// @file
/// @brief      Unit tests for tap.pitchaccum~ and its grm_pitchaccum.h kernel.
/// @details    The kernel scenarios exercise taptools::pitchaccum::accum_bank directly (no Max
///             dependency); the wrapper scenario goes through the Min mock kernel. The test
///             harness pins this translation unit to C++17.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min_unittest.h"
#include "tap.pitchaccum_tilde.cpp"

#include <cmath>
#include <cstddef>
#include <vector>

namespace kpa = taptools::pitchaccum;

namespace {

constexpr double c_sr = 48000.0;

// Fresh bank at 48 kHz, instant parameter application, wet-only through voice 1.
kpa::accum_bank make_solo_bank() {
    kpa::accum_bank bank;
    bank.prepare(c_sr);
    bank.set_smooth_ms(0.0);
    bank.set_mix(100.0);
    bank.set_voice_gain(0, 100.0);
    bank.set_voice_gain(1, 0.0);
    return bank;
}

std::vector<double> sine(double freq, double seconds, double amp = 0.5) {
    std::vector<double> s(static_cast<size_t>(seconds * c_sr));
    for (size_t i = 0; i < s.size(); ++i)
        s[i] = amp * std::sin(2.0 * kpa::k_pi * freq * i / c_sr);
    return s;
}

std::vector<double> run(kpa::accum_bank& bank, const std::vector<double>& in) {
    std::vector<double> out(in.size());
    bank.process(in.data(), out.data(), in.size());
    return out;
}

double energy(const std::vector<double>& x, size_t lo, size_t hi) {
    double acc = 0.0;
    for (size_t i = lo; i < hi && i < x.size(); ++i)
        acc += x[i] * x[i];
    return acc;
}

// Goertzel power of one bin over [lo, hi).
double goertzel(const std::vector<double>& x, size_t lo, size_t hi, double freq_hz) {
    const double w    = 2.0 * kpa::k_pi * freq_hz / c_sr;
    const double coef = 2.0 * std::cos(w);
    double s1 = 0.0, s2 = 0.0;
    for (size_t i = lo; i < hi && i < x.size(); ++i) {
        const double s = x[i] + coef * s1 - s2;
        s2 = s1;
        s1 = s;
    }
    return s1 * s1 + s2 * s2 - coef * s1 * s2;
}

size_t at_s(double seconds) { return static_cast<size_t>(seconds * c_sr); }

}  // namespace


SCENARIO("transposition lands on the expected pitch") {
    GIVEN("+12 semitones") {
        auto bank = make_solo_bank();
        bank.set_trans(0, 12.0);
        auto out = run(bank, sine(440.0, 1.0));
        THEN("the octave dominates the original") {
            const double e880 = goertzel(out, at_s(0.4), at_s(0.9), 880.0);
            const double e440 = goertzel(out, at_s(0.4), at_s(0.9), 440.0);
            REQUIRE(e880 > 5.0 * e440);
        }
    }
    GIVEN("-12 semitones") {
        auto bank = make_solo_bank();
        bank.set_trans(0, -12.0);
        auto out = run(bank, sine(440.0, 1.0));
        THEN("the lower octave dominates") {
            const double e220 = goertzel(out, at_s(0.4), at_s(0.9), 220.0);
            const double e440 = goertzel(out, at_s(0.4), at_s(0.9), 440.0);
            REQUIRE(e220 > 5.0 * e440);
        }
    }
    GIVEN("0 semitones") {
        auto bank = make_solo_bank();
        auto out = run(bank, sine(440.0, 1.0));
        THEN("the input frequency passes untransposed") {
            const double e440 = goertzel(out, at_s(0.4), at_s(0.9), 440.0);
            const double e660 = goertzel(out, at_s(0.4), at_s(0.9), 660.0);
            REQUIRE(e440 > 100.0 * e660);
        }
    }
}

SCENARIO("the voice delay positions the wet signal in time") {
    auto bank = make_solo_bank();
    bank.set_delay(0, 500.0);
    std::vector<double> in(at_s(1.0), 0.0);
    for (size_t i = 0; i < at_s(0.01); ++i)  // 10 ms noise burst at t=0
        in[i] = ((i * 2654435761u) % 2000) / 1000.0 - 1.0;
    auto out = run(bank, in);

    THEN("nothing arrives before the delay") {
        REQUIRE(energy(out, at_s(0.02), at_s(0.45)) < 1e-12);
    }
    THEN("the burst arrives after it") {
        REQUIRE(energy(out, at_s(0.5), at_s(0.8)) > 1e-4);
    }
}

SCENARIO("feedback accumulates the transposition pass after pass") {
    auto bank = make_solo_bank();
    bank.set_trans(0, 7.0);      // +7 st per pass
    bank.set_delay(0, 300.0);
    bank.set_fb(0, 70.0);
    auto in = sine(440.0, 0.2, 0.5);
    in.resize(at_s(1.2), 0.0);
    auto out = run(bank, in);

    const double f1 = 440.0 * std::exp2(7.0 / 12.0);    // 659.26 Hz after one pass
    const double f2 = 440.0 * std::exp2(14.0 / 12.0);   // 987.77 Hz after two passes

    THEN("the first pass rings at +7 semitones") {
        const double target = goertzel(out, at_s(0.32), at_s(0.55), f1);
        const double ref    = goertzel(out, at_s(0.32), at_s(0.55), 500.0);
        REQUIRE(target > 10.0 * ref);
    }
    THEN("the second pass rings at +14 semitones — the accumulation") {
        const double target = goertzel(out, at_s(0.65), at_s(1.0), f2);
        const double ref    = goertzel(out, at_s(0.65), at_s(1.0), 500.0);
        REQUIRE(target > 5.0 * ref);
    }
}

SCENARIO("periodic modulation spreads the spectrum") {
    auto steady = make_solo_bank();
    auto out_steady = run(steady, sine(440.0, 1.0));

    auto modded = make_solo_bank();
    modded.set_moddepth(1.0);
    modded.set_modfreq(5.0);
    auto out_mod = run(modded, sine(440.0, 1.0));

    THEN("energy leaves the carrier bin under modulation") {
        const double e_steady = goertzel(out_steady, at_s(0.4), at_s(0.9), 440.0);
        const double e_mod    = goertzel(out_mod, at_s(0.4), at_s(0.9), 440.0);
        REQUIRE(e_mod < 0.5 * e_steady);
    }
}

SCENARIO("random modulation is deterministic across instances") {
    auto make = [] {
        auto bank = make_solo_bank();
        bank.set_voice_gain(1, 80.0);
        bank.set_randdepth(3.0);
        bank.set_randrate(8.0);
        return bank;
    };
    auto a = make();
    auto b = make();
    const auto in = sine(330.0, 0.5);
    const auto oa = run(a, in);
    const auto ob = run(b, in);

    THEN("two identically configured banks render identical output") {
        double maxdiff = 0.0;
        for (size_t i = 0; i < oa.size(); ++i)
            maxdiff = std::max(maxdiff, std::abs(oa[i] - ob[i]));
        REQUIRE(maxdiff == 0.0);
    }
}

SCENARIO("preset recall morphs without discontinuities") {
    auto bank = make_solo_bank();
    bank.set_trans(0, 2.0);
    bank.set_delay(0, 80.0);
    bank.set_mix(60.0);
    bank.store_preset(0);

    bank.set_trans(0, -9.0);
    bank.set_delay(0, 900.0);
    bank.set_mix(30.0);

    auto sig = [](size_t i) { return 0.3 * std::sin(2.0 * kpa::k_pi * 220.0 * i / c_sr); };
    size_t i = 0;
    for (; i < 4000; ++i)
        bank.process(sig(i));

    REQUIRE(bank.recall_preset(0, 0.08));
    REQUIRE(bank.morphing());

    double prev    = bank.process(sig(i++));
    double maxjump = 0.0;
    for (; i < 4000 + at_s(0.3); ++i) {
        const double y = bank.process(sig(i));
        maxjump = std::max(maxjump, std::abs(y - prev));
        prev    = y;
    }
    THEN("no sample-to-sample click during the morph") {
        REQUIRE(maxjump < 0.3);
    }
    THEN("the parameters land exactly on the preset") {
        REQUIRE(!bank.morphing());
        const auto cur = bank.snap_current();
        const auto pre = bank.preset(0);
        for (int p = 0; p < kpa::k_num_params; ++p)
            REQUIRE(std::abs(cur.v[p] - pre.v[p]) < 1e-9);
    }
}

SCENARIO("mix 0 is an exact passthrough") {
    kpa::accum_bank bank;
    bank.prepare(c_sr);
    bank.set_smooth_ms(0.0);
    bank.set_mix(0.0);
    bank.set_fb(0, 90.0);  // feedback churning inside must not leak
    bank.set_trans(0, 12.0);

    double maxdiff = 0.0;
    for (int i = 0; i < 2048; ++i) {
        const double x = std::sin(0.11 * i) * 0.7;
        maxdiff = std::max(maxdiff, std::abs(bank.process(x) - x));
    }
    THEN("output equals input") {
        REQUIRE(maxdiff < 1e-9);
    }
}

SCENARIO("the pitch follower adapts the window to the input period") {
    GIVEN("a 220 Hz tone with follow enabled") {
        auto bank = make_solo_bank();
        bank.set_follow(true);
        run(bank, sine(220.0, 1.5));
        THEN("the effective window converges near two periods (~9.1 ms)") {
            const double w = bank.effective_window_ms();
            REQUIRE(w > 6.0);
            REQUIRE(w < 13.0);
        }
    }
    GIVEN("unpitched noise with follow enabled") {
        auto bank = make_solo_bank();
        bank.set_follow(true);
        std::vector<double> noise(at_s(1.0));
        uint32_t rng = 22222u;
        for (auto& s : noise) {
            rng = rng * 1664525u + 1013904223u;
            s = 0.4 * ((rng / 2147483648.0) - 1.0);
        }
        run(bank, noise);
        THEN("the window stays at the manual setting") {
            const double w = bank.effective_window_ms();
            REQUIRE(w > 70.0);
            REQUIRE(w < 100.0);
        }
    }
}

SCENARIO("maximum feedback stays bounded") {
    auto bank = make_solo_bank();
    bank.set_trans(0, 5.0);
    bank.set_voice_gain(1, 100.0);
    bank.set_trans(1, -4.0);
    bank.set_fb(0, 99.0);
    bank.set_fb(1, 99.0);
    bank.set_delay(0, 150.0);
    bank.set_delay(1, 220.0);

    auto in = sine(440.0, 0.2, 0.8);
    in.resize(at_s(5.0), 0.0);
    auto out = run(bank, in);

    THEN("five seconds of 0.99 feedback never blows up") {
        double peak = 0.0;
        bool   finite = true;
        for (double s : out) {
            if (!std::isfinite(s))
                finite = false;
            peak = std::max(peak, std::abs(s));
        }
        REQUIRE(finite);
        REQUIRE(peak < 50.0);
    }
}

SCENARIO("the Min wrapper instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("an instance of tap.pitchaccum~") {
        test_wrapper<pitchaccum> an_instance;
        pitchaccum&              my_object = an_instance;

        THEN("attribute defaults match the spec") {
            REQUIRE(static_cast<double>(my_object.trans1) == 0.0);
            REQUIRE(static_cast<double>(my_object.gain1) == 50.0);
            REQUIRE(static_cast<double>(my_object.window) == 87.0);
            REQUIRE(static_cast<double>(my_object.xfade) == 50.0);
            REQUIRE(static_cast<double>(my_object.modphase) == 90.0);
            REQUIRE(static_cast<double>(my_object.mix) == 50.0);
            REQUIRE(static_cast<bool>(my_object.follow) == false);
            REQUIRE(static_cast<double>(my_object.interp) == 500.0);
        }
        THEN("out-of-range attribute values are clamped") {
            my_object.trans1 = 60.0;
            REQUIRE(static_cast<double>(my_object.trans1) == 24.0);
            my_object.fb1 = 150.0;
            REQUIRE(static_cast<double>(my_object.fb1) == 99.0);
            my_object.window = 1.0;
            REQUIRE(static_cast<double>(my_object.window) == 5.0);
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
