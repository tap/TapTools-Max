/// @file
/// @brief      tap.filter~ — a unified multimode audio filter.
/// @details    The one flexible filter: a single-channel biquad whose filter type (mode) is
///             selectable at runtime among the Robert Bristow-Johnson "Audio EQ Cookbook" shapes
///             (lowpass, highpass, bandpass, bandpass_constant_peak, notch, allpass, peaking,
///             lowshelf, highshelf). Historically tap.filter~ was the flagship Jamoma multichannel
///             filter advertised as having "a myriad of filter types" with a mode you can change on
///             the fly; this revival reimplements that spirit directly in portable C++.
///
///             The coefficient math is the exact RBJ cookbook used by tap.biquadcalc. The runtime is
///             a Transposed Direct Form II biquad (good numerical behavior, only two state
///             variables). Frequency can be modulated at audio rate via a signal in the second
///             inlet; mode/frequency/q/gain are all settable by attribute or message. Coefficient
///             changes are linearly smoothed across each signal vector to suppress zipper noise.
///
///             Single channel by design — wrap in an mc. operator for multichannel, per the project
///             convention. No Jamoma, no min-lib: all DSP is plain portable C++.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 1999-2026 Timothy Place.

#include <algorithm>
#include <cmath>

#include "c74_min.h"

using namespace c74::min;

class filter : public object<filter>, public vector_operator<> {
  private:
    static constexpr double k_twopi{6.28318530717958647692};

  public:
    // The set of biquad coefficients (already normalized by a0), in the convention
    //     y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
    // Public so the unit tests can assert the RBJ math directly.
    struct coeffs {
        double b0{1.0}, b1{0.0}, b2{0.0}, a1{0.0}, a2{0.0};
    };

