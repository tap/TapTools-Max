/// @file
/// @brief      tap.convolve~ — true-stereo partitioned convolution (convolution reverb / IR processor).
/// @details    A general-purpose real-time convolver: it convolves a stereo input with an impulse
///             response held in a `buffer~`. The IR can be as long as a reverb tail (seconds), so the
///             engine uses **uniformly-partitioned overlap-save (UPOLS)** convolution — the IR is split
///             into equal blocks, each transformed once with an in-house radix-2 FFT, and the running
///             output is formed by a frequency-domain multiply-accumulate over a delay line of past
///             input spectra. Latency is one partition (`blocksize` samples); everything else is exact
///             linear convolution.
///
///             **True stereo.** Four IR paths describe the full 2×2 response:
///                 out_l = in_l ∗ h_LL + in_r ∗ h_RL
///                 out_r = in_l ∗ h_LR + in_r ∗ h_RR
///             The two input channels are transformed once per block and shared across the four paths.
///             How the four paths are pulled from the bound `buffer~` depends on its channel count:
///               • 4+ channels → true stereo: ch1=LL, ch2=LR, ch3=RL, ch4=RR
///               • 2  channels → stereo IR:  ch1=LL, ch2=RR, no cross-feed (LR=RL=0)
///               • 1  channel  → mono IR:    LL=RR=ch1, no cross-feed (stereo image preserved)
///
///             **Click-free IR swaps.** Re-transforming the IR happens off the audio thread (a deferred
///             `queue`) into an inactive double-buffered slot; a single atomic publishes the new slot,
///             so loading a new IR while audio runs never reallocates or tears the table the perform
///             loop is reading. Engine geometry (`blocksize`/`maxsize`) is only (re)allocated in
///             `dspsetup`, where the perform routine is guaranteed not to be running.
///
///             All DSP is plain portable C++ — no Jamoma, no min-lib, no external FFT library. Written
///             standalone for the revival; it pairs with the algorithmic `tap.verb~` as the second
///             reverb flavour. Deferred optimisation (noted below): the multiply-accumulate and the IR
///             tables use the full complex spectrum; a real-input half-spectrum (N/2+1 bins) form would
///             halve both the CPU and the memory.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include <taptools/conv_engine.h> // the Min-free, unit-tested DSP core (see the header for the algorithm)

#include "c74_min.h"

using namespace c74::min;
using taptools::conv_engine;

