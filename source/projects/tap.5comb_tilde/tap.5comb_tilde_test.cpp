/// @file
/// @brief      Unit tests for tap.5comb~ and its grm_comb.h kernel.
/// @details    The kernel scenarios exercise taptools::fivecomb::comb_bank directly (it has no Max
///             dependency); the wrapper scenario goes through the Min mock kernel. Note the test
///             harness pins this translation unit to C++17.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min_unittest.h"
#include "tap.5comb_tilde.cpp"

#include <cmath>
#include <cstddef>
#include <vector>

namespace kfc = taptools::fivecomb;

namespace {

constexpr double c_sr = 48000.0;

// Fresh bank at 48 kHz with instant parameter application and a single active voice by default.
kfc::comb_bank make_bank() {
    kfc::comb_bank bank;
    bank.prepare(c_sr);
    bank.set_smooth_ms(0.0);
    return bank;
}

void solo_voice(kfc::comb_bank& bank, int voice, double freq_hz, double res_pct, double lp_hz = 20000.0) {
    for (int v = 0; v < kfc::k_voices; ++v)
        bank.set_res(v, v == voice ? res_pct : 0.0);
    bank.set_freq(voice, freq_hz);
    bank.set_lp(voice, lp_hz);
}

std::vector<double> impulse_response(kfc::comb_bank& bank, size_t n) {
    std::vector<double> out(n, 0.0);
    out[0] = bank.process(1.0);
    for (size_t i = 1; i < n; ++i)
        out[i] = bank.process(0.0);
    return out;
}

size_t argmax_abs(const std::vector<double>& x, size_t lo, size_t hi) {
    size_t best  = lo;
    double vbest = 0.0;
    for (size_t i = lo; i < hi && i < x.size(); ++i) {
        if (std::abs(x[i]) > vbest) {
            vbest = std::abs(x[i]);
            best  = i;
        }
    }
    return best;
}

double rms(const std::vector<double>& x, size_t lo, size_t hi) {
    double acc = 0.0;
    size_t n   = 0;
    for (size_t i = lo; i < hi && i < x.size(); ++i, ++n)
        acc += x[i] * x[i];
    return n ? std::sqrt(acc / n) : 0.0;
}

double energy(const std::vector<double>& x, size_t lo, size_t hi) {
    double acc = 0.0;
    for (size_t i = lo; i < hi && i < x.size(); ++i)
        acc += x[i] * x[i];
    return acc;
}

// Goertzel power of one bin over [lo, hi).
double goertzel(const std::vector<double>& x, size_t lo, size_t hi, double freq_hz, double sr) {
    const double w    = 2.0 * kfc::k_pi * freq_hz / sr;
    const double coef = 2.0 * std::cos(w);
    double s1 = 0.0, s2 = 0.0;
    for (size_t i = lo; i < hi && i < x.size(); ++i) {
        const double s = x[i] + coef * s1 - s2;
        s2 = s1;
        s1 = s;
    }
    return s1 * s1 + s2 * s2 - coef * s1 * s2;
}

}  // namespace


SCENARIO("impulse response rings at the tuned frequency") {
    auto bank = make_bank();
    solo_voice(bank, 0, 500.0, 80.0);  // period = 96 samples @ 48k
    bank.set_mix(100.0);

    auto ir = impulse_response(bank, 4 * 96 + 20);

    THEN("the direct impulse passes at unity (wet-sum normalization)") {
        REQUIRE(std::abs(ir[0] - 1.0) < 1e-9);  // 5 voices * 0.2, res contributes nothing at n=0
    }
    THEN("the first three echoes land one period apart") {
        REQUIRE(argmax_abs(ir, 50, 150) >= 94);
        REQUIRE(argmax_abs(ir, 50, 150) <= 98);
        const size_t e2 = argmax_abs(ir, 150, 250);
        REQUIRE(e2 >= 190);
        REQUIRE(e2 <= 195);
        const size_t e3 = argmax_abs(ir, 250, 340);
        REQUIRE(e3 >= 286);
        REQUIRE(e3 <= 292);
    }
}

