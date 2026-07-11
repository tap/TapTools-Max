/// @file
/// @brief      tap.spectra~ — spectral remapping.
/// @details    Reconstructed from its surviving reference documentation (no source survived the
///             revival). The original ran a `pfft~` subpatcher (`tap.spectra.pfft`) that used
///             `tap.scale~` to reorient FFT bins onto different IFFT bins; per the author's decision
///             to reinvent the lost spectral DSP, this is a **self-contained** external that runs
///             its own short-time Fourier transform — no `pfft~` needed.
///
///             Per the reference page: "performs a remapping of spectral data by reorienting the
///             bins of an FFT to different bins of an IFFT. The result is an ultra-non-linear effect
///             that can yield unexpected results." Here, each output bin <i>k</i> takes its complex
///             value from input bin round(<i>k</i> · `remap`). `remap` = 1 is the identity (output
///             reconstructs the input); &gt; 1 stretches the spectrum upward, &lt; 1 compresses it
///             toward DC. Hermitian symmetry is enforced so the output stays real.
///
///             All DSP is plain portable C++ — no Jamoma, no min-lib, no external FFT library. Hann
///             window, 4× overlap, COLA-normalised overlap-add. Latency = one FFT frame.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2002-2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

class spectra : public object<spectra>, public vector_operator<> {
  public:
    static constexpr double k_pi{3.14159265358979323846};

    MIN_DESCRIPTION{"Spectral remapping: reorients the bins of an FFT onto different bins of an "
                    "IFFT, an ultra-non-linear effect. Self-contained — it runs its own STFT, so "
                    "no pfft~ wrapper is required."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.nr~, pfft~, fft~, gizmo~"};

    inlet<>  m_in{this, "(signal) input — and (float) sets the remapping ratio"};
    outlet<> m_out{this, "(signal) the spectrally remapped output", "signal"};

    spectra(const atoms& args = {}) {
        if (!args.empty()) {
            const int requested = static_cast<int>(args[0]);
            if (is_power_of_two(requested) && requested >= 16) {
                m_fftsize = requested;
            }
        }
        configure();
    }

    attribute<number> remap{this,
                            "remap",
                            1.0,
                            range{0.0, 16.0},
                            setter{MIN_FUNCTION{
                                m_remap = args[0];
                                return {m_remap};
                            }},
                            description{"Spectral remapping ratio. Output bin k takes its value from input bin "
                                        "round(k * remap). 1 is the identity; >1 stretches the spectrum upward, <1 "
                                        "compresses it toward DC."}};

    message<> m_number{this, "number", "A float sets the remapping ratio.",
                       MIN_FUNCTION{
                           remap = args[0];
                           return {};
                       }};

    message<> clear{this, "clear", "Reset the internal STFT buffers.",
                    MIN_FUNCTION{
                        reset_buffers();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Reset buffers when the DSP chain starts.",
                       MIN_FUNCTION{
                           reset_buffers();
                           return {};
                       }};

    void operator()(audio_bundle input, audio_bundle output) {
        const long    n   = input.frame_count();
        const double* in  = input.samples(0);
        double*       out = output.samples(0);
        const int     N   = m_fftsize;

        for (long i = 0; i < n; ++i) {
            m_inbuf[m_pos]  = in[i];
            out[i]          = m_outbuf[m_pos];
            m_outbuf[m_pos] = 0.0;
            m_pos           = (m_pos + 1) % N;

            if (++m_hopcount >= m_hop) {
                m_hopcount = 0;
                process_frame();
            }
        }
    }

  private:
    int    m_fftsize{1024};
    int    m_overlap{4};
    int    m_hop{256};
    double m_remap{1.0};
    double m_norm{1.0};

    int m_pos{0};
    int m_hopcount{0};

    std::vector<double> m_window;
    std::vector<double> m_inbuf;
    std::vector<double> m_outbuf;
    std::vector<double> m_re, m_im;   // input spectrum / time scratch
    std::vector<double> m_ore, m_oim; // remapped spectrum

    static bool is_power_of_two(int v) { return v > 0 && (v & (v - 1)) == 0; }

    void configure() {
        const int N = m_fftsize;
        m_hop       = N / m_overlap;

        m_window.assign(N, 0.0);
        for (int k = 0; k < N; ++k) {
            m_window[k] = 0.5 - 0.5 * std::cos(2.0 * k_pi * k / N);
        }

        std::vector<double> cola(N, 0.0);
        for (int o = 0; o < m_overlap; ++o) {
            for (int k = 0; k < N; ++k) {
                cola[(o * m_hop + k) % N] += m_window[k] * m_window[k];
            }
        }
        const double c = cola[N / 2];
        m_norm         = (c > 0.0) ? (1.0 / c) : 1.0;

        m_re.assign(N, 0.0);
        m_im.assign(N, 0.0);
        m_ore.assign(N, 0.0);
        m_oim.assign(N, 0.0);
        reset_buffers();
    }

    void reset_buffers() {
        m_inbuf.assign(m_fftsize, 0.0);
        m_outbuf.assign(m_fftsize, 0.0);
        m_pos      = 0;
        m_hopcount = 0;
    }

    void process_frame() {
        const int N    = m_fftsize;
        const int half = N / 2;

        for (int k = 0; k < N; ++k) {
            m_re[k] = m_inbuf[(m_pos + k) % N] * m_window[k];
            m_im[k] = 0.0;
        }

        fft(m_re, m_im, false);

        // Remap: output bin k <- input bin round(k * remap), over the lower half; then mirror to
        // keep the spectrum Hermitian (so the inverse transform is real).
        for (int k = 0; k <= half; ++k) {
            const int src = static_cast<int>(std::lround(k * m_remap));
            if (src >= 0 && src <= half) {
                m_ore[k] = m_re[src];
                m_oim[k] = m_im[src];
            }
            else {
                m_ore[k] = 0.0;
                m_oim[k] = 0.0;
            }
        }
        m_oim[0]    = 0.0; // DC is real
        m_oim[half] = 0.0; // Nyquist is real
        for (int k = 1; k < half; ++k) {
            m_ore[N - k] = m_ore[k];
            m_oim[N - k] = -m_oim[k];
        }

        fft(m_ore, m_oim, true);

        for (int k = 0; k < N; ++k) {
            m_outbuf[(m_pos + k) % N] += m_ore[k] * m_window[k] * m_norm;
        }
    }

    // In-place iterative radix-2 Cooley–Tukey FFT. `inverse` divides by N.
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
};

MIN_EXTERNAL(spectra);
