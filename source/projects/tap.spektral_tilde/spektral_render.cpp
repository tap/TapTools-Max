/// @file
/// @brief      Offline renderer for the tap.spektral~ kernel — writes demo WAVs.
/// @details    Exercises spektral.h with no Max involved: the spectral swoosh (rising per-band
///             delay ramp), a freeze pad, rotate on a harmonic tone, smear-as-reverb on
///             percussion, and a preset morph. 48 kHz mono float32 WAVs.
///
///             Usage: spektral_render [output-directory]   (default: current directory)
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include "spektral.h"

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

using taptools::spektral::spektral_engine;
namespace k = taptools::spektral;

namespace {

constexpr double g_sr = 48000.0;

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

// Percussive hit: filtered-noise-ish burst via detuned partials with a sharp envelope.
void hit(std::vector<double>& sig, double t_start, double base, double amp = 0.6) {
    const size_t start = static_cast<size_t>(t_start * g_sr);
    const size_t len   = static_cast<size_t>(0.35 * g_sr);
    const double partials[5] = { 1.0, 2.76, 5.4, 8.93, 13.34 };   // bell-ish
    for (size_t i = 0; i < len && start + i < sig.size(); ++i) {
        const double t   = i / g_sr;
        const double env = std::exp(-t * 18.0) * std::min(1.0, t / 0.002);
        double s = 0.0;
        for (double p : partials)
            s += std::sin(2.0 * k::k_pi * base * p * t) / p;
        sig[start + i] += amp * env * s * 0.5;
    }
}

void pluck(std::vector<double>& sig, double t_start, double freq, double amp = 0.5, double decay_s = 0.8) {
    const size_t start = static_cast<size_t>(t_start * g_sr);
    const size_t len   = static_cast<size_t>(decay_s * 4.0 * g_sr);
    for (size_t i = 0; i < len && start + i < sig.size(); ++i) {
        const double t   = i / g_sr;
        const double env = std::exp(-t / decay_s) * std::min(1.0, t / 0.005);
        sig[start + i] += amp * env * (std::sin(2.0 * k::k_pi * freq * t)
                                       + 0.4 * std::sin(2.0 * k::k_pi * freq * 2.0 * t)
                                       + 0.2 * std::sin(2.0 * k::k_pi * freq * 3.0 * t));
    }
}

std::vector<double> render(spektral_engine& e, const std::vector<double>& in) {
    std::vector<double> out(in.size());
    e.process(in.data(), out.data(), in.size());
    return out;
}

}  // namespace


int main(int argc, char** argv) {
    const std::string dir = (argc > 1) ? std::string(argv[1]) + "/" : "";

    // 1. The spectral swoosh: delay rises with frequency, so every hit smears upward in time.
    {
        spektral_engine e;
        e.prepare(g_sr);
        e.set_smooth_ms(0);
        const double dly[2] = { 0.0, 1500.0 };   // low bands dry, highs 1.5 s late
        e.set_curve(k::curve_delay, dly, 2);
        const double fb[2] = { 0.1, 0.55 };
        e.set_curve(k::curve_fb, fb, 2);
        e.set_mix(60.0);
        std::vector<double> in(static_cast<size_t>(10.0 * g_sr), 0.0);
        for (int i = 0; i < 6; ++i)
            hit(in, 0.4 + 1.4 * i, 90.0 + 25.0 * (i % 3));
        write_wav(dir + "spektral_swoosh.wav", render(e, in), g_sr);
    }

    // 2. Freeze pad: capture a chord, hold it for seconds, release.
    {
        spektral_engine e;
        e.prepare(g_sr);
        e.set_smooth_ms(0);
        e.set_mix(85.0);
        std::vector<double> in(static_cast<size_t>(12.0 * g_sr), 0.0);
        pluck(in, 0.2, 196.0, 0.4, 1.5);
        pluck(in, 0.25, 246.94, 0.4, 1.5);
        pluck(in, 0.3, 293.66, 0.4, 1.5);
        pluck(in, 9.0, 220.0, 0.5, 0.8);   // after release
        std::vector<double> out(in.size());
        const size_t t1 = static_cast<size_t>(1.2 * g_sr);
        const size_t t2 = static_cast<size_t>(8.5 * g_sr);
        e.process(in.data(), out.data(), t1);
        e.set_freeze(true);
        e.process(in.data() + t1, out.data() + t1, t2 - t1);
        e.set_freeze(false);
        e.process(in.data() + t2, out.data() + t2, in.size() - t2);
        write_wav(dir + "spektral_freeze_pad.wav", out, g_sr);
    }

    // 3. Rotate: a sustained chord climbs the spectrum, then falls (LFO bandshift on top).
    {
        spektral_engine e;
        e.prepare(g_sr);
        e.set_smooth_ms(0);
        e.set_mix(90.0);
        e.set_lfodest(k::dest_bandshift);
        e.set_lforate(0.15);
        e.set_lfodepth(60.0);
        std::vector<double> in(static_cast<size_t>(12.0 * g_sr), 0.0);
        for (double t = 0.2; t < 11.0; t += 0.05) {
            pluck(in, t, 130.81, 0.06, 1.0);
            pluck(in, t, 196.0, 0.045, 1.0);
        }
        write_wav(dir + "spektral_rotate_lfo.wav", render(e, in), g_sr);
    }

    // 4. Smear as reverb: dry percussion vs heavy amplitude diffusion.
    {
        spektral_engine e;
        e.prepare(g_sr);
        e.set_smooth_ms(0);
        e.set_smear(70.0);
        e.set_mix(65.0);
        std::vector<double> in(static_cast<size_t>(10.0 * g_sr), 0.0);
        for (int i = 0; i < 8; ++i)
            hit(in, 0.4 + 1.1 * i, 120.0 + 40.0 * (i % 4), 0.5);
        write_wav(dir + "spektral_smear_verb.wav", render(e, in), g_sr);
    }

    // 5. Preset morph: dry-ish EQ preset -> deep multiband echo preset over 5 s.
    {
        spektral_engine e;
        e.prepare(g_sr);
        e.set_smooth_ms(0);
        const double flat[1] = { 1.0 };
        e.set_curve(k::curve_level, flat, 1);
        const double no_dly[1] = { 0.0 };
        e.set_curve(k::curve_delay, no_dly, 1);
        e.set_mix(40.0);
        e.store_preset(0);

        const double comb_lvl[8] = { 1.0, 0.1, 1.0, 0.1, 1.0, 0.1, 1.0, 0.4 };
        e.set_curve(k::curve_level, comb_lvl, 8);
        const double stagger[5] = { 100.0, 700.0, 300.0, 1100.0, 500.0 };
        e.set_curve(k::curve_delay, stagger, 5);
        const double fb2[2] = { 0.5, 0.7 };
        e.set_curve(k::curve_fb, fb2, 2);
        e.set_mix(75.0);
        e.store_preset(1);
        e.recall_preset(0, 0.0);

        std::vector<double> in(static_cast<size_t>(16.0 * g_sr), 0.0);
        const double notes[4] = { 146.83, 174.61, 220.0, 174.61 };
        for (int i = 0; i < 10; ++i)
            pluck(in, 0.4 + 1.4 * i, notes[i % 4], 0.5, 0.5);
        std::vector<double> out(in.size());
        const size_t morph_at = static_cast<size_t>(5.0 * g_sr);
        e.process(in.data(), out.data(), morph_at);
        e.recall_preset(1, 5.0);
        e.process(in.data() + morph_at, out.data() + morph_at, in.size() - morph_at);
        write_wav(dir + "spektral_preset_morph.wav", out, g_sr);
    }

    return 0;
}