SCENARIO("resonance maps to ring time") {
    // res chosen so RT60 = 1 s: res = 100 * ln(1/0.02) / ln(100/0.02) with the kernel's log curve.
    const double res_for_1s = 100.0 * std::log(1.0 / kfc::k_rt60_min) / std::log(kfc::k_rt60_max / kfc::k_rt60_min);

    auto bank = make_bank();
    solo_voice(bank, 0, 200.0, res_for_1s);
    bank.set_mix(100.0);
    auto ir = impulse_response(bank, static_cast<size_t>(1.2 * c_sr));

    THEN("the tail decays at roughly -60 dB per second") {
        const double early = rms(ir, static_cast<size_t>(0.20 * c_sr), static_cast<size_t>(0.30 * c_sr));
        const double late  = rms(ir, static_cast<size_t>(0.70 * c_sr), static_cast<size_t>(0.80 * c_sr));
        REQUIRE(early > 0.0);
        REQUIRE(late > 0.0);
        const double drop_db = 20.0 * std::log10(early / late);  // expected ~30 dB over 0.5 s
        REQUIRE(drop_db > 22.0);
        REQUIRE(drop_db < 40.0);
    }

    THEN("higher resonance rings longer") {
        auto bank2 = make_bank();
        solo_voice(bank2, 0, 200.0, 90.0);
        bank2.set_mix(100.0);
        auto ir2 = impulse_response(bank2, static_cast<size_t>(1.2 * c_sr));
        const double late_lo = rms(ir,  static_cast<size_t>(1.0 * c_sr), static_cast<size_t>(1.1 * c_sr));
        const double late_hi = rms(ir2, static_cast<size_t>(1.0 * c_sr), static_cast<size_t>(1.1 * c_sr));
        REQUIRE(late_hi > late_lo * 2.0);
    }

    THEN("res 100 with res master 2 stays bounded and nonzero (no clipper, feedback capped)") {
        auto bank3 = make_bank();
        solo_voice(bank3, 0, 200.0, 100.0);
        bank3.set_res_master(2.0);
        bank3.set_mix(100.0);
        auto ir3 = impulse_response(bank3, static_cast<size_t>(2.0 * c_sr));
        double peak = 0.0;
        for (double s : ir3)
            peak = std::max(peak, std::abs(s));
        REQUIRE(peak < 10.0);
        REQUIRE(rms(ir3, static_cast<size_t>(1.8 * c_sr), static_cast<size_t>(2.0 * c_sr)) > 1e-6);
    }
}

SCENARIO("the feedback lowpass darkens the ring") {
    auto bright = make_bank();
    solo_voice(bright, 0, 250.0, 85.0, 20000.0);
    bright.set_mix(100.0);
    auto ir_bright = impulse_response(bright, static_cast<size_t>(0.5 * c_sr));

    auto dark = make_bank();
    solo_voice(dark, 0, 250.0, 85.0, 1000.0);
    dark.set_mix(100.0);
    auto ir_dark = impulse_response(dark, static_cast<size_t>(0.5 * c_sr));

    THEN("normalized high-frequency content drops with a low cutoff") {
        auto hf_ratio = [](const std::vector<double>& x) {
            double diff = 0.0, tot = 0.0;
            for (size_t i = static_cast<size_t>(0.1 * c_sr) + 1; i < x.size(); ++i) {
                const double d = x[i] - x[i - 1];
                diff += d * d;
                tot += x[i] * x[i];
            }
            return diff / (tot + 1e-30);
        };
        REQUIRE(hf_ratio(ir_dark) < hf_ratio(ir_bright) * 0.5);
    }
}

SCENARIO("master multipliers scale all voices") {
    GIVEN("a frequency master of 0.5") {
        auto bank = make_bank();
        solo_voice(bank, 0, 500.0, 80.0);
        bank.set_freq_master(0.5);  // effective 250 Hz -> period 192
        bank.set_mix(100.0);
        auto ir = impulse_response(bank, 400);
        THEN("the echo period doubles") {
            const size_t e1 = argmax_abs(ir, 100, 290);
            REQUIRE(e1 >= 190);
            REQUIRE(e1 <= 194);
        }
    }
    GIVEN("a resonance master of 0") {
        auto bank = make_bank();
        solo_voice(bank, 0, 500.0, 100.0);
        bank.set_res_master(0.0);
        bank.set_mix(100.0);
        auto ir = impulse_response(bank, 2000);
        THEN("nothing rings after the direct impulse") {
            REQUIRE(energy(ir, 50, 2000) < 1e-20);
        }
    }
    GIVEN("a frequency below the floor") {
        auto bank = make_bank();
        solo_voice(bank, 0, 45.0, 80.0);
        bank.set_freq_master(0.01);  // 0.45 Hz requested -> clamped to the 5 Hz floor
        bank.set_mix(100.0);
        auto ir = impulse_response(bank, 12000);
        THEN("the echo lands at the 5 Hz floor period (9600 samples)") {
            const size_t e1 = argmax_abs(ir, 9000, 10200);
            REQUIRE(e1 >= 9596);
            REQUIRE(e1 <= 9604);
        }
    }
}

