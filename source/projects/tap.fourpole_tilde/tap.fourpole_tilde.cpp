/// @file
/// @brief      tap.fourpole~ — 4-pole resonant lowpass (Moog ladder) filter, stereo.
/// @details    The original 2015 object was built on the jamoma2 `LowpassFourPole`, whose source is
///             not present in this repository (the jamoma2 submodule was never populated). This is
///             therefore a *reimplementation* rather than a literal port: a classic Stilson/Smith
///             Moog-ladder 4-pole lowpass with `frequency` (Hz) and `q` (resonance) controls,
///             processed independently on two channels. DSP is portable C++.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2015-2026 Timothy Place.

#include <algorithm>

#include "c74_min.h"

using namespace c74::min;

class fourpole : public object<fourpole>, public sample_operator<2, 2> {
  private:
    struct ladder {
        double in1{0}, in2{0}, in3{0}, in4{0};
        double out1{0}, out2{0}, out3{0}, out4{0};
        void   clear() { in1 = in2 = in3 = in4 = out1 = out2 = out3 = out4 = 0.0; }
    };

  public:
    MIN_DESCRIPTION{"A 4-pole resonant lowpass (Moog ladder) filter, processed on two channels. "
                    "Controlled by frequency (Hz) and q (resonance)."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.svf~, lores~, reson~, onepole~"};

    inlet<>  m_in_l{this, "(signal) left input"};
    inlet<>  m_in_r{this, "(signal) right input"};
    outlet<> m_out_l{this, "(signal) left filtered output", "signal"};
    outlet<> m_out_r{this, "(signal) right filtered output", "signal"};

    attribute<number> frequency{this, "frequency", 1000.0, setter{MIN_FUNCTION{
                                    m_frequency = args[0];
                                    update_coefficients();
                                    return args;
                                }},
                                description{"Cutoff frequency in Hz."}};

    attribute<number> q{this, "q", 0.0, setter{MIN_FUNCTION{
                            m_q = MIN_CLAMP(static_cast<double>(args[0]), 0.0, 1.0);
                            update_coefficients();
                            return args;
                        }},
                        description{"Resonance (0 = none, approaching 1 = self-oscillation)."}};

    message<> clear{this, "clear", "Reset the filter's internal state.",
                    MIN_FUNCTION{
                        m_l.clear();
                        m_r.clear();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Recompute coefficients for the current sample rate.",
                       MIN_FUNCTION{
                           update_coefficients();
                           return {};
                       }};

    samples<2> operator()(sample in_l, sample in_r) { return {tick(m_l, in_l), tick(m_r, in_r)}; }

  private:
    double m_frequency{1000.0};
    double m_q{0.0};
    double m_f{0.0};  // normalized cutoff coefficient
    double m_fb{0.0}; // resonance feedback amount
    ladder m_l, m_r;

    void update_coefficients() {
        const double sr = samplerate();
        double       f  = (m_frequency / (sr * 0.5)) * 1.16;
        f               = std::clamp(f, 0.0, 0.999);
        m_f             = f;
        m_fb            = m_q * 4.0 * (1.0 - 0.15 * f * f);
    }

    double tick(ladder& s, double x) {
        x -= s.out4 * m_fb;
        x *= 0.35013 * (m_f * m_f) * (m_f * m_f);

        s.out1 = x + 0.3 * s.in1 + (1.0 - m_f) * s.out1;
        s.in1  = x;
        s.out2 = s.out1 + 0.3 * s.in2 + (1.0 - m_f) * s.out2;
        s.in2  = s.out1;
        s.out3 = s.out2 + 0.3 * s.in3 + (1.0 - m_f) * s.out3;
        s.in3  = s.out2;
        s.out4 = s.out3 + 0.3 * s.in4 + (1.0 - m_f) * s.out4;
        s.in4  = s.out3;

        return s.out4;
    }
};

MIN_EXTERNAL(fourpole);
