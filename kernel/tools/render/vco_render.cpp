/// @file
/// @brief      Offline renderer for the tap.vco~ kernel — writes demo WAVs.
/// @details    Exercises vco.h with no Max involved: a PWM pad from two drifted instances, a
///             hard-sync sweep lead, a through-zero FM growl, the shape-morph tour, and a preset
///             morph. 48 kHz mono float32 WAVs, oscillator straight to disk — no effects.
///
///             Usage: vco_render [output-directory]   (default: current directory)
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include <taptools/vco.h>

using taptools::vco::vco_osc;
namespace k = taptools::vco;

namespace {

    constexpr double k_g_sr = 48000.0;

    bool write_wav(const std::string& path, const std::vector<double>& samples, double sr) {
        std::FILE* f = std::fopen(path.c_str(), "wb");
        if (!f) {
            std::fprintf(stderr, "cannot open %s\n", path.c_str());
            return false;
        }
        const uint32_t n          = static_cast<uint32_t>(samples.size());
        const uint32_t data_bytes = n * 4;
        const uint32_t rate       = static_cast<uint32_t>(sr);

        auto u16 = [&](uint16_t v) { std::fwrite(&v, 2, 1, f); };
        auto u32 = [&](uint32_t v) { std::fwrite(&v, 4, 1, f); };

        std::fwrite("RIFF", 1, 4, f);
        u32(36 + data_bytes);
        std::fwrite("WAVE", 1, 4, f);
        std::fwrite("fmt ", 1, 4, f);
        u32(16);
        u16(3);
        u16(1);
        u32(rate);
        u32(rate * 4);
        u16(4);
        u16(32);
        std::fwrite("data", 1, 4, f);
        u32(data_bytes);
        for (double s : samples) {
            const float v = static_cast<float>(s);
            std::fwrite(&v, 4, 1, f);
        }
        std::fclose(f);
        std::printf("wrote %s (%.1f s)\n", path.c_str(), n / sr);
        return true;
    }

} // namespace

int main(int argc, char** argv) {
    const std::string dir = (argc > 1) ? std::string(argv[1]) + "/" : "";

    // 1. PWM pad: two drifted, detuned pulses with a slow width sweep.
    {
        vco_osc a, b;
        a.prepare(k_g_sr);
        b.prepare(k_g_sr);
        a.set_smooth_ms(0);
        b.set_smooth_ms(0);
        a.set_seed(11u);
        b.set_seed(23u);
        a.clear();
        b.clear();
        for (vco_osc* o : {&a, &b}) {
            o->set_shape(k::wave_pulse);
            o->set_drift(8.0);
        }
        a.set_frequency(110.0);
        b.set_frequency(110.0);
        b.set_detune(9.0);
        const size_t        n = static_cast<size_t>(10.0 * k_g_sr);
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            const double t   = i / k_g_sr;
            const double pwv = 50.0 + 38.0 * std::sin(2.0 * k::k_pi * 0.13 * t);
            if (i % 64 == 0) { // control-rate pw updates, smoothed by the ramps
                a.set_smooth_ms(4.0);
                b.set_smooth_ms(4.0);
                a.set_pw(pwv);
                b.set_pw(100.0 - pwv);
            }
            out[i] = 0.28 * (a.process() + b.process());
        }
        write_wav(dir + "vco_pwm_pad.wav", out, k_g_sr);
    }

    // 2. Hard-sync sweep: slave swept 1x -> 4x over a fixed master — the classic scream.
    {
        vco_osc master, slave;
        master.prepare(k_g_sr);
        slave.prepare(k_g_sr);
        master.set_smooth_ms(0);
        slave.set_smooth_ms(0);
        master.set_shape(k::wave_sine);
        master.set_frequency(82.41);
        slave.set_shape(k::wave_saw);
        const size_t        n = static_cast<size_t>(10.0 * k_g_sr);
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            const double t     = i / k_g_sr;
            const double sweep = 82.41 * (1.0 + 3.0 * 0.5 * (1.0 - std::cos(2.0 * k::k_pi * 0.1 * t)));
            const double m     = master.process();
            out[i]             = 0.4 * slave.process_at(sweep, 0.0, m);
        }
        write_wav(dir + "vco_sync_sweep.wav", out, k_g_sr);
    }

    // 3. Through-zero FM growl: modulation depth swept way past the carrier.
    {
        vco_osc carrier, modulator;
        carrier.prepare(k_g_sr);
        modulator.prepare(k_g_sr);
        carrier.set_smooth_ms(0);
        modulator.set_smooth_ms(0);
        carrier.set_shape(k::wave_sine);
        carrier.set_frequency(146.83);
        modulator.set_shape(k::wave_sine);
        modulator.set_frequency(146.83); // 1:1 — the classic FM growl ratio
        const size_t        n = static_cast<size_t>(10.0 * k_g_sr);
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            const double t     = i / k_g_sr;
            const double depth = 1200.0 * 0.5 * (1.0 - std::cos(2.0 * k::k_pi * 0.1 * t));
            const double m     = modulator.process();
            out[i]             = 0.4 * carrier.process(depth * m);
        }
        write_wav(dir + "vco_tzfm_growl.wav", out, k_g_sr);
    }

    // 4. The shape-morph tour: sine -> triangle -> saw -> pulse over 9 s.
    {
        vco_osc o;
        o.prepare(k_g_sr);
        o.set_smooth_ms(0);
        o.set_frequency(146.83);
        o.set_shape(0.0);
        o.snap();
        o.set_smooth_ms(9000.0);
        o.set_shape(3.0);
        const size_t        n = static_cast<size_t>(10.0 * k_g_sr);
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            out[i] = 0.4 * o.process();
        }
        write_wav(dir + "vco_shape_morph.wav", out, k_g_sr);
    }

    // 5. Preset morph: airy drifted sine -> hard nasal pulse over 4 s, mid-note.
    {
        vco_osc o;
        o.prepare(k_g_sr);
        o.set_smooth_ms(0);
        o.set_frequency(220.0);
        o.set_shape(0.2);
        o.set_drift(12.0);
        o.store_preset(0);
        o.set_frequency(220.0);
        o.set_shape(3.0);
        o.set_pw(12.0);
        o.set_drift(0.0);
        o.store_preset(1);
        o.recall_preset(0, 0.0);

        const size_t        n = static_cast<size_t>(12.0 * k_g_sr);
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            if (i == static_cast<size_t>(4.0 * k_g_sr)) {
                o.recall_preset(1, 4.0);
            }
            out[i] = 0.4 * o.process();
        }
        write_wav(dir + "vco_preset_morph.wav", out, k_g_sr);
    }

    return 0;
}