SCENARIO("mix crossfade is equal-power with unity endpoints") {
    GIVEN("mix 0") {
        auto bank = make_bank();
        bank.set_mix(0.0);
        THEN("the output is exactly the input") {
            double maxdiff = 0.0;
            for (int i = 0; i < 512; ++i) {
                const double x = std::sin(0.1 * i) * 0.7;
                maxdiff = std::max(maxdiff, std::abs(bank.process(x) - x));
            }
            REQUIRE(maxdiff < 1e-9);
        }
    }
    GIVEN("mix 100 with all resonance off") {
        auto bank = make_bank();
        for (int v = 0; v < kfc::k_voices; ++v)
            bank.set_res(v, 0.0);
        bank.set_mix(100.0);
        THEN("the wet-sum normalization gives unity throughput") {
            double maxdiff = 0.0;
            for (int i = 0; i < 512; ++i) {
                const double x = std::sin(0.13 * i) * 0.5;
                maxdiff = std::max(maxdiff, std::abs(bank.process(x) - x));
            }
            REQUIRE(maxdiff < 1e-9);
        }
    }
}

SCENARIO("preset recall morphs without discontinuities") {
    auto bank = make_bank();
    for (int v = 0; v < kfc::k_voices; ++v) {
        bank.set_freq(v, 100.0 + 50.0 * v);
        bank.set_res(v, 70.0);
        bank.set_lp(v, 20000.0);
    }
    bank.set_mix(80.0);
    bank.store_preset(0);

    for (int v = 0; v < kfc::k_voices; ++v) {
        bank.set_freq(v, 500.0 + 200.0 * v);
        bank.set_res(v, 40.0);
        bank.set_lp(v, 5000.0);
    }
    bank.set_mix(30.0);

    auto sine = [](size_t i) { return 0.3 * std::sin(2.0 * kfc::k_pi * 220.0 * i / c_sr); };

    size_t i = 0;
    for (; i < 2000; ++i)
        bank.process(sine(i));

    GIVEN("a 50 ms recall while audio runs") {
        REQUIRE(bank.recall_preset(0, 0.05));
        REQUIRE(bank.morphing());

        double prev    = bank.process(sine(i++));
        double maxjump = 0.0;
        for (; i < 2000 + 8000; ++i) {
            const double y = bank.process(sine(i));
            maxjump = std::max(maxjump, std::abs(y - prev));
            prev    = y;
        }
        THEN("no sample-to-sample jump exceeds a click threshold") {
            REQUIRE(maxjump < 0.25);
        }
        THEN("the parameters land exactly on the preset") {
            REQUIRE(!bank.morphing());
            const auto cur = bank.snap_current();
            const auto pre = bank.preset(0);
            for (int p = 0; p < kfc::k_num_params; ++p)
                REQUIRE(std::abs(cur.v[p] - pre.v[p]) < 1e-9);
        }
    }
}

SCENARIO("a direct setter overrides the morph for that parameter only") {
    auto bank = make_bank();
    bank.set_mix(100.0);
    bank.set_freq(0, 100.0);
    bank.store_preset(3);  // preset: mix 100, freq1 100

    bank.set_mix(0.0);
    bank.set_freq(0, 800.0);
    bank.recall_preset(3, 1.0);  // 1-second morph toward mix 100 / freq1 100

    for (int i = 0; i < 4800; ++i)  // 0.1 s in
        bank.process(0.0);

    bank.set_smooth_ms(10.0);
    bank.set_freq(0, 300.0);  // grab one slider mid-morph

    for (int i = 0; i < 9600; ++i)  // 0.2 s more
        bank.process(0.0);

    const auto cur = bank.snap_current();
    THEN("the grabbed parameter reached its new value") {
        REQUIRE(std::abs(cur.v[kfc::p_freq1] - 300.0) < 1e-9);
    }
    THEN("the others are still morphing toward the preset") {
        REQUIRE(bank.morphing());
        REQUIRE(cur.v[kfc::p_mix] > 5.0);
        REQUIRE(cur.v[kfc::p_mix] < 95.0);
    }
}

SCENARIO("warp stretches partials while staying stable and in tune at zero") {
    auto neutral = make_bank();
    solo_voice(neutral, 0, 500.0, 85.0);
    neutral.set_mix(100.0);
    auto ir_neutral = impulse_response(neutral, static_cast<size_t>(0.5 * c_sr));

    auto warped = make_bank();
    solo_voice(warped, 0, 500.0, 85.0);
    warped.set_warp(100.0);
    warped.set_mix(100.0);
    auto ir_warped = impulse_response(warped, static_cast<size_t>(0.5 * c_sr));

    THEN("warp 0 is the harmonic comb (echo at one period)") {
        const size_t e1 = argmax_abs(ir_neutral, 50, 150);
        REQUIRE(e1 >= 94);
        REQUIRE(e1 <= 98);
    }
    THEN("warp 100 stays bounded and still resonates") {
        double peak = 0.0;
        for (double s : ir_warped)
            peak = std::max(peak, std::abs(s));
        REQUIRE(peak < 10.0);
        REQUIRE(energy(ir_warped, 1000, ir_warped.size()) > 1e-8);
    }
    THEN("warp audibly changes the spectrum (low correlation with the neutral tail)") {
        const size_t lo = 1000, hi = ir_neutral.size();
        double dot = 0.0;
        for (size_t n = lo; n < hi; ++n)
            dot += ir_neutral[n] * ir_warped[n];
        const double corr = std::abs(dot) / (std::sqrt(energy(ir_neutral, lo, hi)) * std::sqrt(energy(ir_warped, lo, hi)) + 1e-30);
        REQUIRE(corr < 0.5);
    }
}

