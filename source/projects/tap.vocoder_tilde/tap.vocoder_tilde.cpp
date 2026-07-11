/// @file
/// @brief      tap.vocoder~ — a basic 24-band channel vocoder.
/// @details    Reconstructed from its surviving reference documentation (no source survived the
///             revival). A classic channel vocoder: a bank of 24 log-spaced bandpass filters
///             analyses the modulator (left inlet, e.g. a voice); a per-band envelope follower
///             tracks each band's amplitude; an identical filter bank splits the carrier (right
///             inlet, e.g. a synth) and each carrier band is multiplied by the matching modulator
///             envelope. The bands are summed to the output, imposing the modulator's spectral
///             envelope onto the carrier.
///
///             The documented attributes are honoured: `q` (the Q factor shared by every resonant
///             filter) and `response_interval` (the envelope-follower analysis period, in ms). The
///             original registered both as `symbol`; here they are `number` attributes, which is
///             how they actually behave (a Q value and a millisecond time). A practical `gain`
///             (linear makeup) attribute is added for level staging.
///
///             All DSP is plain portable C++ — no Jamoma, no min-lib. The bandpass sections are
///             RBJ Audio-EQ-Cookbook constant-0 dB-peak bandpass biquads (the same family as
///             tap.biquadcalc), which are unconditionally stable across the full band range.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <array>
#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class vocoder : public object<vocoder>, public sample_operator<2, 1> {
  public:
    static constexpr int    k_bands{24}; // "basic 24-band vocoder" (per the reference page)
    static constexpr double k_pi{3.14159265358979323846};
    static constexpr double k_fmin{50.0};    // lowest band centre (Hz)
    static constexpr double k_fmax{12000.0}; // highest band centre (Hz)

    MIN_DESCRIPTION{"A basic 24-band channel vocoder. The modulator (left inlet) imposes its "
                    "spectral envelope onto the carrier (right inlet) through a bank of bandpass "
                    "filters and per-band envelope followers."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.svf~, tap.comb~, vocoder~, fffb~"};

    inlet<>  m_in_mod{this, "(signal) modulator — the spectral envelope source (e.g. a voice)"};
    inlet<>  m_in_car{this, "(signal) carrier — the signal to be shaped (e.g. a synth)"};
    outlet<> m_out{this, "(signal) the vocoded output", "signal"};

    attribute<number> q{this,
                        "q",
                        20.0,
                        range{0.5, 200.0},
                        setter{MIN_FUNCTION{
                            m_q = args[0];
                            recalc_filters();
                            return {m_q};
                        }},
                        description{"The Q factor (resonance) shared by all of the bandpass filters. Higher values "
                                    "give narrower, more 'robotic' bands."}};

    attribute<number> response_interval{
        this,
        "response_interval",
        20.0,
        range{0.1, 1000.0},
        setter{MIN_FUNCTION{
            m_response_ms = args[0];
            recalc_envelope();
            return {m_response_ms};
        }},
        description{"The envelope-follower analysis period, in milliseconds. Shorter values track "
                    "the modulator more sharply; longer values smooth it."}};

    attribute<number> gain{this, "gain", 1.0, range{0.0, 100.0},
                           description{"Linear makeup gain applied to the summed output."}};

    message<> clear{this, "clear", "Reset all filter and envelope-follower state.",
                    MIN_FUNCTION{
                        for (auto& b : m_mod) {
                            b.clear();
                        }
                        for (auto& b : m_car) {
                            b.clear();
                        }
                        m_env.fill(0.0);
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Recompute coefficients when the DSP chain starts.",
                       MIN_FUNCTION{
                           recalc_filters();
                           recalc_envelope();
                           return {};
                       }};

    vocoder(const atoms& = {}) {
        recalc_filters();
        recalc_envelope();
    }

    // One RBJ constant-0 dB-peak bandpass biquad section, Direct Form I.
    struct biquad {
        double b0{0.0}, b1{0.0}, b2{0.0}, a1{0.0}, a2{0.0};
        double x1{0.0}, x2{0.0}, y1{0.0}, y2{0.0};

        inline double process(double x) {
            const double y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2             = x1;
            x1             = x;
            y2             = y1;
            y1             = y;
            return y;
        }
        void clear() { x1 = x2 = y1 = y2 = 0.0; }
    };

    sample operator()(sample modulator, sample carrier) {
        double out = 0.0;
        for (int i = 0; i < k_bands; ++i) {
            // Analyse the modulator band and follow its amplitude envelope.
            const double m    = m_mod[i].process(modulator);
            const double rect = std::fabs(m);
            m_env[i]          = m_env_coef * m_env[i] + (1.0 - m_env_coef) * rect;

            // Shape the matching carrier band by that envelope.
            const double c = m_car[i].process(carrier);
            out += c * m_env[i];
        }
        return out * m_gain_cached();
    }

  private:
    double m_q{20.0};
    double m_response_ms{20.0};
    double m_env_coef{0.0};

    std::array<biquad, k_bands> m_mod{};
    std::array<biquad, k_bands> m_car{};
    std::array<double, k_bands> m_env{};

    double m_gain_cached() const { return static_cast<double>(gain); }

    // Log-spaced band centre frequency for band i.
    static double band_frequency(int i) {
        const double t = (k_bands > 1) ? static_cast<double>(i) / (k_bands - 1) : 0.0;
        return k_fmin * std::pow(k_fmax / k_fmin, t);
    }

    void recalc_filters() {
        const double sr = samplerate();
        if (sr <= 0.0) {
            return;
        }
        const double q = (m_q > 0.001) ? m_q : 0.001;

        for (int i = 0; i < k_bands; ++i) {
            double fc = band_frequency(i);
            fc        = std::min(fc, 0.45 * sr); // keep every band well below Nyquist

            const double w0    = 2.0 * k_pi * fc / sr;
            const double cosw0 = std::cos(w0);
            const double alpha = std::sin(w0) / (2.0 * q);
            const double a0    = 1.0 + alpha;

            // Constant 0 dB peak-gain bandpass (RBJ cookbook), normalised by a0.
            const double b0 = alpha / a0;
            const double b2 = -alpha / a0;
            const double a1 = (-2.0 * cosw0) / a0;
            const double a2 = (1.0 - alpha) / a0;

            for (auto* bank : {&m_mod, &m_car}) {
                auto& bq = (*bank)[i];
                bq.b0    = b0;
                bq.b1    = 0.0;
                bq.b2    = b2;
                bq.a1    = a1;
                bq.a2    = a2;
            }
        }
    }

    void recalc_envelope() {
        const double sr = samplerate();
        if (sr <= 0.0) {
            return;
        }
        const double tau = (m_response_ms > 0.0001 ? m_response_ms : 0.0001) * 0.001; // ms → s
        m_env_coef       = std::exp(-1.0 / (tau * sr));
    }
};

MIN_EXTERNAL(vocoder);