    MIN_DESCRIPTION{"A unified multimode audio filter. One biquad whose filter type (mode) is "
                    "selectable at runtime among the RBJ Audio EQ Cookbook shapes: lowpass, "
                    "highpass, bandpass, bandpass_constant_peak, notch, allpass, peaking, lowshelf, "
                    "and highshelf. Frequency, Q, and gain are settable by message, attribute, or "
                    "(for frequency) an audio-rate signal. Wrap in an mc. operator for multichannel."};
    MIN_TAGS{"filters"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.svf~, tap.biquadcalc, biquad~, filtergraph~"};

    inlet<>  m_in{this, "(signal) audio input"};
    inlet<>  m_in_freq{this, "(signal/float) cutoff / center frequency in Hz"};
    outlet<> m_out{this, "(signal) filtered audio output", "signal"};

    attribute<symbol> mode{this,
                           "mode",
                           "lowpass",
                           range{"lowpass", "highpass", "bandpass", "bandpass_constant_peak", "notch", "allpass",
                                 "peaking", "lowshelf", "highshelf"},
                           setter{MIN_FUNCTION{
                               m_dirty = true;
                               return args;
                           }},
                           description{"The filter type. One of: lowpass, highpass, bandpass, "
                                       "bandpass_constant_peak, notch, allpass, peaking, lowshelf, highshelf. "
                                       "Changeable at runtime."}};

    attribute<number> frequency{this, "frequency", 1000.0, setter{MIN_FUNCTION{
                                    m_freq  = std::max(0.0, static_cast<double>(args[0]));
                                    m_dirty = true;
                                    return {m_freq};
                                }},
                                description{"Cutoff / center frequency of the filter, in Hz."}};

    attribute<number> q{this, "q", 0.707, setter{MIN_FUNCTION{
                            m_q     = std::max(1e-6, static_cast<double>(args[0]));
                            m_dirty = true;
                            return {m_q};
                        }},
                        description{"The Q (resonance / narrowness / steepness) of the filter. For the shelf "
                                    "types this is reused as the shelf slope S."}};

    attribute<number> gain{this, "gain", 0.0, setter{MIN_FUNCTION{
                               m_gain  = args[0];
                               m_dirty = true;
                               return args;
                           }},
                           description{"Gain in decibels, applied by the peaking, lowshelf, and highshelf types."}};

    message<> clear{this, "clear", "Reset the filter's internal state (flush the delay memory).",
                    MIN_FUNCTION{
                        m_z1 = 0.0;
                        m_z2 = 0.0;
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Recompute coefficients when the DSP chain starts.",
                       MIN_FUNCTION{
                           m_z1      = 0.0;
                           m_z2      = 0.0;
                           m_current = compute(m_freq); // snap to target so no smoothing ramp at startup
                           m_dirty   = false;
                           return {};
                       }};

    filter(const atoms& = {}) { m_current = compute(m_freq); }

    void operator()(audio_bundle input, audio_bundle output) override {
        const long    n   = input.frame_count();
        const double* in  = input.samples(0);
        double*       out = output.samples(0);

        // An audio-rate signal patched into the frequency inlet drives the cutoff. We read its
        // first sample per vector (control-rate modulation), matching the per-vector coefficient
        // update used here; a float into that inlet sets the frequency attribute instead.
        const bool freq_signal = m_in_freq.has_signal_connection();
        if (freq_signal) {
            const double f = std::max(0.0, input.samples(1)[0]);
            if (f != m_freq) {
                m_freq  = f;
                m_dirty = true;
            }
        }

        // Recompute the target coefficients once per vector when anything changed, then linearly
        // smooth the live coefficients toward that target across the vector to avoid zipper noise.
        coeffs target = m_dirty ? compute(m_freq) : m_current;
        m_dirty       = false;

        const double inv  = (n > 0) ? 1.0 / static_cast<double>(n) : 0.0;
        const double d_b0 = (target.b0 - m_current.b0) * inv;
        const double d_b1 = (target.b1 - m_current.b1) * inv;
        const double d_b2 = (target.b2 - m_current.b2) * inv;
        const double d_a1 = (target.a1 - m_current.a1) * inv;
        const double d_a2 = (target.a2 - m_current.a2) * inv;

        coeffs c = m_current;
        for (long i = 0; i < n; ++i) {
            c.b0 += d_b0;
            c.b1 += d_b1;
            c.b2 += d_b2;
            c.a1 += d_a1;
            c.a2 += d_a2;

            // Transposed Direct Form II: only two state variables, good float behavior.
            const double x = in[i];
            const double y = c.b0 * x + m_z1;
            m_z1           = c.b1 * x - c.a1 * y + m_z2;
            m_z2           = c.b2 * x - c.a2 * y;
            out[i]         = y;
        }
        m_z1 = anti_denormal(m_z1);
        m_z2 = anti_denormal(m_z2);

        m_current = target;
    }

    // Compute normalized biquad coefficients for the given cutoff using the current mode/q/gain.
    // Exposed (public) so unit tests can assert the RBJ math directly.
    coeffs compute(double f0) const {
        const double Fs = samplerate();
        coeffs       co;

        // Degenerate / out-of-range cutoff -> pass-through (avoids NaNs at startup or fs==0).
        if (Fs <= 0.0 || f0 <= 0.0 || f0 >= Fs * 0.5) {
            return co;
        }

        const double d_bgain = m_gain;
        const double Q       = m_q;
        const double S       = m_q; // q doubles as the shelf slope, as in tap.biquadcalc

        const double A   = std::pow(10.0, d_bgain / 40.0);
        const double w0  = k_twopi * f0 / Fs;
        const double cw0 = std::cos(w0);
        const double sw0 = std::sin(w0);

        const symbol t = mode;

        double alpha;
        if (t == "lowshelf" || t == "highshelf") {
            alpha = sw0 / 2.0 * std::sqrt((A + 1.0 / A) * (1.0 / S - 1.0) + 2.0);
        }
        else {
            alpha = sw0 / (2.0 * Q);
        }

        // RBJ cookbook, normalized so that a0 == 1. Names here are the RBJ convention:
        //     b* = feedforward (numerator), a* = feedback (denominator).
        double b0, b1, b2, a0, a1, a2;

        if (t == "lowpass") {
            b0 = (1.0 - cw0) / 2.0;
            b1 = 1.0 - cw0;
            b2 = (1.0 - cw0) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cw0;
            a2 = 1.0 - alpha;
        }
        else if (t == "highpass") {
            b0 = (1.0 + cw0) / 2.0;
            b1 = -(1.0 + cw0);
            b2 = (1.0 + cw0) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cw0;
            a2 = 1.0 - alpha;
        }
        else if (t == "bandpass") {
            // constant skirt gain, peak gain == Q
            b0 = sw0 / 2.0;
            b1 = 0.0;
            b2 = -sw0 / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cw0;
            a2 = 1.0 - alpha;
        }
        else if (t == "bandpass_constant_peak") {
            // constant 0 dB peak gain
            b0 = alpha;
            b1 = 0.0;
            b2 = -alpha;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cw0;
            a2 = 1.0 - alpha;
        }
        else if (t == "notch") {
            b0 = 1.0;
            b1 = -2.0 * cw0;
            b2 = 1.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cw0;
            a2 = 1.0 - alpha;
        }
        else if (t == "allpass") {
            b0 = 1.0 - alpha;
            b1 = -2.0 * cw0;
            b2 = 1.0 + alpha;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cw0;
            a2 = 1.0 - alpha;
        }
        else if (t == "peaking") {
            b0 = 1.0 + alpha * A;
            b1 = -2.0 * cw0;
            b2 = 1.0 - alpha * A;
            a0 = 1.0 + alpha / A;
            a1 = -2.0 * cw0;
            a2 = 1.0 - alpha / A;
        }
        else if (t == "lowshelf") {
            const double tsa = 2.0 * std::sqrt(A) * alpha;
            b0               = A * ((A + 1.0) - (A - 1.0) * cw0 + tsa);
            b1               = 2.0 * A * ((A - 1.0) - (A + 1.0) * cw0);
            b2               = A * ((A + 1.0) - (A - 1.0) * cw0 - tsa);
            a0               = (A + 1.0) + (A - 1.0) * cw0 + tsa;
            a1               = -2.0 * ((A - 1.0) + (A + 1.0) * cw0);
            a2               = (A + 1.0) + (A - 1.0) * cw0 - tsa;
        }
        else if (t == "highshelf") {
            const double tsa = 2.0 * std::sqrt(A) * alpha;
            b0               = A * ((A + 1.0) + (A - 1.0) * cw0 + tsa);
            b1               = -2.0 * A * ((A - 1.0) + (A + 1.0) * cw0);
            b2               = A * ((A + 1.0) + (A - 1.0) * cw0 - tsa);
            a0               = (A + 1.0) - (A - 1.0) * cw0 + tsa;
            a1               = 2.0 * ((A - 1.0) - (A + 1.0) * cw0);
            a2               = (A + 1.0) - (A - 1.0) * cw0 - tsa;
        }
        else {
            // unknown mode: pass through unchanged
            return co;
        }

        const double inv_a0 = 1.0 / a0;
        co.b0               = b0 * inv_a0;
        co.b1               = b1 * inv_a0;
        co.b2               = b2 * inv_a0;
        co.a1               = a1 * inv_a0;
        co.a2               = a2 * inv_a0;
        return co;
    }

  private:
    static double anti_denormal(double x) { return (std::abs(x) < 1e-15) ? 0.0 : x; }

    // Cached parameter state (mirrors the attributes so the perform loop reads plain doubles).
    double m_freq{1000.0};
    double m_q{0.707};
    double m_gain{0.0};
    bool   m_dirty{true};

    coeffs m_current; // the live (smoothed) coefficients
    double m_z1{0.0}; // TDF-II state
    double m_z2{0.0};
};

MIN_EXTERNAL(filter);
