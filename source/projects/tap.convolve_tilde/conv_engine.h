/// @file
/// @brief      conv_engine — a Min-free, uniformly-partitioned overlap-save convolution engine.
/// @details    The DSP core of tap.convolve~, kept deliberately independent of Max/Min so it can be
///             unit-tested directly against a reference convolution (see tap.convolve_tilde_test.cpp)
///             — buffer~-backed Min objects can't link against the mock test kernel, so the portable
///             engine lives here on its own. Plain C++17: no Jamoma, no min-lib, no external FFT
///             library.
///
///             Uniformly-partitioned overlap-save: the impulse response is split into equal blocks,
///             each transformed once with an in-house radix-2 FFT; the running output is a
///             frequency-domain multiply-accumulate over a frequency-domain delay line (FDL) of past
///             input spectra. Latency is one partition; everything else is exact linear convolution.
///
///             True stereo — four IR paths give the full 2×2 response, with the two input channels
///             transformed once per block and shared across the paths:
///                 out_l = in_l ∗ h_LL + in_r ∗ h_RL ;  out_r = in_l ∗ h_LR + in_r ∗ h_RR
///             Path indexing: path = in_channel * 2 + out_channel, i.e. 0=LL, 1=LR, 2=RL, 3=RR.
///
///             Deferred optimisation: the multiply-accumulate and the IR tables use the full complex
///             spectrum; a real-input half-spectrum (N/2+1 bins) form would halve both CPU and memory.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <vector>

class conv_engine {
  public:
    static constexpr double k_pi       = 3.14159265358979323846;
    static constexpr int    k_paths    = 4;
    static constexpr int    k_channels = 2;

    // Allocate all buffers for the given partition size and maximum partition count. Must be called
    // where the audio thread is not running concurrently (i.e. from dspsetup). Resets all state and
    // discards any loaded IR.
    void configure(int blocksize, int max_partitions) {
        m_block     = std::max(1, blocksize);
        m_fftsize   = 2 * m_block;
        m_max_parts = std::max(1, max_partitions);

        const int flat = m_max_parts * m_fftsize;
        for (int path = 0; path < k_paths; ++path) {
            for (int slot = 0; slot < 2; ++slot) {
                m_ir_re[path][slot].assign(flat, 0.0);
                m_ir_im[path][slot].assign(flat, 0.0);
            }
        }
        for (int ch = 0; ch < k_channels; ++ch) {
            m_fdl_re[ch].assign(flat, 0.0);
            m_fdl_im[ch].assign(flat, 0.0);
            m_prev[ch].assign(m_block, 0.0);
            m_inblk[ch].assign(m_block, 0.0);
            m_outblk[ch].assign(m_block, 0.0);
        }
        m_fre.assign(m_fftsize, 0.0);
        m_fim.assign(m_fftsize, 0.0);
        m_are.assign(m_fftsize, 0.0);
        m_aim.assign(m_fftsize, 0.0);

        m_slot_parts[0] = m_slot_parts[1] = 0;
        m_active.store(-1, std::memory_order_release); // no IR yet
        clear();
    }

    // Flush the running state (input history + pending output) without discarding the loaded IR.
    // Zeroes buffers the audio thread reads; safe to call from a message handler (no reallocation).
    void clear() {
        for (int ch = 0; ch < k_channels; ++ch) {
            std::fill(m_fdl_re[ch].begin(), m_fdl_re[ch].end(), 0.0);
            std::fill(m_fdl_im[ch].begin(), m_fdl_im[ch].end(), 0.0);
            std::fill(m_prev[ch].begin(), m_prev[ch].end(), 0.0);
            std::fill(m_inblk[ch].begin(), m_inblk[ch].end(), 0.0);
            std::fill(m_outblk[ch].begin(), m_outblk[ch].end(), 0.0);
        }
        m_pos = 0;
        m_fdl = 0;
    }

    int  block_size() const { return m_block; }
    int  max_partitions() const { return m_max_parts; }
    bool configured() const { return m_block > 0 && !m_ir_re[0][0].empty(); }
    bool has_ir() const { return m_active.load(std::memory_order_acquire) >= 0; }

