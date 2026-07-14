/// @file
/// @brief      Offline renderer for the tap.5comb~ kernel — writes demo WAVs for listening checks.
/// @details    Exercises grm_comb.h with no Max involved (the kernel's portability, demonstrated).
///             Renders a handful of scenarios — defaults, high resonance, a master-frequency glide,
///             a preset morph, and a warp/phase tour — as 48 kHz mono float32 WAVs.
///
///             Usage: grm_comb_render [output-directory]   (default: current directory)
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include <taptools/grm_comb.h>

using taptools::fivecomb::comb_bank;
namespace k = taptools::fivecomb;

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
        const uint32_t byte_rate  = rate * 4;

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
        u32(byte_rate);
        u16(4);  // block align
        u16(32); // bits
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

    // Deterministic white noise in [-1, 1].
    struct lcg {
        uint32_t state{22222};
        double   operator()() {
            state = state * 1664525u + 1013904223u;
            return (state / 2147483648.0) - 1.0;
        }
    };

    std::vector<double> render(comb_bank& bank, const std::vector<double>& in) {
        std::vector<double> out(in.size());
        bank.process(in.data(), out.data(), in.size());
        return out;
    }

    std::vector<double> noise_burst(double burst_s, double total_s, double amp = 0.5) {
        std::vector<double> sig(static_cast<size_t>(total_s * k_g_sr), 0.0);
        lcg                 rng;
        const size_t        nb = static_cast<size_t>(burst_s * k_g_sr);
        for (size_t i = 0; i < nb && i < sig.size(); ++i) {
            sig[i] = amp * rng();
        }
        return sig;
    }

} // namespace

int main(int argc, char** argv) {
    const std::string dir = (argc > 1) ? std::string(argv[1]) + "/" : "";

    // 1. Defaults: a noise burst through the factory tuning.
    {
        comb_bank bank;
        bank.prepare(k_g_sr);
        write_wav(dir + "5comb_defaults_burst.wav", render(bank, noise_burst(0.1, 5.0)), k_g_sr);
    }

    // 2. High resonance: sparse clicks, long clean rings (the res=100 no-clipper behavior).
    {
        comb_bank bank;
        bank.prepare(k_g_sr);
        bank.set_smooth_ms(0);
        for (int v = 0; v < k::k_voices; ++v) {
            bank.set_res(v, 92.0);
        }
        std::vector<double> sig(static_cast<size_t>(8.0 * k_g_sr), 0.0);
        for (size_t i = 0; i < sig.size(); i += static_cast<size_t>(2.5 * k_g_sr)) {
            sig[i] = 0.8;
        }
        write_wav(dir + "5comb_res92_clicks.wav", render(bank, sig), k_g_sr);
    }

    // 3. Master-frequency glide: the per-sample smoothing that the legacy object lacked.
    {
        comb_bank bank;
        bank.prepare(k_g_sr);
        bank.set_smooth_ms(0);
        for (int v = 0; v < k::k_voices; ++v) {
            bank.set_res(v, 85.0);
        }
        bank.set_freq_master(0.5);
        bank.snap();
        bank.set_smooth_ms(8000.0); // ride one long ramp...
        bank.set_freq_master(2.0);  // ...from 0.5x to 2x over 8 s
        write_wav(dir + "5comb_master_freq_glide.wav", render(bank, noise_burst(8.0, 10.0, 0.25)), k_g_sr);
    }

    // 4. Preset morph: sustained noise while everything interpolates between two presets.
    {
        comb_bank bank;
        bank.prepare(k_g_sr);
        bank.set_smooth_ms(0);
        // preset A: the defaults, darker
        for (int v = 0; v < k::k_voices; ++v) {
            bank.set_res(v, 80.0);
            bank.set_lp(v, 3000.0);
        }
        bank.store_preset(0);
        // preset B: a high, bright, dissonant cluster
        const double freqs_b[k::k_voices] = {523.25, 587.33, 622.25, 830.61, 987.77};
        for (int v = 0; v < k::k_voices; ++v) {
            bank.set_freq(v, freqs_b[v]);
            bank.set_res(v, 90.0);
            bank.set_lp(v, 20000.0);
        }
        bank.store_preset(1);
        bank.recall_preset(0, 0.0);

        std::vector<double> in = noise_burst(9.5, 10.0, 0.2);
        std::vector<double> out(in.size());
        const size_t        morph_at = static_cast<size_t>(2.0 * k_g_sr);
        bank.process(in.data(), out.data(), morph_at);
        bank.recall_preset(1, 4.0); // 4-second morph, mid-audio
        bank.process(in.data() + morph_at, out.data() + morph_at, in.size() - morph_at);
        write_wav(dir + "5comb_preset_morph.wav", out, k_g_sr);
    }

    // 5. Warp then phase: clicks while warp ramps up, then phase sweeps to odd-harmonics-only.
    {
        comb_bank bank;
        bank.prepare(k_g_sr);
        bank.set_smooth_ms(0);
        for (int v = 0; v < k::k_voices; ++v) {
            bank.set_res(v, 88.0);
        }
        std::vector<double> in(static_cast<size_t>(12.0 * k_g_sr), 0.0);
        for (size_t i = 0; i < in.size(); i += static_cast<size_t>(1.5 * k_g_sr)) {
            in[i] = 0.8;
        }
        std::vector<double> out(in.size());
        const size_t        half = in.size() / 2;
        bank.set_smooth_ms(6000.0);
        bank.set_warp(100.0);
        bank.process(in.data(), out.data(), half);
        bank.set_smooth_ms(0);
        bank.set_warp(0.0);
        bank.set_smooth_ms(6000.0);
        bank.set_phase(100.0);
        bank.process(in.data() + half, out.data() + half, in.size() - half);
        write_wav(dir + "5comb_warp_then_phase.wav", out, k_g_sr);
    }

    return 0;
}
