/// @file
/// @brief      Unit tests for tap.spektral~ and its spektral.h kernel.
/// @details    The kernel scenarios exercise taptools::spektral::spektral_engine directly (no Max
///             dependency); one scenario goes through the Min mock kernel. The harness pins this
///             translation unit to C++17.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include "c74_min_unittest.h"
#include "tap.spektral_tilde.cpp"

#include <cmath>
#include <cstddef>
#include <vector>

namespace ksp = taptools::spektral;

namespace {

constexpr double c_sr = 48000.0;
constexpr int    c_N  = ksp::k_default_fftsize;   // 2048 -> hop 512, latency 2048

ksp::spektral_engine make_engine() {
    ksp::spektral_engine e;
    e.prepare(c_sr);
    e.set_smooth_ms(0.0);
    e.set_mix(100.0);   // wet only unless a test says otherwise
    return e;
}

std::vector<double> sine(double freq, double seconds, double amp = 0.5) {
    std::vector<double> s(static_cast<size_t>(seconds * c_sr));
    for (size_t i = 0; i < s.size(); ++i)
        s[i] = amp * std::sin(2.0 * ksp::k_pi * freq * i / c_sr);
    return s;
}

std::vector<double> run(ksp::spektral_engine& e, const std::vector<double>& in) {
    std::vector<double> out(in.size());
    e.process(in.data(), out.data(), in.size());
    return out;
}

double goertzel(const std::vector<double>& x, size_t lo, size_t hi, double freq_hz) {
    const double w    = 2.0 * ksp::k_pi * freq_hz / c_sr;
    const double coef = 2.0 * std::cos(w);
    double s1 = 0.0, s2 = 0.0;
    for (size_t i = lo; i < hi && i < x.size(); ++i) {
        const double s = x[i] + coef * s1 - s2;
        s2 = s1;
        s1 = s;
    }
    return s1 * s1 + s2 * s2 - coef * s1 * s2;
}

double energy(const std::vector<double>& x, size_t lo, size_t hi) {
    double acc = 0.0;
    for (size_t i = lo; i < hi && i < x.size(); ++i)
        acc += x[i] * x[i];
    return acc;
}

size_t zero_crossings(const std::vector<double>& x, size_t lo, size_t hi) {
    size_t n = 0;
    for (size_t i = std::max(lo, size_t(1)); i < hi && i < x.size(); ++i)
        if ((x[i] >= 0.0) != (x[i - 1] >= 0.0))
            ++n;
    return n;
}

size_t at_s(double seconds) { return static_cast<size_t>(seconds * c_sr); }

// Zero the curve around a band (the Hann mainlobe leaks into neighbors).
void notch_band(ksp::spektral_engine& e, double freq, int halfwidth = 2) {
    const int b = e.band_of_freq(freq);
    for (int d = -halfwidth; d <= halfwidth; ++d)
        e.set_curve_point(ksp::curve_level, b + d, 0.0);
}

void set_band(ksp::spektral_engine& e, int which, double freq, double value, int halfwidth = 2) {
    const int b = e.band_of_freq(freq);
    for (int d = -halfwidth; d <= halfwidth; ++d)
        e.set_curve_point(which, b + d, value);
}

}  // namespace


SCENARIO("unity settings reconstruct the input at one frame of latency") {
    auto e  = make_engine();
    auto in = sine(1000.0, 0.5);
    auto out = run(e, in);

    THEN("out[i] equals in[i - N] to near machine precision") {
        double max_err = 0.0;
        for (size_t i = c_N + 4 * 512; i < out.size(); ++i)
            max_err = std::max(max_err, std::abs(out[i] - in[i - c_N]));
        REQUIRE(max_err < 1e-6);
    }
}

SCENARIO("mix 0 is an exact latency-aligned dry passthrough") {
    auto e = make_engine();
    e.set_mix(0.0);
    e.set_smear(80.0);          // wet machinery churning must not leak
    e.set_rotate(200.0);
    auto in = sine(440.0, 0.4);
    auto out = run(e, in);

    THEN("output equals the input delayed by the FFT size") {
        double max_err = 0.0;
        for (size_t i = c_N; i < out.size(); ++i)
            max_err = std::max(max_err, std::abs(out[i] - in[i - c_N]));
        REQUIRE(max_err < 1e-9);
    }
}

SCENARIO("the level curve notches a band") {
    auto flat    = make_engine();
    auto notched = make_engine();
    notch_band(notched, 1000.0);

    auto in = sine(1000.0, 0.5, 0.4);
    for (size_t i = 0; i < in.size(); ++i)
        in[i] += 0.4 * std::sin(2.0 * ksp::k_pi * 250.0 * i / c_sr);

    auto out_flat  = run(flat, in);
    auto out_notch = run(notched, in);

    const size_t lo = at_s(0.2), hi = at_s(0.45);
    THEN("the notched band is attenuated by more than 20 dB") {
        REQUIRE(goertzel(out_notch, lo, hi, 1000.0) < 0.01 * goertzel(out_flat, lo, hi, 1000.0));
    }
    THEN("a distant band is untouched") {
        const double r = goertzel(out_notch, lo, hi, 250.0) / goertzel(out_flat, lo, hi, 250.0);
        REQUIRE(r > 0.9);
        REQUIRE(r < 1.1);
    }
}

