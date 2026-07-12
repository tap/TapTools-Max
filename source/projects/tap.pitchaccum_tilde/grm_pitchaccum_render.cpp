/// @file
/// @brief      Offline renderer for the tap.pitchaccum~ kernel — writes demo WAVs.
/// @details    Exercises grm_pitchaccum.h with no Max involved. Renders dual-shadow harmonies,
///             the accumulating feedback spiral, the modulation section, and a preset morph as
///             48 kHz mono float32 WAVs.
///
///             Usage: grm_pitchaccum_render [output-directory]   (default: current directory)
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "grm_pitchaccum.h"

using taptools::pitchaccum::accum_bank;
namespace k = taptools::pitchaccum;

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
        u16(3); // IEEE float
        u16(1); // mono
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

    // A plucked tone: decaying sine with a soft attack.
    void pluck(std::vector<double>& sig, double t_start, double freq, double amp = 0.5, double decay_s = 0.8) {
        const size_t start = static_cast<size_t>(t_start * k_g_sr);
        const size_t len   = static_cast<size_t>(decay_s * 4.0 * k_g_sr);
        for (size_t i = 0; i < len && start + i < sig.size(); ++i) {
            const double t   = i / k_g_sr;
            const double env = std::exp(-t / decay_s) * std::min(1.0, t / 0.005);
            sig[start + i] += amp * env * std::sin(2.0 * k::k_pi * freq * t);
        }
    }

    std::vector<double> render(accum_bank& bank, const std::vector<double>& in) {
        std::vector<double> out(in.size());
        bank.process(in.data(), out.data(), in.size());
        return out;
    }

} // namespace

int main(int argc, char** argv) {
    const std::string dir = (argc > 1) ? std::string(argv[1]) + "/" : "";

    // 1. Dual shadows: two delayed harmony voices around a plucked line.
    {
        accum_bank bank;
        bank.prepare(k_g_sr);
        bank.set_smooth_ms(0);
        bank.set_trans(0, 4.0);  // major third up
        bank.set_trans(1, -5.0); // fourth down
        bank.set_delay(0, 250.0);
        bank.set_delay(1, 420.0);
        bank.set_voice_gain(0, 70.0);
        bank.set_voice_gain(1, 70.0);
        bank.set_mix(55.0);
        std::vector<double> in(static_cast<size_t>(8.0 * k_g_sr), 0.0);
        const double        notes[] = {220.0, 277.18, 329.63, 440.0};
        for (int i = 0; i < 4; ++i) {
            pluck(in, 0.4 + 1.6 * i, notes[i]);
        }
        write_wav(dir + "pitchaccum_dual_shadows.wav", render(bank, in), k_g_sr);
    }

    // 2. The signature: accumulating feedback spiral (each pass +7 semitones).
    {
        accum_bank bank;
        bank.prepare(k_g_sr);
        bank.set_smooth_ms(0);
        bank.set_trans(0, 7.0);
        bank.set_delay(0, 350.0);
        bank.set_fb(0, 85.0);
        bank.set_voice_gain(0, 90.0);
        bank.set_voice_gain(1, 0.0);
        bank.set_mix(70.0);
        std::vector<double> in(static_cast<size_t>(12.0 * k_g_sr), 0.0);
        pluck(in, 0.3, 220.0, 0.6, 0.4);
        pluck(in, 6.0, 146.83, 0.6, 0.4);
        write_wav(dir + "pitchaccum_spiral_up.wav", render(bank, in), k_g_sr);
    }

    // 3. Downward spiral with random modulation — the darker GRM texture.
    {
        accum_bank bank;
        bank.prepare(k_g_sr);
        bank.set_smooth_ms(0);
        bank.set_trans(0, -5.0);
        bank.set_delay(0, 280.0);
        bank.set_fb(0, 80.0);
        bank.set_voice_gain(0, 90.0);
        bank.set_trans(1, -0.15); // subtle detuned shadow on top
        bank.set_voice_gain(1, 40.0);
        bank.set_randdepth(0.6);
        bank.set_randrate(3.0);
        bank.set_mix(65.0);
        std::vector<double> in(static_cast<size_t>(12.0 * k_g_sr), 0.0);
        pluck(in, 0.3, 440.0, 0.6, 0.5);
        pluck(in, 6.0, 587.33, 0.6, 0.5);
        write_wav(dir + "pitchaccum_spiral_down_rand.wav", render(bank, in), k_g_sr);
    }

    // 4. LFO modulation tour: vibrato-to-seasick sweep of moddepth while a tone sustains.
    {
        accum_bank bank;
        bank.prepare(k_g_sr);
        bank.set_smooth_ms(0);
        bank.set_voice_gain(0, 80.0);
        bank.set_voice_gain(1, 80.0);
        bank.set_modfreq(0.8);
        bank.set_modphase(180.0);
        bank.set_mix(80.0);
        bank.snap();
        bank.set_smooth_ms(9000.0);
        bank.set_moddepth(5.0); // ramp depth 0 -> 5 st over 9 s
        std::vector<double> in(static_cast<size_t>(10.0 * k_g_sr), 0.0);
        for (double t = 0.2; t < 9.0; t += 0.02) {
            pluck(in, t, 261.63, 0.03, 2.0); // dense re-triggering = sustained tone
        }
        write_wav(dir + "pitchaccum_lfo_tour.wav", render(bank, in), k_g_sr);
    }

    // 5. Preset morph: clean harmony -> wild accumulation, morphed over 4 s mid-audio.
    {
        accum_bank bank;
        bank.prepare(k_g_sr);
        bank.set_smooth_ms(0);
        bank.set_trans(0, 3.86); // hair under a major third
        bank.set_trans(1, -0.1);
        bank.set_delay(0, 120.0);
        bank.set_delay(1, 60.0);
        bank.set_fb(0, 20.0);
        bank.set_voice_gain(0, 60.0);
        bank.set_voice_gain(1, 60.0);
        bank.set_mix(45.0);
        bank.store_preset(0);
        bank.set_trans(0, 12.0);
        bank.set_trans(1, -12.0);
        bank.set_delay(0, 500.0);
        bank.set_delay(1, 750.0);
        bank.set_fb(0, 88.0);
        bank.set_fb(1, 88.0);
        bank.set_mix(80.0);
        bank.store_preset(1);
        bank.recall_preset(0, 0.0);

        std::vector<double> in(static_cast<size_t>(14.0 * k_g_sr), 0.0);
        for (int i = 0; i < 8; ++i) {
            pluck(in, 0.4 + 1.5 * i, i % 2 ? 293.66 : 196.0, 0.5, 0.6);
        }
        std::vector<double> out(in.size());
        const size_t        morph_at = static_cast<size_t>(4.0 * k_g_sr);
        bank.process(in.data(), out.data(), morph_at);
        bank.recall_preset(1, 4.0);
        bank.process(in.data() + morph_at, out.data() + morph_at, in.size() - morph_at);
        write_wav(dir + "pitchaccum_preset_morph.wav", out, k_g_sr);
    }

    return 0;
}
