/// @file
/// @brief      tap.nr~ — spectral noise reduction (a spectral expander / gate).
/// @details    Reconstructed from its surviving reference documentation (no source survived the
///             revival). The original was a `pfft~`-hosted object (`tap.xnr~`) wrapped in an
///             abstraction; per the author's decision to reinvent the lost spectral DSP, this is a
///             **self-contained** external that runs its own short-time Fourier transform — no
///             `pfft~` needed.
///
///             Each analysis frame is windowed, transformed (an in-house radix-2 FFT), and every
///             bin whose magnitude falls below `threshold` is attenuated; bins above it pass. The
///             `slope` parameter controls how gradually the gate engages across the threshold (a
///             soft knee): low values fade gently, high values approach a hard gate. The frame is
///             inverse-transformed, windowed, and overlap-added back to the output. Faithful to the
///             documented behaviour: "a spectral expander/gate to silence bins whose signal falls
///             below a threshold," with the left inlet setting the threshold and the right inlet
///             setting how gradually it engages.
///
///             All DSP is plain portable C++ — no Jamoma, no min-lib, no external FFT library. The
///             window is a Hann window with 4× overlap; the overlap-add is normalised for perfect
///             reconstruction (with the gate open, the output equals the input delayed by the FFT
///             size). Latency = one FFT frame.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <cmath>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

class nr : public object<nr>, public vector_operator<> {
  public:
    static constexpr double k_pi{3.14159265358979323846};

    MIN_DESCRIPTION{"Spectral noise reduction: a spectral expander/gate that silences FFT bins "
                    "whose magnitude falls below a threshold. Self-contained — it runs its own "
                    "STFT, so no pfft~ wrapper is required."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.gate~, gate~, pfft~, fft~"};

    inlet<>  m_in{this, "(signal) input — and (float) sets the threshold"};
    inlet<>  m_thr{this, "(float) sets how gradually the threshold engages (the gate slope)"};
    outlet<> m_out{this, "(signal) the de-noised output", "signal"};

    // First argument = FFT size (a power of two). The original abstraction also took fftsize/2 as a
    // second argument for its pfft~; here it is redundant (derived internally) and ignored.
    nr(const atoms& args = {}) {
        if (!args.empty()) {
            const int requested = static_cast<int>(args[0]);
            if (is_power_of_two(requested) && requested >= 16) {
                m_fftsize = requested;
            }
        }
        configure();
    }

    attribute<number> threshold{
        this,
        "threshold",
        0.01,
        range{0.0, 1.0},
        setter{MIN_FUNCTION{
            m_threshold = args[0];
            return {m_threshold};
        }},
        description{"Linear-amplitude threshold. Bins below this level are attenuated; bins above "
                    "it pass. 0 disables the gate (the output then reconstructs the input)."}};

    attribute<number> slope{this,
                            "slope",
                            2.0,
                            range{0.0, 64.0},
                            setter{MIN_FUNCTION{
                                m_slope = args[0];
                                return {m_slope};
                            }},
                            description{"How gradually the gate engages across the threshold (soft knee). Low values "
                                        "fade gently; high values approach a hard gate; 0 passes everything."}};

    message<> m_number{this, "number",
                       "A float in the left inlet sets the threshold; in the right "
                       "inlet sets the slope.",
                       MIN_FUNCTION{
                           if (inlet == 1) {
                               slope = args[0];
                           }
                           else {
                               threshold = args[0];
                           }
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
            // Write the incoming sample; read (and clear) the reconstructed output sample.
            m_inbuf[m_pos]  = in[i];
            out[i]          = m_outbuf[m_pos];
            m_outbuf[m_pos] = 0.0;

            m_pos = (m_pos + 1) % N;

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
    double m_threshold{0.01};
    double m_slope{2.0};
    double m_norm{1.0};

    int m_pos{0};
    int m_hopcount{0};

    std::vector<double> m_window; // Hann (used for both analysis and synthesis)
    std::vector<double> m_inbuf;  // circular input buffer (length N)
    std::vector<double> m_outbuf; // circular overlap-add accumulator (length N)
    std::vector<double> m_re;     // scratch FFT real
    std::vector<double> m_im;     // scratch FFT imag

    static bool is_power_of_two(int v) { return v > 0 && (v & (v - 1)) == 0; }

    void configure() {
        const int N = m_fftsize;
        m_hop       = N / m_overlap;

        m_window.assign(N, 0.0);
        for (int k = 0; k < N; ++k) {
            m_window[k] = 0.5 - 0.5 * std::cos(2.0 * k_pi * k / N);
        }

        // COLA normalisation: overlap-add `overlap` copies of window^2 and read the steady-state
        // value. Dividing by it makes analysis+synthesis windowing reconstruct unity gain.
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
        reset_buffers();
    }

    void reset_buffers() {
        m_inbuf.assign(m_fftsize, 0.0);
        m_outbuf.assign(m_fftsize, 0.0);
        m_pos      = 0;
        m_hopcount = 0;
    }

    void process_frame() {
        const int N = m_fftsize;

        // Assemble the most recent N samples (oldest first) and apply the analysis window.
        for (int k = 0; k < N; ++k) {
            m_re[k] = m_inbuf[(m_pos + k) % N] * m_window[k];
            m_im[k] = 0.0;
        }

        fft(m_re, m_im, false);

        // Spectral gate: attenuate bins whose magnitude is below the threshold.
        const double thr = m_threshold;
        for (int k = 0; k < N; ++k) {
            const double mag  = std::sqrt(m_re[k] * m_re[k] + m_im[k] * m_im[k]) * (2.0 / N);
            double       gain = 1.0;
            if (thr > 0.0 && mag < thr) {
                if (m_slope <= 0.0) {
                    gain = 1.0;
                }
                else {
                    gain = std::pow(mag / thr, m_slope); // soft-knee downward expansion
                }
            }
            m_re[k] *= gain;
            m_im[k] *= gain;
        }

        fft(m_re, m_im, true); // inverse (scales by 1/N internally)

        // Synthesis window + overlap-add into the output accumulator.
        for (int k = 0; k < N; ++k) {
            m_outbuf[(m_pos + k) % N] += m_re[k] * m_window[k] * m_norm;
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

MIN_EXTERNAL(nr);