SCENARIO("the delay curve positions a band in time") {
    auto e = make_engine();
    set_band(e, ksp::curve_delay, 1000.0, 500.0);   // 500 ms on the 1 kHz band

    auto in = sine(1000.0, 0.1, 0.5);
    in.resize(at_s(1.0), 0.0);
    auto out = run(e, in);

    THEN("the band is silent before its delay") {
        REQUIRE(goertzel(out, at_s(0.20), at_s(0.45), 1000.0) < 1e-6);
    }
    THEN("the band arrives after its delay") {
        REQUIRE(goertzel(out, at_s(0.53), at_s(0.75), 1000.0) > 1e-2);
    }
    THEN("an undelayed band still arrives immediately") {
        auto e2 = make_engine();
        set_band(e2, ksp::curve_delay, 1000.0, 500.0);
        auto in2 = sine(250.0, 0.1, 0.5);
        in2.resize(at_s(0.5), 0.0);
        auto out2 = run(e2, in2);
        REQUIRE(goertzel(out2, at_s(0.04), at_s(0.16), 250.0) > 1e-2);
    }
}

SCENARIO("per-band feedback produces decaying echoes") {
    auto e = make_engine();
    set_band(e, ksp::curve_delay, 1000.0, 250.0);
    set_band(e, ksp::curve_fb, 1000.0, 0.5);

    auto in = sine(1000.0, 0.1, 0.5);
    in.resize(at_s(1.5), 0.0);
    auto out = run(e, in);

    // The wet tap is post-delay (input -> delay -> output, like the original), so even the FIRST
    // arrival is at ~250 ms (+ one frame of latency); echoes follow at 250 ms spacing.
    const double a0 = std::sqrt(goertzel(out, at_s(0.27), at_s(0.44), 1000.0));
    const double a1 = std::sqrt(goertzel(out, at_s(0.52), at_s(0.69), 1000.0));
    const double a2 = std::sqrt(goertzel(out, at_s(0.76), at_s(0.93), 1000.0));

    THEN("each echo is roughly half the previous") {
        REQUIRE(a1 > 0.25 * a0);
        REQUIRE(a1 < 0.75 * a0);
        REQUIRE(a2 > 0.25 * a1);
        REQUIRE(a2 < 0.75 * a1);
    }
}

SCENARIO("freeze recirculates losslessly until released") {
    auto e = make_engine();
    std::vector<double> in = sine(1000.0, 0.5, 0.5);
    in.resize(at_s(4.0), 0.0);

    std::vector<double> out(in.size());
    e.process(in.data(), out.data(), at_s(0.5));
    e.set_freeze(true);
    e.process(in.data() + at_s(0.5), out.data() + at_s(0.5), at_s(3.0));
    e.set_freeze(false);
    e.process(in.data() + at_s(3.5), out.data() + at_s(3.5), in.size() - at_s(3.5));

    const double early = goertzel(out, at_s(1.0), at_s(1.2), 1000.0);
    const double late  = goertzel(out, at_s(3.2), at_s(3.4), 1000.0);
    const double freed = goertzel(out, at_s(3.7), at_s(3.9), 1000.0);

    THEN("the frozen band holds its level for seconds (within 1 dB)") {
        REQUIRE(late > early * 0.79);
        REQUIRE(late < early * 1.26);
    }
    THEN("releasing freeze silences it") {
        REQUIRE(freed < early * 0.01);
    }
}

SCENARIO("rotate sweeps energy upward through the spectrum") {
    auto still   = make_engine();
    auto rotated = make_engine();
    rotated.set_rotate(100.0);   // bins per second

    auto in = sine(500.0, 1.6, 0.5);
    auto out_still = run(still, in);
    auto out_rot   = run(rotated, in);

    THEN("the original frequency drains away") {
        const double before = goertzel(out_still, at_s(1.3), at_s(1.5), 500.0);
        const double after  = goertzel(out_rot, at_s(1.3), at_s(1.5), 500.0);
        REQUIRE(after < 0.05 * before);
    }
    THEN("the pitch of the wet signal rises (zero-crossing rate grows)") {
        const size_t zc_early = zero_crossings(out_rot, at_s(0.2), at_s(0.4));
        const size_t zc_late  = zero_crossings(out_rot, at_s(1.3), at_s(1.5));
        REQUIRE(zc_late > zc_early * 2);
    }
}

SCENARIO("smear diffuses a click into a tail") {
    auto dry_e   = make_engine();
    auto smear_e = make_engine();
    smear_e.set_smear(80.0);

    std::vector<double> in(at_s(1.2), 0.0);
    in[100] = 1.0;

    auto out_dry   = run(dry_e, in);
    auto out_smear = run(smear_e, in);

    THEN("tail energy grows by orders of magnitude") {
        const double tail_dry   = energy(out_dry, at_s(0.3), at_s(1.1));
        const double tail_smear = energy(out_smear, at_s(0.3), at_s(1.1));
        REQUIRE(tail_smear > 100.0 * (tail_dry + 1e-20));
        REQUIRE(tail_smear > 1e-8);
    }
}