    // Build the four IR paths into the currently-inactive slot and publish it atomically. `paths` holds
    // four source pointers (any may be null for a silent path); `length` is the IR length in samples
    // (clamped to capacity); `scale` is applied to every sample (used for normalisation / gain).
    // Runs off the audio thread; only writes the inactive slot, then flips one atomic.
    void load_ir(const float* const paths[k_paths], int length, double scale) {
        if (!configured()) {
            return;
        }

        const int a        = m_active.load(std::memory_order_acquire);
        const int inactive = (a < 0) ? 0 : (1 - a);

        const int P = std::min(m_max_parts, (std::max(0, length) + m_block - 1) / m_block);

        for (int path = 0; path < k_paths; ++path) {
            const float* src = paths[path];
            double*      Hre = m_ir_re[path][inactive].data();
            double*      Him = m_ir_im[path][inactive].data();

            for (int p = 0; p < P; ++p) {
                std::fill(m_fre.begin(), m_fre.end(), 0.0);
                std::fill(m_fim.begin(), m_fim.end(), 0.0);
                if (src) {
                    for (int j = 0; j < m_block; ++j) {
                        const int idx = p * m_block + j;
                        if (idx < length) {
                            m_fre[j] = static_cast<double>(src[idx]) * scale;
                        }
                    }
                }
                fft(m_fre, m_fim, false);
                std::copy(m_fre.begin(), m_fre.end(), Hre + p * m_fftsize);
                std::copy(m_fim.begin(), m_fim.end(), Him + p * m_fftsize);
            }
        }

        m_slot_parts[inactive] = P;                          // written before the publish...
        m_active.store(inactive, std::memory_order_release); // ...so (slot, P) stay consistent.
    }

    // Process n stereo samples. Wet (fully convolved) output is written to out_l/out_r. Safe for any n;
    // input/output must not alias each other.
    void process(const double* in_l, const double* in_r, double* out_l, double* out_r, long n) {
        for (long i = 0; i < n; ++i) {
            m_inblk[0][m_pos] = in_l[i];
            m_inblk[1][m_pos] = in_r[i];
            out_l[i]          = m_outblk[0][m_pos];
            out_r[i]          = m_outblk[1][m_pos];
            if (++m_pos == m_block) {
                process_block();
                m_pos = 0;
            }
        }
    }

  private:
    // Transform this block's two input frames, store them in the frequency-domain delay line, and form
    // the four-path multiply-accumulate for both outputs.
    void process_block() {
        const int cur = m_fdl;

        // 1. Analyse both input channels into the FDL (overlap-save frame = [prev block ; this block]).
        for (int ch = 0; ch < k_channels; ++ch) {
            for (int j = 0; j < m_block; ++j) {
                m_fre[j]           = m_prev[ch][j];
                m_fre[m_block + j] = m_inblk[ch][j];
                m_fim[j]           = 0.0;
                m_fim[m_block + j] = 0.0;
            }
            fft(m_fre, m_fim, false);
            std::copy(m_fre.begin(), m_fre.end(), m_fdl_re[ch].data() + cur * m_fftsize);
            std::copy(m_fim.begin(), m_fim.end(), m_fdl_im[ch].data() + cur * m_fftsize);
            std::copy(m_inblk[ch].begin(), m_inblk[ch].end(), m_prev[ch].begin());
        }

        const int a = m_active.load(std::memory_order_acquire);
        if (a < 0) {
            // No IR loaded — emit silence.
            for (int ch = 0; ch < k_channels; ++ch) {
                std::fill(m_outblk[ch].begin(), m_outblk[ch].end(), 0.0);
            }
            m_fdl = (cur + 1) % m_max_parts;
            return;
        }
        const int P = m_slot_parts[a];

        // 2. For each output channel, accumulate the frequency-domain product over both input channels
        //    and every partition, then inverse-transform and keep the non-aliased second half.
        for (int oc = 0; oc < k_channels; ++oc) {
            std::fill(m_are.begin(), m_are.end(), 0.0);
            std::fill(m_aim.begin(), m_aim.end(), 0.0);

            for (int ic = 0; ic < k_channels; ++ic) {
                const int     path = ic * 2 + oc;
                const double* Hre  = m_ir_re[path][a].data();
                const double* Him  = m_ir_im[path][a].data();
                const double* Xre  = m_fdl_re[ic].data();
                const double* Xim  = m_fdl_im[ic].data();

                for (int p = 0; p < P; ++p) {
                    int slot = cur - p;
                    if (slot < 0) {
                        slot += m_max_parts;
                    }
                    const double* hr = Hre + p * m_fftsize;
                    const double* hi = Him + p * m_fftsize;
                    const double* xr = Xre + slot * m_fftsize;
                    const double* xi = Xim + slot * m_fftsize;
                    for (int k = 0; k < m_fftsize; ++k) {
                        m_are[k] += hr[k] * xr[k] - hi[k] * xi[k];
                        m_aim[k] += hr[k] * xi[k] + hi[k] * xr[k];
                    }
                }
            }

            fft(m_are, m_aim, true);
            for (int j = 0; j < m_block; ++j) {
                m_outblk[oc][j] = m_are[m_block + j]; // overlap-save: discard the aliased first half
            }
        }

        m_fdl = (cur + 1) % m_max_parts;
    }

