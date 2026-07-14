/// @file
/// @brief      Offline renderer for the tap.ladder~ kernel — writes demo WAVs.
/// @details    Exercises ladder.h with no Max involved: the classic resonant sweep on a saw riff,
///             a self-oscillation "synth voice" played via per-sample cutoff control, a drive
///             A/B, a mode tour, and a preset morph. 48 kHz mono float32 WAVs.
///
///             Usage: ladder_render [output-directory]   (default: current directory)
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include <taptools/ladder.h>

using taptools::ladder::ladder_filter;
namespace k = taptools::ladder;

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

    // A simple saw-wave riff (naive saw — the filter is the star, and it tames the edge anyway).
    std::vector<double> saw_riff(double seconds, double amp = 0.35) {
        const double        notes[] = {55.0, 55.0, 82.41, 55.0, 65.41, 55.0, 98.0, 73.42};
        std::vector<double> sig(static_cast<size_t>(seconds * k_g_sr), 0.0);
        double              phase = 0.0;
        for (size_t i = 0; i < sig.size(); ++i) {
            const double t    = i / k_g_sr;
            const int    step = static_cast<int>(t * 4.0) % 8; // 16th-ish notes
            const double gate = (std::fmod(t * 4.0, 1.0) < 0.8) ? 1.0 : 0.0;
            phase += notes[step] / k_g_sr;
            phase -= std::floor(phase);
            sig[i] = amp * gate * (2.0 * phase - 1.0);
        }
        return sig;
    }

    std::vector<double> render(ladder_filter& f, const std::vector<double>& in) {
        std::vector<double> out(in.size());
        f.process(in.data(), out.data(), in.size());
        return out;
    }

} // namespace

int main(int argc, char** argv) {
    const std::string dir = (argc > 1) ? std::string(argv[1]) + "/" : "";

    // 1. The classic: resonant sweep over a saw riff.
    {
        ladder_filter f;
        f.prepare(k_g_sr);
        f.set_smooth_ms(0);
        f.set_resonance(0.75);
        f.set_frequency(120.0);
        f.snap();
        f.set_smooth_ms(9000.0);
        f.set_frequency(8000.0); // 9-second opening sweep
        write_wav(dir + "ladder_sweep_saw.wav", render(f, saw_riff(10.0)), k_g_sr);
    }

    // 2. Self-oscillation synth voice: ping it, then play the cutoff per sample.
    {
        ladder_filter f;
        f.prepare(k_g_sr);
        f.set_smooth_ms(0);
        f.set_resonance(1.05);
        f.set_gain(15.0); // self-oscillation sits well below full scale; bring it up for the demo
        const double        melody[] = {220.0, 261.63, 329.63, 440.0, 329.63, 261.63, 196.0, 220.0};
        const size_t        n        = static_cast<size_t>(8.0 * k_g_sr);
        std::vector<double> out(n);
        for (size_t i = 0; i < n; ++i) {
            const double t    = i / k_g_sr;
            const int    step = static_cast<int>(t) % 8;
            const double ping = (std::fmod(t, 1.0) < 1.0 / k_g_sr) ? 0.4 : 0.0; // one ping per note
            out[i]            = f.process(ping, melody[step]);
        }
        write_wav(dir + "ladder_selfosc_melody.wav", out, k_g_sr);
    }

    // 3. Drive A/B: four bars clean, four bars +18 dB into the ladder.
    {
        ladder_filter f;
        f.prepare(k_g_sr);
        f.set_smooth_ms(0);
        f.set_frequency(1200.0);
        f.set_resonance(0.5);
        auto                in = saw_riff(8.0);
        std::vector<double> out(in.size());
        const size_t        half = in.size() / 2;
        f.process(in.data(), out.data(), half);
        f.set_drive(18.0);
        f.set_gain(-9.0); // rough makeup
        f.process(in.data() + half, out.data() + half, in.size() - half);
        write_wav(dir + "ladder_drive_ab.wav", out, k_g_sr);
    }

    // 4. Mode tour: lp24 -> lp12 -> bp12 -> bp24 -> hp12 -> hp24, ~1.6 s each.
    {
        ladder_filter f;
        f.prepare(k_g_sr);
        f.set_smooth_ms(0);
        f.set_frequency(900.0);
        f.set_resonance(0.6);
        auto                in = saw_riff(9.6);
        std::vector<double> out(in.size());
        const size_t        seg = in.size() / 6;
        for (int m = 0; m < 6; ++m) {
            f.set_mode(m);
            f.process(in.data() + m * seg, out.data() + m * seg, seg);
        }
        write_wav(dir + "ladder_mode_tour.wav", out, k_g_sr);
    }

    // 5. Preset morph: dark and heavy -> open and screaming over 4 s.
    {
        ladder_filter f;
        f.prepare(k_g_sr);
        f.set_smooth_ms(0);
        f.set_frequency(150.0);
        f.set_resonance(0.4);
        f.set_drive(6.0);
        f.store_preset(0);
        f.set_frequency(5000.0);
        f.set_resonance(1.0);
        f.set_drive(15.0);
        f.set_gain(-6.0);
        f.store_preset(1);
        f.recall_preset(0, 0.0);

        auto                in = saw_riff(12.0);
        std::vector<double> out(in.size());
        const size_t        morph_at = static_cast<size_t>(4.0 * k_g_sr);
        f.process(in.data(), out.data(), morph_at);
        f.recall_preset(1, 4.0);
        f.process(in.data() + morph_at, out.data() + morph_at, in.size() - morph_at);
        write_wav(dir + "ladder_preset_morph.wav", out, k_g_sr);
    }

    return 0;
}