SCENARIO("phase 100 cancels the even harmonics") {
    auto reference = make_bank();
    solo_voice(reference, 0, 1000.0, 70.0);  // period 48 -> half tap 24, exact
    reference.set_mix(100.0);
    auto ir0 = impulse_response(reference, 24000);

    auto odd = make_bank();
    solo_voice(odd, 0, 1000.0, 70.0);
    odd.set_phase(100.0);
    odd.set_mix(100.0);
    auto ir1 = impulse_response(odd, 24000);

    const double f1_0 = goertzel(ir0, 100, 24000, 1000.0, c_sr);
    const double f2_0 = goertzel(ir0, 100, 24000, 2000.0, c_sr);
    const double f1_1 = goertzel(ir1, 100, 24000, 1000.0, c_sr);
    const double f2_1 = goertzel(ir1, 100, 24000, 2000.0, c_sr);

    THEN("both harmonics ring at phase 0") {
        REQUIRE(f1_0 > 0.0);
        REQUIRE(f2_0 > 0.0);
    }
    THEN("the second harmonic collapses relative to the fundamental at phase 100") {
        REQUIRE(f1_1 > 0.0);
        REQUIRE((f2_1 / f1_1) < 0.05 * (f2_0 / f1_0));
    }
}

SCENARIO("presets round-trip and clear() silences") {
    auto bank = make_bank();
    solo_voice(bank, 2, 432.0, 66.0, 12000.0);
    bank.set_warp(25.0);
    bank.store_preset(7);

    solo_voice(bank, 2, 1000.0, 10.0, 20000.0);
    bank.set_warp(0.0);
    bank.recall_preset(7, 0.0);

    THEN("targets match the stored snapshot") {
        const auto t = bank.snap_targets();
        REQUIRE(std::abs(t.v[kfc::p_freq1 + 2] - 432.0) < 1e-12);
        REQUIRE(std::abs(t.v[kfc::p_res1 + 2] - 66.0) < 1e-12);
        REQUIRE(std::abs(t.v[kfc::p_lp1 + 2] - 12000.0) < 1e-12);
        REQUIRE(std::abs(t.v[kfc::p_warp] - 25.0) < 1e-12);
    }
    THEN("invalid slots are rejected") {
        REQUIRE(!bank.store_preset(16));
        REQUIRE(!bank.recall_preset(-1, 0.1));
    }
    THEN("clear() gives exact zeros on silence") {
        for (int i = 0; i < 1000; ++i)
            bank.process(std::sin(0.2 * i));
        bank.clear();
        double acc = 0.0;
        for (int i = 0; i < 1000; ++i)
            acc += std::abs(bank.process(0.0));
        REQUIRE(acc == 0.0);
    }
}

SCENARIO("the Min wrapper instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("an instance of tap.5comb~") {
        test_wrapper<fivecomb> an_instance;
        fivecomb&              my_object = an_instance;

        THEN("attribute defaults match the spec") {
            REQUIRE(static_cast<double>(my_object.freq1) == 80.0);
            REQUIRE(static_cast<double>(my_object.freq5) == 102.0);
            REQUIRE(static_cast<double>(my_object.res3) == 50.0);
            REQUIRE(static_cast<double>(my_object.lp2) == 20000.0);
            REQUIRE(static_cast<double>(my_object.freq) == 1.0);
            REQUIRE(static_cast<double>(my_object.mix) == 100.0);
            REQUIRE(static_cast<double>(my_object.warp) == 0.0);
            REQUIRE(static_cast<double>(my_object.phase) == 0.0);
            REQUIRE(static_cast<double>(my_object.interp) == 500.0);
        }
        THEN("out-of-range attribute values are clamped") {
            my_object.freq1 = 10.0;
            REQUIRE(static_cast<double>(my_object.freq1) == 45.0);
            my_object.res1 = 150.0;
            REQUIRE(static_cast<double>(my_object.res1) == 100.0);
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