SCENARIO("curve lists resample to the band count") {
    auto e = make_engine();
    const double pts[4] = { 0.0, 1.0, 0.5, 1.0 };
    e.set_curve(ksp::curve_level, pts, 4);

    THEN("endpoints and interior interpolate piecewise-linearly") {
        REQUIRE(e.curve_value(ksp::curve_level, 0) == 0.0);
        REQUIRE(std::abs(e.curve_value(ksp::curve_level, 53) - 1.0) < 0.02);    // x == 1.0
        REQUIRE(std::abs(e.curve_value(ksp::curve_level, 106) - 0.5) < 0.02);   // x == 2.0
        REQUIRE(e.curve_value(ksp::curve_level, 159) == 1.0);
        REQUIRE(std::abs(e.curve_value(ksp::curve_level, 27) - 0.509) < 0.03);  // mid-segment
    }
    THEN("a single [band value] poke touches only that band") {
        e.set_curve_point(ksp::curve_level, 80, 0.25);
        REQUIRE(e.curve_value(ksp::curve_level, 80) == 0.25);
        REQUIRE(std::abs(e.curve_value(ksp::curve_level, 79) - e.curve_value(ksp::curve_level, 81)) < 0.05);
    }
}

SCENARIO("preset recall morphs curves and scalars without discontinuities") {
    auto e = make_engine();
    e.set_mix(70.0);
    const double flat[1] = { 1.0 };
    e.set_curve(ksp::curve_level, flat, 1);
    e.store_preset(0);

    // radically different state
    const double ramp_pts[2] = { 0.0, 2.0 };
    e.set_curve(ksp::curve_level, ramp_pts, 2);
    const double dly[3] = { 0.0, 800.0, 100.0 };
    e.set_curve(ksp::curve_delay, dly, 3);
    e.set_mix(20.0);

    auto sig = [](size_t i) { return 0.3 * std::sin(2.0 * ksp::k_pi * 330.0 * i / c_sr); };
    size_t i = 0;
    for (; i < at_s(0.3); ++i)
        e.process(sig(i));

    REQUIRE(e.recall_preset(0, 0.1));
    REQUIRE(e.morphing());

    double prev = e.process(sig(i++));
    double maxjump = 0.0;
    for (; i < at_s(0.3) + at_s(0.4); ++i) {
        const double y = e.process(sig(i));
        maxjump = std::max(maxjump, std::abs(y - prev));
        prev = y;
    }
    THEN("no sample-to-sample click during the morph") {
        REQUIRE(maxjump < 0.25);
    }
    THEN("scalars and curves land exactly on the preset") {
        REQUIRE(!e.morphing());
        REQUIRE(std::abs(e.curve_value(ksp::curve_level, 100) - 1.0) < 1e-9);
        REQUIRE(std::abs(e.curve_value(ksp::curve_delay, 100) - 0.0) < 1e-9);
    }
}

SCENARIO("the Min wrapper instantiates with the documented defaults") {
    ext_main(nullptr);

    GIVEN("an instance of tap.spektral~") {
        test_wrapper<spektral> an_instance;
        spektral&              my_object = an_instance;

        THEN("attribute defaults match the spec") {
            REQUIRE(static_cast<int>(my_object.bands) == 160);
            REQUIRE(static_cast<double>(my_object.level) == 1.0);
            REQUIRE(static_cast<double>(my_object.fb) == 1.0);
            REQUIRE(static_cast<double>(my_object.rotate) == 0.0);
            REQUIRE(static_cast<double>(my_object.smear) == 0.0);
            REQUIRE(static_cast<int>(my_object.lfodest) == 0);
            REQUIRE(static_cast<bool>(my_object.freeze) == false);
            REQUIRE(static_cast<double>(my_object.mix) == 50.0);
            REQUIRE(my_object.engine().fftsize() == 2048);
            REQUIRE(my_object.engine().latency() == 2048);
        }
        THEN("curve and preset messages are callable") {
            my_object.gains(atoms { 0.0, 1.0, 0.5 });
            my_object.delays(atoms { 100.0, 200.0 });
            my_object.fbs(atoms { 0, 0.5 });   // [band value] poke
            my_object.store(atoms { 1 });
            my_object.recall(atoms { 1 });
            my_object.clear(atoms {});
            REQUIRE(my_object.engine().curve_value(ksp::curve_fb, 0) == 0.5);
        }
        THEN("a non-power-of-two fftsize request falls back to the default") {
            ksp::spektral_engine odd(1000);
            REQUIRE(odd.fftsize() == 2048);
            ksp::spektral_engine tiny_engine(512);   // "small" is a macro (char) in Windows rpcndr.h
            REQUIRE(tiny_engine.fftsize() == 512);
        }
    }
}