// The convolution DSP lives in taptools::conv_engine. Everything below is the Min wrapper:
// reading the IR from a buffer~, dry/wet, predelay, output gain, energy normalisation, bypass/mute.
// ------------------------------------------------------------------------------------------------
class convolve : public object<convolve>, public vector_operator<> {
  public:
    MIN_DESCRIPTION{"True-stereo partitioned convolution: convolve a stereo signal with an impulse "
                    "response stored in a buffer~ (a convolution reverb / IR processor). Self-contained "
                    "— no pfft~ host needed. Latency is one 'blocksize'."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.verb~, buffer~, pfft~, tap.comb~"};

    inlet<>  m_in_l{this, "(signal) left input"};
    inlet<>  m_in_r{this, "(signal) right input"};
    outlet<> m_out_l{this, "(signal) left output", "signal"};
    outlet<> m_out_r{this, "(signal) right output", "signal"};

    // Rebuilds the IR whenever the bound buffer~ is (re)set or its contents change. The callback may
    // arrive on any thread, so it only flags a deferred rebuild.
    buffer_reference m_buffer{this,
                              MIN_FUNCTION{
                                  m_rebuild.set();
                                  return {};
                              }};

    convolve(const atoms& args = {}) {
        if (!args.empty()) {
            m_buffer.set(args[0]);
        }
        if (args.size() > 1) {
            const int b = static_cast<int>(args[1]);
            if (is_power_of_two(b)) {
                m_blocksize_pending = b;
            }
        }
    }

    argument<symbol> name_arg{this, "buffer", "Name of the buffer~ holding the impulse response."};
    argument<int>    block_arg{this, "blocksize", "FFT partition size in samples (power of two)."};

    attribute<number> mix{this, "mix", 100.0, setter{MIN_FUNCTION{
                              return {std::clamp(static_cast<double>(args[0]), 0.0, 100.0)};
                          }},
                          description{"Dry/wet mix as a percentage (0 = dry, 100 = fully wet convolution)."}};

    attribute<number> gain{this, "gain", 0.0, setter{MIN_FUNCTION{
                               m_gain_lin = std::pow(10.0, static_cast<double>(args[0]) / 20.0);
                               return {args[0]};
                           }},
                           description{"Output gain in dB, applied to the wet+dry mix."}};

    attribute<number> predelay{this, "predelay", 0.0, setter{MIN_FUNCTION{
                                   return {std::clamp(static_cast<double>(args[0]), 0.0, k_max_predelay_ms)};
                               }},
                               description{"Pre-delay in milliseconds applied to the wet signal (0–2000)."}};

    attribute<bool> normalize{this, "normalize", true, setter{MIN_FUNCTION{
                                  const bool v = args[0];
                                  m_rebuild.set(); // re-derive the IR scale
                                  return {v};
                              }},
                              description{"Energy-normalise the impulse response so its overall level is "
                                          "consistent regardless of the IR's loudness or length."}};

    attribute<int> blocksize{this, "blocksize", 512, setter{MIN_FUNCTION{
                                 int b = static_cast<int>(args[0]);
                                 if (!is_power_of_two(b)) {
                                     b = next_power_of_two(b);
                                 }
                                 b                   = std::clamp(b, 16, 8192);
                                 m_blocksize_pending = b;
                                 return {b};
                             }},
                             description{"FFT partition size in samples (a power of two). This is also the "
                                         "processing latency. Changes take effect when the DSP chain is next "
                                         "started."}};

    attribute<int> maxsize{this, "maxsize", 131072, setter{MIN_FUNCTION{
                               int m             = std::max(static_cast<int>(args[0]), 256);
                               m_maxsize_pending = m;
                               return {m};
                           }},
                           description{"Maximum impulse-response length in samples that can be loaded. Longer "
                                       "IRs are truncated. Changes take effect when the DSP chain is next "
                                       "started (larger values use more memory)."}};

    attribute<bool> bypass{this, "bypass", false, description{"Pass the input through unprocessed."}};
    attribute<bool> mute{this, "mute", false, description{"Silence the output."}};

    message<> clear{this, "clear", "Flush the reverb tail (clears the input history and pending output).",
                    MIN_FUNCTION{
                        m_engine.clear();
                        for (auto& r : m_pd_ring) {
                            std::fill(r.begin(), r.end(), 0.0);
                        }
                        m_pd_w = 0;
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Configure the engine when the DSP chain starts.",
                       MIN_FUNCTION{
                           m_sr = samplerate();

                           const int B    = m_blocksize_pending;
                           const int cap  = std::max(m_maxsize_pending, B);
                           const int Pmax = (cap + B - 1) / B;
                           m_engine.configure(B, Pmax);

                           m_pd_len = static_cast<int>(k_max_predelay_ms * 0.001 * m_sr) + 2;
                           for (auto& r : m_pd_ring) {
                               r.assign(m_pd_len, 0.0);
                           }
                           m_pd_w = 0;

                           m_gain_lin = std::pow(10.0, static_cast<double>(gain) / 20.0);
                           m_rebuild.set(); // (re)load the IR from the bound buffer, off-thread
                           return {};
                       }};

    void operator()(audio_bundle input, audio_bundle output) override {
        const long    n     = input.frame_count();
        const double* in_l  = input.samples(0);
        const double* in_r  = input.samples(1);
        double*       out_l = output.samples(0);
        double*       out_r = output.samples(1);

        if (mute || !m_engine.configured()) {
            for (long i = 0; i < n; ++i) {
                out_l[i] = 0.0;
                out_r[i] = 0.0;
            }
            return;
        }
        if (bypass) {
            for (long i = 0; i < n; ++i) {
                out_l[i] = in_l[i];
                out_r[i] = in_r[i];
            }
            return;
        }

        // Wet convolution written in place to the output buffers.
        m_engine.process(in_l, in_r, out_l, out_r, n);

        const double wet = static_cast<double>(mix) * 0.01;
        const double dry = 1.0 - wet;
        const double g   = m_gain_lin;
        const int    D   = std::min(m_pd_len - 1, static_cast<int>(static_cast<double>(predelay) * 0.001 * m_sr + 0.5));

        for (long i = 0; i < n; ++i) {
            double wl = out_l[i];
            double wr = out_r[i];

            if (D > 0 && m_pd_len > 0) {
                m_pd_ring[0][m_pd_w] = wl;
                m_pd_ring[1][m_pd_w] = wr;
                int rd               = m_pd_w - D;
                if (rd < 0) {
                    rd += m_pd_len;
                }
                wl     = m_pd_ring[0][rd];
                wr     = m_pd_ring[1][rd];
                m_pd_w = (m_pd_w + 1) % m_pd_len;
            }

            out_l[i] = (dry * in_l[i] + wet * wl) * g;
            out_r[i] = (dry * in_r[i] + wet * wr) * g;
        }
    }

  private:
    static constexpr double k_max_predelay_ms = 2000.0;

    static bool is_power_of_two(int v) { return v > 0 && (v & (v - 1)) == 0; }
    static int  next_power_of_two(int v) {
        int p = 1;
        while (p < v) {
            p <<= 1;
        }
        return p;
    }

    // Deferred (main-thread) IR (re)build from the bound buffer~.
    queue<> m_rebuild{this,
                      MIN_FUNCTION{
                          rebuild_ir();
                          return {};
                      }};

    void rebuild_ir() {
        if (!m_engine.configured()) {
            return;
        }

        buffer_lock<> b(m_buffer);
        if (!b.valid()) {
            return; // nothing bound / buffer gone — keep whatever IR we had
        }

        const int frames = static_cast<int>(b.frame_count());
        const int nc     = static_cast<int>(b.channel_count());
        const int cap    = m_engine.max_partitions() * m_engine.block_size();
        const int L      = std::min(frames, cap);
        if (L <= 0) {
            return;
        }

        // Map buffer channels to the four IR paths (LL, LR, RL, RR); -1 = silent path.
        int srcch[conv_engine::k_paths];
        if (nc >= 4) {
            srcch[0] = 0;
            srcch[1] = 1;
            srcch[2] = 2;
            srcch[3] = 3; // true stereo
        }
        else if (nc >= 2) {
            srcch[0] = 0;
            srcch[1] = -1;
            srcch[2] = -1;
            srcch[3] = 1; // stereo IR, no cross-feed
        }
        else {
            srcch[0] = 0;
            srcch[1] = -1;
            srcch[2] = -1;
            srcch[3] = 0; // mono IR on both diagonals
        }

        // Deinterleave the needed channels into contiguous float buffers.
        std::array<std::vector<float>, conv_engine::k_paths> data;
        const float* ptrs[conv_engine::k_paths] = {nullptr, nullptr, nullptr, nullptr};
        double       energy                     = 0.0;
        for (int path = 0; path < conv_engine::k_paths; ++path) {
            const int ch = srcch[path];
            if (ch < 0 || ch >= nc) {
                continue;
            }
            data[path].resize(L);
            for (int j = 0; j < L; ++j) {
                const float s = b.lookup(static_cast<size_t>(j), static_cast<size_t>(ch));
                data[path][j] = s;
                energy += static_cast<double>(s) * static_cast<double>(s);
            }
            ptrs[path] = data[path].data();
        }

        double scale = 1.0;
        if (normalize && energy > 0.0) {
            scale = 1.0 / std::sqrt(energy);
        }

        m_engine.load_ir(ptrs, L, scale);
    }

    conv_engine m_engine;

    double m_sr{48000.0};
    double m_gain_lin{1.0};
    int    m_blocksize_pending{512};
    int    m_maxsize_pending{131072};

    std::array<std::vector<double>, 2> m_pd_ring; // wet-path predelay lines
    int                                m_pd_len{0};
    int                                m_pd_w{0};
};

MIN_EXTERNAL(convolve);