    // In-place iterative radix-2 Cooley–Tukey FFT (same routine as tap.nr~). `inverse` divides by N.
    static void fft(std::vector<double>& re, std::vector<double>& im, bool inverse) {
        const int n = static_cast<int>(re.size());

        for (int i = 1, j = 0; i < n; ++i) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) {
                j ^= bit;
            }
            j ^= bit;
            if (i < j) {
                std::swap(re[i], re[j]);
                std::swap(im[i], im[j]);
            }
        }

        for (int len = 2; len <= n; len <<= 1) {
            const double ang = 2.0 * k_pi / len * (inverse ? 1.0 : -1.0);
            const double wr  = std::cos(ang);
            const double wi  = std::sin(ang);
            for (int i = 0; i < n; i += len) {
                double cwr = 1.0, cwi = 0.0;
                for (int k = 0; k < len / 2; ++k) {
                    const double vr     = re[i + k + len / 2] * cwr - im[i + k + len / 2] * cwi;
                    const double vi     = re[i + k + len / 2] * cwi + im[i + k + len / 2] * cwr;
                    const double ur     = re[i + k];
                    const double ui     = im[i + k];
                    re[i + k]           = ur + vr;
                    im[i + k]           = ui + vi;
                    re[i + k + len / 2] = ur - vr;
                    im[i + k + len / 2] = ui - vi;
                    const double ncwr   = cwr * wr - cwi * wi;
                    cwi                 = cwr * wi + cwi * wr;
                    cwr                 = ncwr;
                }
            }
        }

        if (inverse) {
            for (int i = 0; i < n; ++i) {
                re[i] /= n;
                im[i] /= n;
            }
        }
    }

    int m_block{0};     // partition (block) size = latency in samples
    int m_fftsize{0};   // FFT size = 2 * m_block
    int m_max_parts{0}; // capacity in partitions

    std::atomic<int> m_active{-1};          // published IR slot (0/1), or -1 for none. Audio thread reads it.
    int              m_slot_parts[2]{0, 0}; // partition count per slot (kept consistent with m_active)

    std::array<std::array<std::vector<double>, 2>, k_paths> m_ir_re, m_ir_im;   // IR spectra [path][slot]
    std::array<std::vector<double>, k_channels>             m_fdl_re, m_fdl_im; // input FDL [channel]
    std::array<std::vector<double>, k_channels>             m_prev, m_inblk, m_outblk;

    std::vector<double> m_fre, m_fim, m_are, m_aim; // scratch

    int m_pos{0}; // fill/read index within the current block [0, m_block)
    int m_fdl{0}; // ring index of the newest input spectrum
};
